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

#include <ghoul/io/socket/websocket.h>

#include <ghoul/logging/logmanager.h>
#include <ghoul/fmt.h>
#include <chrono>
#include <functional>
#include <websocketpp/common/functional.hpp>

namespace {
    constexpr const char* _loggerCat = "WebSocket";
    constexpr const std::chrono::milliseconds MaxWaitDuration(1000);
} // namespace

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

namespace ghoul::io {

WebSocket::WebSocketError::WebSocketError(std::string msg, std::string comp)
    : RuntimeError(std::move(msg), std::move(comp))
{}

WebSocket::WebSocket(std::unique_ptr<TcpSocket> socket,
    websocketpp::server<websocketpp::config::core>& server
) {
    _tcpSocket = std::move(socket);

    server.set_message_handler(bind(&WebSocket::onMessage, this, ::_1, ::_2));
    server.set_open_handler(bind(&WebSocket::onOpen, this, ::_1));
    server.set_close_handler(bind(&WebSocket::onClose, this, ::_1));

    _socketConnection = server.get_connection();
    _socketConnection->register_ostream(&_outputStream);
    _socketConnection->start();

     _tcpSocket->interceptInput(
        [this](const char* data, size_t nBytes) {
            _socketConnection->read_some(data, nBytes);
            _inputNotifier.notify_one();
        }
    );
}

WebSocket::~WebSocket() {
    LDEBUG("Destroying socket connection");
    _socketConnection->eof();
    _tcpSocket = nullptr;
}

std::string WebSocket::address() const {
    return _tcpSocket->address();
}

int WebSocket::port() const {
    return _tcpSocket->port();
}

void WebSocket::disconnect(int) {
    _tcpSocket->disconnect();
}

bool WebSocket::getMessage(std::string& message) {
    auto messageOrDisconnected = [this]() {
        // `_inputMessageQueueMutex` must be locked when calling this function.
        return (!_tcpSocket->isConnected() && !_tcpSocket->isConnecting()) ||
            !_inputMessageQueue.empty();
    };

    while (!messageOrDisconnected()) {
        std::unique_lock lock(_inputMessageQueueMutex);
        _inputNotifier.wait_for(lock, MaxWaitDuration, messageOrDisconnected);
    }

    std::lock_guard<std::mutex> guard(_inputMessageQueueMutex);
    if (_inputMessageQueue.empty()) {
        return false;
    }

    message = _inputMessageQueue.front();
    _inputMessageQueue.pop_front();
    return true;
}

bool WebSocket::putMessage(const std::string &message) {
    _socketConnection->send(message);
    _tcpSocket->put<char>(_outputStream.str().c_str(), _outputStream.str().size());
    _outputStream.str("");
    return true;
}

bool WebSocket::isConnected() const {
    return _tcpSocket && _tcpSocket->isConnected();
}

bool WebSocket::isConnecting() const {
    return _tcpSocket && _tcpSocket->isConnecting();
}

void WebSocket::startStreams() {
    _tcpSocket->startStreams();
}

/**
 * Callback for incoming messages
 * \param hdl A handle to uniquely identify a connection.
 * \param msg The message
 */
void WebSocket::onMessage(const websocketpp::connection_hdl&,
                   const websocketpp::server<websocketpp::config::core>::message_ptr& msg)
{
    std::string msgContent = msg->get_payload();
    std::lock_guard<std::mutex> guard(_inputMessageQueueMutex);
    _inputMessageQueue.push_back(msgContent);
    _inputNotifier.notify_one();
}

void WebSocket::onOpen(const websocketpp::connection_hdl& hdl) {
    LDEBUG(fmt::format(
        "onOpen: WebSocket opened. Client: {}:{}.",
        _tcpSocket->address(),
        _tcpSocket->port()
    ));
    std::lock_guard guard(_connectionHandlesMutex);
    _connectionHandles.insert(hdl);
    _tcpSocket->put<char>(_outputStream.str().c_str(), _outputStream.str().size());
    _outputStream.str("");
}

void WebSocket::onClose(const websocketpp::connection_hdl& hdl) {
    LDEBUG(fmt::format(
        "onClose: WebSocket closing. Client: {}:{}.",
        _tcpSocket->address(),
        _tcpSocket->port()
    ));

    std::lock_guard guard(_connectionHandlesMutex);
    _connectionHandles.erase(hdl);
    _inputNotifier.notify_one();
}

}  // namespace ghoul::io
