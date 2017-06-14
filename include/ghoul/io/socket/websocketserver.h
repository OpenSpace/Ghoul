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

#ifndef __GHOUL___WEBSOCKETSERVER___H__
#define __GHOUL___WEBSOCKETSERVER___H__

#include <libwebsockets.h>

#include <ghoul/io/socket/socketserver.h>
#include <ghoul/io/socket/websocket.h>

#include <string>
#include <memory>
#include <thread>

namespace ghoul {
namespace io {

class WebSocketServer : public SocketServer {
public:
    WebSocketServer();
    virtual ~WebSocketServer();
    virtual std::string address() const;
    virtual int port() const;
    virtual void close();
    virtual void listen(std::string address, int port);
    virtual bool isListening() const;
    virtual bool hasPendingSockets() const;
    // Get next pending connection. Non-blocking. Can return nullptr.
    std::unique_ptr<WebSocket> nextPendingWebSocket();
    virtual std::unique_ptr<Socket> nextPendingSocket();

    // Get next pending connection. Blocking.
    // Only returns nullptr if the socket server closes.
    std::unique_ptr<WebSocket> awaitPendingWebSocket();
    virtual std::unique_ptr<Socket> awaitPendingSocket();

private:
    std::string _address;
    int _port;
    bool _listening = false;

    std::unique_ptr<std::thread> _serverThread;

    void waitForConnections();
};


} // namespace io
} // namespace ghoul

#endif // __GHOUL___WEBSOCKETSERVER___H__
