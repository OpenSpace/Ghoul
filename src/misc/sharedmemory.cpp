/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2018                                                               *
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

#include <ghoul/misc/crc32.h>
#include <atomic>

#ifndef WIN32
#include <errno.h>
#include <string.h>
#include <sys/shm.h>
/* Common access type bits, used with ipcperm(). */
#define IPC_R       000400  /* read permission */
#define IPC_W       000200  /* write/alter permission */
#define IPC_M       010000  /* permission to change control info */
#else
#include <Windows.h>
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

    Header* header(void* memory) {
        return reinterpret_cast<Header*>(memory);
    }

    unsigned int hash(const std::string& name) {
        return hashCRC32(name);
    }

#ifdef WIN32
    std::string lastErrorToString(DWORD error) {
        LPTSTR errorBuffer = nullptr;
        DWORD nValues = FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&errorBuffer,
            0,
            NULL
        );
        if ((nValues > 0) && (errorBuffer != nullptr)) {
            std::string errorMsg(errorBuffer);
            LocalFree(errorBuffer);
            return errorMsg;
        }
        else {
            return std::string("Error constructing format message for error: ") +
                std::to_string(error);
        }
    }
#endif
}

SharedMemory::SharedMemoryError::SharedMemoryError(std::string msg)
    : RuntimeError(std::move(msg), "SharedMemory")
{}

SharedMemory::SharedMemoryNotFoundError::SharedMemoryNotFoundError()
    : SharedMemoryError("Shared memory did not exist")
{}

void SharedMemory::create(const std::string& name, size_t size) {
    // adjust for the header size
    size += sizeof(Header);
#ifdef WIN32
    HANDLE handle = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        static_cast<DWORD>(size),
        name.c_str()
    );
    const DWORD error = GetLastError();
    if (handle == NULL) {
        std::string errorMsg = lastErrorToString(error);
        throw SharedMemoryError(
            "Error creating shared memory '" + name + "': " + errorMsg
        );
    }
    else {
        if (error == ERROR_ALREADY_EXISTS) {
            throw SharedMemoryError(
                "Error creating shared memory '" + name + "': Section exists"
            );
        }
        else {
            void* memory = MapViewOfFileEx(handle, FILE_MAP_ALL_ACCESS, 0, 0, 0, NULL);

            if (memory == nullptr) {
                std::string errorMsg = lastErrorToString(error);

                throw SharedMemoryError(
                    "Error creating a view on shared memory '" + name + "': " + errorMsg
                );
            }
            else {
                Header* h = header(memory);
                h->mutex.clear();
                h->size = size  - sizeof(Header);
                UnmapViewOfFile(memory);
                _createdSections[name] = handle;
            }
        }
    }
#else
    unsigned int h = hash(name);
    int result = shmget(h, size, IPC_CREAT | IPC_EXCL | IPC_R | IPC_W | IPC_M);
    if (result == -1) {
        std::string errorMsg = strerror(errno);
        throw SharedMemoryError(
            "Error creating shared memory '" + name + "': " + errorMsg
        );
    }
    void* memory = shmat(result, nullptr, SHM_R | SHM_W);
    Header* memoryHeader = header(memory);

    memoryHeader->mutex.clear();
    shmdt(memory);
#endif
}

void SharedMemory::remove(const std::string& name) {
#ifdef WIN32
    if (_createdSections.find(name) == _createdSections.end()) {
        throw SharedMemoryNotFoundError();
    }

    HANDLE h = _createdSections[name];
    _createdSections.erase(name);
    BOOL result = CloseHandle(h);
    if (result == 0) {
        DWORD error = GetLastError();
        std::string errorMsg = lastErrorToString(error);
        throw SharedMemoryError("Error closing handle: " + errorMsg);
    }
#else
    unsigned int h = hash(name);
    int result = shmget(h, 0, IPC_R | IPC_W | IPC_M);
    if (result == -1) {
        std::string errorMsg = strerror(errno);
        throw SharedMemoryError("Error while retrieving shared memory: " + errorMsg);
    }
    else {
        result = shmctl(result, IPC_RMID, nullptr);
        if (result == -1) {
            std::string errorMsg = strerror(errno);
            throw SharedMemoryError("Error while removing shared memory: " + errorMsg);
        }
    }
#endif
}

bool SharedMemory::exists(const std::string& name) {
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
        if (error == ERROR_FILE_NOT_FOUND) {
            return false;
        }
        else {
            std::string errorMsg = lastErrorToString(error);
            throw SharedMemoryError(
                "Error checking if shared memory exists: " + errorMsg
            );
        }
    }
#else
    unsigned int h = hash(name);
    int result = shmget(h, 0, IPC_EXCL);
    return result != -1;
#endif
}

SharedMemory::SharedMemory(std::string name)
    : _memory(nullptr)
    , _name(std::move(name))
#ifndef WIN32
    , _size(0)
#endif
{
#ifdef WIN32
    _sharedMemoryHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _name.c_str());
    if (_sharedMemoryHandle == nullptr) {
        std::string errorMsg = lastErrorToString(GetLastError());
        throw SharedMemoryError(
            "Error accessing shared memory '" + name + "': " + errorMsg
        );
    }

    _memory = MapViewOfFileEx(_sharedMemoryHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0, NULL);
    if (_memory == nullptr) {
        CloseHandle(_sharedMemoryHandle);

        std::string errorMsg = lastErrorToString(GetLastError());
        throw SharedMemoryError(
            "Error creating view for shared memory '" + name + "': " + errorMsg
        );
    }
#else
    unsigned int h = hash(_name);
    _sharedMemoryHandle = shmget(h, 0, IPC_R | IPC_W | IPC_M);
    if (_sharedMemoryHandle == -1) {
        std::string errorMsg = strerror(errno);
        throw SharedMemoryError(
            "Error accessing shared memory '" + name + "': " + errorMsg
        );
    }
    else {
        _memory = shmat(_sharedMemoryHandle, nullptr, SHM_R | SHM_W);
        if (_memory == reinterpret_cast<void*>(-1)) {
            std::string errorMsg = strerror(errno);
            throw SharedMemoryError(
                "Error mapping shared memory '" + name + "':" + errorMsg
            );
        }

        struct shmid_ds sharedMemoryInfo;
        shmctl(_sharedMemoryHandle, IPC_STAT, &sharedMemoryInfo);
        _size = sharedMemoryInfo.shm_segsz  - sizeof(Header);
    }
#endif
}

SharedMemory::~SharedMemory() {
#ifdef WIN32
    CloseHandle(_sharedMemoryHandle);
    UnmapViewOfFile(_memory);
#else
    shmdt(_memory);
#endif
}

void* SharedMemory::memory() const {
    return reinterpret_cast<void*>(reinterpret_cast<char*>(_memory) + sizeof(Header));
}

size_t SharedMemory::size() const {
#ifdef WIN32
    return header(_memory)->size;
#else
    return _size;
#endif
}

std::string SharedMemory::name() const {
    return _name;
}

void SharedMemory::acquireLock() {
    Header* h = header(_memory);
    while (h->mutex.test_and_set()) {}
}

void SharedMemory::releaseLock() {
    Header* h = header(_memory);
    h->mutex.clear();
}

} // namespace ghoul
