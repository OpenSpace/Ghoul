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

#ifdef _WIN32

#else
#include <sys/shm.h>
#endif

namespace ghoul {

namespace {
    const std::string _loggerCat = "SharedMemory";
    struct Header {
        std::atomic_flag mutex;
    };
    
    Header* header(void* memory) {
        return reinterpret_cast<Header*>(memory);
    }
    
    unsigned int hash(const std::string& name) {
        return hashCRC32(name);
    }
}
    
void SharedMemory::create(const std::string& name, size_t size) {
    const unsigned int h = hash(name);
#ifdef _WIN32
    
#else
    const int result = shmget(h, size, IPC_CREAT | IPC_EXCL | IPC_R | IPC_W | IPC_M);
    if (result == -1)
        LERROR_SAFE("Error occurred while creating shared memory: '" <<
                    strerror(errno) << "'");
#endif
}
 
void SharedMemory::remove(const std::string& name) {
    const unsigned int h = hash(name);
#ifdef _WIN32
    
#else
    int result = shmget(h, 0, IPC_R | IPC_W | IPC_M);
    if (result == -1)
        LERROR_SAFE("Error occurred while retrieving shared memory: '" <<
                    strerror(errno) << "'");
    else {
        result = shmctl(result, IPC_RMID, nullptr);
        if (result == -1)
            LERROR_SAFE("Error occurred while removing shared memory: '" <<
                        strerror(errno) << "'");
    }
#endif
}
    
bool SharedMemory::exists(const std::string& name) {
    const unsigned int h = hash(name);
#ifdef _WIN32
    
#else
    const int result = shmget(h, 0, IPC_EXCL);
        return result != -1;
#endif
}
    
SharedMemory::SharedMemory(const std::string& name)
    : _memory(nullptr)
    , _size(0)
{
    const unsigned int h = hash(name);
#ifdef _WIN32
    
#else
    _sharedMemoryHandle = shmget(h, 0, IPC_R | IPC_W | IPC_M);
    if (_sharedMemoryHandle == -1)
        LERROR_SAFE("Error occurred while accessing shared memory: '" <<
                    strerror(errno) << "'");
    else {
        _memory = shmat(_sharedMemoryHandle, NULL, SHM_R | SHM_W);
        
        struct shmid_ds sharedMemoryInfo;
        shmctl(_sharedMemoryHandle, IPC_STAT, &sharedMemoryInfo);
        _size = sharedMemoryInfo.shm_segsz;
    }
#endif
}
    
SharedMemory::~SharedMemory() {
#ifdef _WIN32
    
#else
    if (_sharedMemoryHandle != -1) {
        const int result = shmdt(_memory);
        if (result == -1)
            LERROR_SAFE("Error detaching shared memory '" << strerror(errno) << "'");
        _memory = nullptr;
    }
#endif
}
    
SharedMemory::operator void*() {
    return reinterpret_cast<void*>(reinterpret_cast<char*>(_memory) + sizeof(Header));
}
    
size_t SharedMemory::size() const {
    return _size;
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
