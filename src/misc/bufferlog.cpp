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

#include <ghoul/misc/bufferlog.h>

#include <ghoul/logging/logmanager.h>
#include <ghoul/glm.h>
#include <atomic>
#include <cassert>
#include <fstream>
#include <string.h>

//todo semaphore

namespace ghoul {
    
namespace {
    const glm::detail::uint8 CURRENT_VERSION = 1;
    
    const std::string _loggerCat = "BufferLog";
    
    struct Header {
        /**
         * The version header contains in increasing unsigned integer, which specifies the
         * general layout of the buffer in this BufferLog. The size of the header and,
         * thus, the offset into the data block may depend on the version.
         */
        glm::detail::uint8 version;
        
        /**
         * This atomic is set to <code>true</code> if some process is currently writing to
         * the log, otherwise it is <code>false</code>. It is not guaranteed that this
         * value is usable when the buffer is written to disk in its entirety.
         */
        std::atomic_flag mutex;
        
        /**
         * The attributes are used for user-defined behavior. Information that is
         * necessary to interpret the buffer may be put in here.
         */
        glm::detail::uint8 attributes;
        
        /**
         * This byte is not used by the BufferLog at the moment and are added to force the
         * compiler to a 4 byte alignment. The usage of this padding might change in the
         * future.
         */
        unsigned char __padding[1];
        
        /**
         * This value provides an offset to find the first byte in the buffer that has not
         * been used already. The values between <code>_buffer + sizeof(Header)</code> and
         * <code>_buffer + sizeof(Header) + firstEmptyByte</code> are the logs that have 
         * been stored before.
         */
        glm::detail::uint32 firstEmptyByte;
    };
    
    /**
     * This method returns the beginning part of the buffer that contains the header
     * information.
     */
    Header* header(void* buffer) {
        return reinterpret_cast<Header*>(buffer);
    }
    
    /**
     This method returns the buffer block that begins at the first empty byte. This offset
     * is guaranteed to be in valid memory if the values provided in the constructor or
     * the \see setBuffer function were correct.
     */
    void* firstEmptyMemory(void* buffer) {
        const Header* const h = header(buffer);
        
        return reinterpret_cast<void*>(
                 reinterpret_cast<char*>(buffer) + sizeof(Header) + h->firstEmptyByte
                                       );
    }
}
    
BufferLog::BufferLog(void* address, const size_t totalSize)
    : _buffer(address)
    , _totalSize(totalSize)
    , _inCallbackStack(false)
{
    initializeBuffer();
    Header* h = header(_buffer);
    h->mutex.clear();
}
    
BufferLog::BufferLog(void* address, const size_t totalSize,
                     const MemoryExhaustedCallback& callback)
    : _buffer(address)
    , _totalSize(totalSize)
    , _callback(callback)
    , _inCallbackStack(false)
{
    initializeBuffer();
    Header* h = header(_buffer);
    h->mutex.clear();
}
    
void BufferLog::initializeBuffer() {
    assert(_buffer != nullptr);
    Header* h = header(_buffer);
    h->version = CURRENT_VERSION;
    h->attributes = 0;
    h->firstEmptyByte = 0;
}
    
void BufferLog::setCallback(const MemoryExhaustedCallback& callback) {
    _callback = callback;
}
    
void BufferLog::resetBuffer() {
    // Resetting the buffer does not overwrite any of the data but resets the
    // firstEmptyByte pointer so that following log will overwrite the buffer
    Header* h = header(_buffer);
    
    // if test_and_set returns 'true', someone else is in the critical section
    // unless we are currently in a callstack containing the callback function. In which
    // case we are already safe
    while (h->mutex.test_and_set() && !_inCallbackStack) {}
    // the moment we return, we own the mutex
    h->firstEmptyByte = 0;
    if (!_inCallbackStack)
        // only release the mutex if we are not in the callback stack
        h->mutex.clear();
}
    
bool BufferLog::log(unsigned long long int timestamp, const char* message) {
    Header* h = header(_buffer);
    // This is the full size of the incoming message. +1 for the terminating \0 character
    const size_t fullSize = sizeof(unsigned long long int) + strlen(message) + 1;
    
    // if test_and_set returns 'true', someone else is in the critical section
    while (h->mutex.test_and_set()) {}
    // the moment we return, we own the mutex
    
    // If this message would exceed the available memory...
    if (h->firstEmptyByte + sizeof(Header) + fullSize > _totalSize) {
        // ... and we have a valid callback function
        if (_callback) {
            // delegate the cleaning up to the callback
            _inCallbackStack = true;
            _callback(*this, timestamp);
            _inCallbackStack = false;
            if (h->firstEmptyByte + fullSize > _totalSize) {
                // The callback failed to clear the memory
                LERROR("Memory log is full. Callback has to clear enough memory");
                h->mutex.clear();
                return false;
            }
        }
        else {
            // We have to fail if there is no callback
            LERROR("Memory log is full");
            h->mutex.clear();
            return false;
        }
    }
    // Copy the values of the timestamp
    memcpy(firstEmptyMemory(_buffer),
           reinterpret_cast<void*>(&timestamp), sizeof(unsigned long long int));
    // Advance the empty pointer
    h->firstEmptyByte += sizeof(unsigned long long int);
    // Copy the message into the buffer, strcpy will copy the \0 terminator character, too
    char* destination = reinterpret_cast<char*>(firstEmptyMemory(_buffer));
#ifdef WIN32
    strcpy_s(destination, strlen(message) + 1, message);
#else
    strcpy(destination, message);
#endif
    // Advance the empty pointer; +1 for the \0 terminator
    h->firstEmptyByte += glm::detail::uint32(strlen(message) + 1);
    h->mutex.clear();
    return true;
}
    
void* BufferLog::buffer() {
    return _buffer;
}

size_t BufferLog::totalSize() const {
    return _totalSize;
}
    
size_t BufferLog::usedSize() const {
    const Header* const h = header(_buffer);
    return h->firstEmptyByte + sizeof(Header);
}

void BufferLog::setBuffer(void* buffer, size_t totalSize) {
    Header* h = header(_buffer);
    // if test_and_set returns 'true', someone else is in the critical section
    // unless we are currently in a callstack containing the callback function. In which
    // case we are already safe
    while (h->mutex.test_and_set() && !_inCallbackStack) {}
    // the moment we return, we own the mutex

    _buffer = buffer;
    _totalSize = totalSize;
    initializeBuffer();
    
    if (!_inCallbackStack)
        // only release the mutex if we are not in the callback stack
        h->mutex.clear();
}
    
void BufferLog::writeToDisk(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.good())
        LERROR("Error opening file '" + filename + "'");
    else {
        Header* h = header(_buffer);
        std::cout << h->firstEmptyByte << std::endl;
        // if test_and_set returns 'true', someone else is in the critical section
        // unless we are currently in a callstack containing the callback function. In
        // which case we are already safe
        while (h->mutex.test_and_set() && !_inCallbackStack) {}
        // the moment we return, we own the mutex
        file.write(reinterpret_cast<char*>(_buffer), usedSize());
        if (!_inCallbackStack)
            // only release the mutex if we are not in the callback stack
            h->mutex.clear();
    }
}

} // namespace ghoul
