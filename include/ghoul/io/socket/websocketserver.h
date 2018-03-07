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

#ifndef __GHOUL___WEBSOCKETSERVER___H__
#define __GHOUL___WEBSOCKETSERVER___H__

#include <ghoul/misc/exception.h>
#include <ghoul/io/socket/socketserver.h>
#include <ghoul/io/socket/websocket.h>
#include <atomic>
#include <ghoul/fmt.h>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace ghoul::io {

class WebSocketServer : public SocketServer {
public:
    virtual ~WebSocketServer();

    std::string address() const override;
    int port() const override;
    void close() override;
    void listen(std::string address, int port) override;
    bool isListening() const override;
    bool hasPendingSockets() const override;

    // Get next pending connection. Non-blocking. Can return nullptr.
    std::unique_ptr<WebSocket> nextPendingWebSocket();
    std::unique_ptr<Socket> nextPendingSocket() override;

    // Get next pending connection. Blocking.
    // Only returns nullptr if the socket server closes.
    std::unique_ptr<WebSocket> awaitPendingWebSocket();
    std::unique_ptr<Socket> awaitPendingSocket() override;

private:
    void waitForConnections();

    std::string _address;
    int _port;
    bool _listening = false;

    std::unique_ptr<std::thread> _serverThread;
    _SOCKET _serverSocket;

    mutable std::mutex _settingsMutex;
    mutable std::mutex _connectionMutex;
    mutable std::mutex _connectionNotificationMutex;

    std::condition_variable _clientConnectionNotifier;

    std::deque<std::unique_ptr<WebSocket>> _pendingClientConnections;
};

} // namespace ghoul::io

#endif // __GHOUL___WEBSOCKETSERVER___H__
