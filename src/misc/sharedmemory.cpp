/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012 Alexander Bock                                                     *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#include <ghoul/misc/sharedmemory.h>

#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/crc32.h>
#include <atomic>

#ifndef WIN32
#include <sys/shm.h>
#endif

namespace ghoul {

#ifdef WIN32
std::map<const std::string, HANDLE> SharedMemory::_createdSections;
#endif

namespace {
    struct Header {
        std::atomic_flag mutex;
#ifdef WIN32
        size_t size;
#endif
    };

    const unsigned char STATUS_SUCCESS  = 0;
    const unsigned char STATUS_NOT_FOUND = 1;
    const unsigned char STATUS_NO_MAPPING = 2;
    const unsigned char STATUS_ALL_ERRORS = 255;
    
    Header* header(void* memory) {
        return reinterpret_cast<Header*>(memory);
    }
    
    unsigned int hash(const std::string& name) {
        return hashCRC32(name);
    }

#ifdef WIN32
    std::string lastErrorToString(DWORD error) {
        LPTSTR errorBuffer = nullptr;
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&errorBuffer,
            0,
            NULL);
        if (errorBuffer != nullptr) {
            std::string error(errorBuffer);
            LocalFree(errorBuffer);
            return error;
        }
        else {
            LERRORC_SAFE("SharedMemory",
                "Error constructing format message for error: " << error);
            return "";
        }
    }
#endif
}
    
bool SharedMemory::create(const std::string& name, size_t size) {
    const std::string _loggerCat = "SharedMemory(" + name + ")";
    // adjust for the header size
    size += sizeof(Header);
#ifdef WIN32
    HANDLE handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                      0, static_cast<DWORD>(size), name.c_str());
    const DWORD error = GetLastError();
    if (handle == NULL) {
        const std::string&& errorMsg = lastErrorToString(error);
        LERROR_SAFE("Error occurred while creating shared memory: " << errorMsg);
        return false;
    }
    else { 
        if (error == ERROR_ALREADY_EXISTS) {
            LERROR_SAFE("Error occurred while creating shared memory: " <<
                "Section already exists");
            return false;
        }
        else {
            void* memory = MapViewOfFileEx(handle, FILE_MAP_ALL_ACCESS, 0, 0, 0, NULL);

            if (memory == nullptr) {
                const std::string&& errorMsg = lastErrorToString(error);
                LERROR_SAFE("Error occurred while creating a view on shared memory: " <<
                    errorMsg);
                return false;
            }
            else {
                Header* h = header(memory);
                h->mutex.clear();
                h->size = size  - sizeof(Header);
                UnmapViewOfFile(memory);
                _createdSections[name] = handle;
                return true;
            }
        }
    }
#else
    const unsigned int h = hash(name);
    const int result = shmget(h, size, IPC_CREAT | IPC_EXCL | IPC_R | IPC_W | IPC_M);
    if (result == -1) {
        LERROR_SAFE("Error occurred while creating shared memory: " << strerror(errno));
        return false;
    }
    void* memory = shmat(_sharedMemoryHandle, NULL, SHM_R | SHM_W);
    Header* h = header(memory);
    h->mutex.clear();
    shmdt(memory);
    return true;
#endif
}
 
bool SharedMemory::remove(const std::string& name) {
    const std::string&& _loggerCat = "SharedMemory(" + name + ")";
#ifdef WIN32
#ifdef GHL_DEBUG
    if (_createdSections.find(name) == _createdSections.end()) {
        LERROR_SAFE("Shared memory section was not found");
        return false;
    }
#endif
    HANDLE h = _createdSections[name];
    _createdSections.erase(name);
    const BOOL result = CloseHandle(h);
    if (result == 0) {
        const DWORD error = GetLastError();
        const std::string&& errorMsg = lastErrorToString(error);
        LERROR_SAFE("Error closing handle: " << errorMsg);
        return false;
    }
    else
        return true;
#else
    const unsigned int h = hash(name);
    int result = shmget(h, 0, IPC_R | IPC_W | IPC_M);
    if (result == -1) {
        LERROR_SAFE("Error occurred while retrieving shared memory: " <<
                    strerror(errno));
        return false;
    }
    else {
        result = shmctl(result, IPC_RMID, nullptr);
        if (result == -1) {
            LERROR_SAFE("Error occurred while removing shared memory: " <<
                        strerror(errno));
            return false;
        }
        else
            return true;
    }
#endif
}
    
bool SharedMemory::exists(const std::string& name) {
    const std::string&& _loggerCat = "SharedMemory(" + name + ")";
#ifdef WIN32
    HANDLE handle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name.c_str());
    if (handle != NULL) {
        // the file exists, so we have to close it immediately to not leak the handle
        CloseHandle(handle);
        return true;
    }
    else {
        // The handle doesn't exist, which can mean two things: the memory mapped file
        // doesn't exist or it exists but there was an error accessing it
        const DWORD error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND)
            return false;
        else {
            const std::string&& errorMsg = lastErrorToString(error);
            LERROR_SAFE("Error checking if shared memory existed: " << errorMsg);
            return true;
        }
    }
#else
    const unsigned int h = hash(name);
    const int result = shmget(h, 0, IPC_EXCL);
    return result != -1;
#endif
}
    
SharedMemory::SharedMemory(const std::string& name)
    : _memory(nullptr)
    , _loggerCat("SharedMemory(" + name + ")")
    , _status(STATUS_SUCCESS)
#ifndef WIN32
    , _size(0)
#endif
{
#ifdef WIN32
    _sharedMemoryHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name.c_str());
    if (_sharedMemoryHandle == NULL) {
        const DWORD error = GetLastError();
        const std::string&& errorMsg = lastErrorToString(error);
        LERROR_SAFE("Error occurred while accessing shared memory: " << errorMsg);
        _status |= STATUS_NOT_FOUND;
        return;
    }

    _memory = MapViewOfFileEx(_sharedMemoryHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0, NULL);
    if (_memory == nullptr) {
        const DWORD error = GetLastError();
        const std::string&& errorMsg = lastErrorToString(error);
        LERROR_SAFE("Error occurred while creating view for shared memory: " <<
                    errorMsg);
        _status |= STATUS_NO_MAPPING;
        return;
    }
#else
    const unsigned int h = hash(name);
    _sharedMemoryHandle = shmget(h, 0, IPC_R | IPC_W | IPC_M);
    if (_sharedMemoryHandle == -1) {
        LERROR_SAFE("Error occurred while accessing shared memory: " << strerror(errno));
        _status |= STATUS_NOT_FOUND;
    }
    else {
        _memory = shmat(_sharedMemoryHandle, NULL, SHM_R | SHM_W);

        // TODO: Error handling (STATUS_NO_MAPPING)
        
        struct shmid_ds sharedMemoryInfo;
        shmctl(_sharedMemoryHandle, IPC_STAT, &sharedMemoryInfo);
        _size = sharedMemoryInfo.shm_segsz  - sizeof(Header);
    }
#endif
}
    
SharedMemory::~SharedMemory() {
#ifdef WIN32
    BOOL result;
    if ((_status & STATUS_NOT_FOUND) != 0) {
        result = CloseHandle(_sharedMemoryHandle);
        if (result == 0) {
            const DWORD error = GetLastError();
            const std::string&& errorMsg = lastErrorToString(error);
            LERROR_SAFE("Error closing handle: " << errorMsg);
        }
    }
    if ((_status & STATUS_NO_MAPPING) != 0) {
        result = UnmapViewOfFile(_memory);
        if (result == 0) {
            const DWORD error = GetLastError();
            const std::string&& errorMsg = lastErrorToString(error);
            LERROR_SAFE("Error unmapping view: " << errorMsg);
        }
    }
    _memory = nullptr;
#else
    if (_sharedMemoryHandle != -1) {
        const int result = shmdt(_memory);
        if (result == -1)
            LERROR_SAFE("Error detaching shared memory: " << strerror(errno));
        _memory = nullptr;
    }
#endif
}
    
SharedMemory::operator void*() {
    return reinterpret_cast<void*>(reinterpret_cast<char*>(_memory) + sizeof(Header));
}
    
size_t SharedMemory::size() const {
#ifdef WIN32
    if (_memory != nullptr) {
        const Header* const h = header(_memory);
        return h->size;
    }
    else
        return 0;
#else
    return _size;
#endif
}
    
void SharedMemory::acquireLock() {
    if (_memory != nullptr) {
        Header* h = header(_memory);
        while (h->mutex.test_and_set()) {}
    }
}
    
void SharedMemory::releaseLock() {
    if (_memory != nullptr) {
        Header* h = header(_memory);
        h->mutex.clear();
    }
}
    
} // namespace ghoul
