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

#ifndef __GHOUL___WEBSOCKET___H__
#define __GHOUL___WEBSOCKET___H__

#include <ghoul/io/socket/tcpsocket.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/exception.h>
#include <websocketpp/config/core.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/common/functional.hpp>
#include <set>

namespace ghoul {
namespace io {

typedef websocketpp::server<websocketpp::config::core> WsServer;

/**
 * WebSockets are essentially a wrapper around regular TCP sockets.
 * The difference is how the messages are interpeted. In TCP sockets,
 * we delimit the messages using `_delimiter`.
 *
 * In WebSockets however, the message delimiting is handled by a
 * message header.
 */
class WebSocket : public TcpSocket {
public:
    struct WebSocketError : public ghoul::RuntimeError {
        explicit WebSocketError(std::string message, std::string component = "");
    };

    /**
     * WebSocket close event reason: https://developer.mozilla.org/en-US/docs/Web/API/CloseEvent
     */
    struct ClosingReason {
        static const enum CLOSING_REASON {
            Normal      = 1000,
            GoingAway   = 1001,
            // custom app-specific reasons
            ClosingAll  = 4000
        };
    };

    WebSocket(std::string address, int portNumber);
    WebSocket(std::string address, int portNumber, _SOCKET socket);

    virtual ~WebSocket();
    void disconnect(const int reason = WebSocket::ClosingReason::Normal);

    bool getMessage(std::string& message) override;
    bool putMessage(const std::string& message) override;
    void startStreams();

private:
    void streamInput();
    void streamOutput();
    bool waitForOutput(size_t nBytes);
    bool waitForInput(size_t nBytes = 0);
    int errorCode();

    bool socketHasConnection();

    std::thread _inputThread;
    std::thread _outputThread;

    WsServer server;

    WsServer::connection_ptr socketConnection;
    void onMessage(websocketpp::connection_hdl hdl, WsServer::message_ptr msg);
    void onOpen(websocketpp::connection_hdl hdl);
    void onClose(websocketpp::connection_hdl hdl);

    std::mutex _connectionHandlesMutex;
    std::set<websocketpp::connection_hdl,std::owner_less<websocketpp::connection_hdl> > _connectionHandles;

    std::mutex _inputBufferMutex;
    std::mutex _inputQueueMutex;
    std::condition_variable _inputNotifier;
    std::deque<std::string> _inputQueue;
    std::array<char, 4096> _inputBuffer;

    std::stringstream _outputStream;
    std::mutex _outputStreamMutex;
    std::mutex _outputBufferMutex;
    int outputStreamSize();
    std::mutex _outputQueueMutex;
    std::condition_variable _outputNotifier;
};


} // namespace io
} // namespace ghoul

#endif // __GHOUL___WEBSOCKET___H__
