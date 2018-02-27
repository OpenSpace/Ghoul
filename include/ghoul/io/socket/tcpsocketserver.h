/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2017                                                               *
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

#ifndef __GHOUL___TCPSOCKETSERVER___H__
#define __GHOUL___TCPSOCKETSERVER___H__

#include <ghoul/misc/exception.h>
#include <ghoul/io/socket/tcpsocket.h>
#include <ghoul/io/socket/socketserver.h>

#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

namespace ghoul::io {

class TcpSocketServer : public SocketServer {
public:
    TcpSocketServer();
    virtual ~TcpSocketServer();
    virtual std::string address() const;
    virtual int port() const;
    virtual void close();
    virtual void listen(std::string address, int port);
    virtual bool isListening() const;
    virtual bool hasPendingSockets() const;
    std::unique_ptr<TcpSocket> nextPendingTcpSocket();
    virtual std::unique_ptr<Socket> nextPendingSocket();

    // Blocking methods
    std::unique_ptr<TcpSocket> awaitPendingTcpSocket();
    virtual std::unique_ptr<Socket> awaitPendingSocket();
private:
    void closeSocket(_SOCKET socket);

    mutable std::mutex _settingsMutex;
    std::string _address;
    int _port;
    bool _listening;

    mutable std::mutex _connectionMutex;
    std::deque<std::unique_ptr<TcpSocket>> _pendingConnections;

    std::mutex _connectionNotificationMutex;
    std::condition_variable _connectionNotifier;

    std::unique_ptr<std::thread> _serverThread;
    _SOCKET _serverSocket;
    void waitForConnections();
    void setOptions(_SOCKET socket);
};

} // namespace ghoul::io

#endif // __GHOUL___TCPSOCKETSERVER___H__
