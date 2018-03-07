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

#ifndef __GHOUL___SOCKET___H__
#define __GHOUL___SOCKET___H__

#include <atomic>
#include <memory>
#include <string>

namespace ghoul::io {
    
class Socket {
public:
    Socket();
    virtual void disconnect(int reason = 0) = 0;
    virtual bool isConnected() = 0;
    virtual bool isConnecting() = 0;
    virtual void startStreams() = 0;
    int socketId() const;

    /**
     * Methods for binary communication
     */
    template <typename T = char>
    bool get(T* buffer, size_t nItems = 1);

    template <typename T = char>
    bool peek(T* buffer, size_t nItems = 1);

    template <typename T = char>
    bool skip(size_t nItems = 1);

    template <typename T = char>
    bool put(const T* buffer, size_t nItems = 1);

    /**
     * Methods for text based communication.
     */
    virtual bool getMessage(std::string& message) = 0;
    virtual bool putMessage(const std::string& message) = 0;

protected:
    /**
     * Methods for binary communication
     */
    virtual bool getBytes(char* buffer, size_t nItems) = 0;
    virtual bool peekBytes(char* buffer, size_t nItems) = 0;
    virtual bool skipBytes(size_t nItems) = 0;
    virtual bool putBytes(const char* buffer, size_t nItems) = 0;

private:
    const int _socketId;
    static std::atomic<int> _nextSocketId;
};

} // namespace ghoul::io

#include <ghoul/io/socket/socket.inl>

#endif //  __GHOUL___SOCKET___H__
