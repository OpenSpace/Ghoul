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

#ifndef __GHOUL___SOCKETSERVER___H__
#define __GHOUL___SOCKETSERVER___H__

#include <memory>
#include <string>

namespace ghoul::io {

class Socket;

class SocketServer {
public:
    virtual ~SocketServer() = default;

    virtual int port() const = 0;
    virtual void close() = 0;
    virtual void listen(int port) = 0;
    virtual bool isListening() const = 0;
    virtual bool hasPendingSockets() const = 0;
    // Get next pending connection. Non-blocking. Can return nullptr.
    virtual std::unique_ptr<Socket> nextPendingSocket() = 0;

    // Get next pending connection. Blocking.
    // Only returns nullptr if the socket server closes.
    virtual std::unique_ptr<Socket> awaitPendingSocket() = 0;
};

} // namespace ghoul::io

#endif // __GHOUL___SOCKETSERVER___H__
