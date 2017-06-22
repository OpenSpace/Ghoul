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

#include <ghoul/io/socket/websocket.h>
#include <fmt/format.h>

namespace {
    const std::string _loggerCat = "WebSocket";
}

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

namespace ghoul {
namespace io {

WebSocket::WebSocketError::WebSocketError(std::string message, std::string component)
        : RuntimeError(message, component) {}

WebSocket::WebSocket(std::string address, int port) : TcpSocket(address, port) {
    LDEBUG(fmt::format("WebSocket started on {}:{}.", address, port));
    // starting ws server needs to be handled here
}

WebSocket::WebSocket(std::string address, int port, _SOCKET socket)
        : TcpSocket(address, port, socket)
{
    LDEBUG(fmt::format("WebSocket started on port {}. Client: {}. Socket provided.", port, address));
//    server.set_message_handler(bind(&WebSocket::onMessage, &server, ::_1, ::_2));

    server.set_message_handler([this](websocketpp::connection_hdl hdl, WsServer::message_ptr msg) {
        onMessage(&server, hdl, msg);
    });
    server.set_close_handler([this](websocketpp::connection_hdl hdl) {
        _shouldDisconnect = true;
    });

    server.register_ostream(&outputStream);
    socketConnection = server.get_connection();
    socketConnection->start();

    startStreams();
}

WebSocket::~WebSocket() {
    socketConnection->eof();
//     socketConnection->terminate();
}

void WebSocket::startStreams() {
    _inputThread = std::thread(
            [this]() { streamInput(); }
    );
    _outputThread = std::thread(
            [this]() { streamOutput(); }
    );
}

bool WebSocket::getMessage(std::string &message) {
    return false;
}

bool WebSocket::putMessage(const std::string &message) {
    if (!_isConnected) {
        LERROR("Cannot send message when not connected.");
        return false;
    }
    outputStream << message;
    return true;
}

void WebSocket::streamInput() {
    // feed to server somehow
    int nReadBytes = 0;

    while (_isConnected && !_shouldDisconnect) {
        std::lock_guard<std::mutex> inputBufferGuard(_inputBufferMutex);

        nReadBytes = recv(
                _socket,
                _inputBuffer.data(),
                static_cast<int>(_inputBuffer.size()),
                0);

        if (nReadBytes <= 0) {
            _error = true;
            _shouldDisconnect = true;
            _inputNotifier.notify_one();
            return;
        }

        socketConnection->read_some(_inputBuffer.data(), nReadBytes);
    }
}

void WebSocket::streamOutput() {

}

/**
 * Callback for incoming messages
 * @param s   - server pointer
 * @param hdl - A handle to uniquely identify a connection.
 * @param msg - the message
 */
void WebSocket::onMessage(WsServer *s, websocketpp::connection_hdl hdl, WsServer::message_ptr msg) {
    std::string msgContent = msg->get_payload();
    LDEBUG(fmt::format("Message received: {}", msgContent));
    std::lock_guard<std::mutex> guard(_inputQueueMutex);
    // store message
    _inputNotifier.notify_one();
}

}
}
