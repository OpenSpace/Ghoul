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
#include <fmt/format.h>
#include <websocketpp/common/functional.hpp>

namespace {
    constexpr const char* _loggerCat = "WebSocket";
} // namespace

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

namespace ghoul::io {

WebSocket::WebSocketError::WebSocketError(std::string msg, std::string comp)
    : RuntimeError(std::move(msg), std::move(comp))
{}

WebSocket::WebSocket(std::string address, int port) : TcpSocket(address, port) {
    LDEBUG(fmt::format("WebSocket started on {}:{}.", address, port));
    // starting ws server needs to be handled here
}

WebSocket::WebSocket(std::string address, int port, _SOCKET socket)
    : TcpSocket(address, port, socket)
{
    _server.set_message_handler(bind(&WebSocket::onMessage, this, ::_1, ::_2));
    _server.set_open_handler(bind(&WebSocket::onOpen,this,::_1));
    _server.set_close_handler(bind(&WebSocket::onClose,this,::_1));

    // set up WebSocket++ logging
    _server.clear_access_channels(websocketpp::log::alevel::all);
    _server.set_access_channels(websocketpp::log::alevel::connect);
    _server.set_access_channels(websocketpp::log::alevel::disconnect);
    _server.set_access_channels(websocketpp::log::alevel::app);

    _server.register_ostream(&_outputStream);
    _socketConnection = _server.get_connection();
    _socketConnection->start();

    startStreams();
    LDEBUG(fmt::format("WebSocket started. Client: {}:{}.", address, port));
}

WebSocket::~WebSocket() {
    LDEBUG("Destroying socket connection");
    _socketConnection->eof();
}

void WebSocket::startStreams() {
    _inputThread = std::thread([this]() { streamInput(); } );
    _outputThread = std::thread([this]() { streamOutput(); });
}

void WebSocket::disconnect(int reason) {
    if (!_isConnected) return;

    if (socketHasConnection()) {
        _socketConnection->close(
            static_cast<websocketpp::close::status::value>(reason),
            ""
        );
    }

    _socketConnection->eof();
    _outputNotifier.notify_all();
    if (_outputThread.joinable()) {
        _outputThread.join();
    }

#ifdef WIN32
    shutdown(_socket, SD_BOTH);
    closesocket(_socket);
#else
    shutdown(_socket, SHUT_RDWR);
    close(_socket);
#endif
    _socket = INVALID_SOCKET;

    _shouldDisconnect = false;
    _isConnected = false;
    _isConnecting = false;
    _inputNotifier.notify_all();
    if (_inputThread.joinable()) {
        _inputThread.join();
    }

    LDEBUG(fmt::format("Disconnected client {}:{}.", _address, _port));
}

bool WebSocket::socketHasConnection() {
    return _socketConnection->get_state() == websocketpp::session::state::open;
}

bool WebSocket::getMessage(std::string& message) {
    waitForInput();
    std::lock_guard<std::mutex> guard(_inputQueueMutex);

    if (_inputQueue.empty()) {
        return false;
    }

    message = _inputQueue.front();
    _inputQueue.pop_front();
    return true;
}

bool WebSocket::putMessage(const std::string &message) {
    if (!_isConnected) {
        LERROR("Cannot send message when not connected.");
        return false;
    }
    _socketConnection->send(message);
    _outputNotifier.notify_one();
    return true;
}

void WebSocket::streamInput() {
    int nReadBytes = 0;

    while (_isConnected && !_shouldDisconnect) {
        std::lock_guard<std::mutex> inputBufferGuard(_inputBufferMutex);

        nReadBytes = recv(
            _socket,
            _inputBuffer.data(),
            static_cast<int>(_inputBuffer.size()),
            0
        );

        if (nReadBytes <= 0) {
            _error = true;
            LDEBUG("Received graceful close request.");
            _shouldDisconnect = true;
            _inputNotifier.notify_one();
            return;
        }

        _socketConnection->read_some(_inputBuffer.data(), nReadBytes);

        // Poke output notifier, as the received message
        // might trigger something like a handshake or similar.
        _outputNotifier.notify_one();
    }
}

void WebSocket::streamOutput() {
    while (_isConnected && !_shouldDisconnect) {
        waitForOutput(1);

        int bytesToSend = 0;
        std::lock_guard<std::mutex> queueGuard(_outputQueueMutex);
        while ((bytesToSend = outputStreamSize()) > 0) {
            std::lock_guard<std::mutex> streamGuard(_outputStreamMutex);
            std::string output = _outputStream.str();
            int sentBytes = send(_socket, output.c_str(), bytesToSend, 0);

            if (sentBytes <= 0) {
                LERROR(
                    fmt::format("Bad send return code: {}. Disconnecting!", errorCode())
                );
                _error = true;
                _shouldDisconnect = true;
                _inputNotifier.notify_one();
                return;
            }

            _outputStream.clear();
            _outputStream.str(std::string());
        }
    }
}

int WebSocket::errorCode() {
#if defined(WIN32)
    return WSAGetLastError();
#else
    return 0;
#endif
}

int WebSocket::outputStreamSize() {
    if (!_outputStream) {
        return 0;
    }

    std::lock_guard<std::mutex> guard(_outputStreamMutex);
    _outputStream.seekg(0, std::ios::end);
    std::streampos size = _outputStream.tellg();
    _outputStream.seekg(0, std::ios::beg);
    return static_cast<int>(size);
}

/**
 * Callback for incoming messages
 * \param hdl A handle to uniquely identify a connection.
 * \param msg The message
 */
void WebSocket::onMessage(websocketpp::connection_hdl, WsServer::message_ptr msg) {
    std::string msgContent = msg->get_payload();
    std::lock_guard<std::mutex> guard(_inputQueueMutex);
    _inputQueue.push_back(msgContent);
    _inputNotifier.notify_one();
}

void WebSocket::onOpen(websocketpp::connection_hdl hdl) {
    LDEBUG(fmt::format("onOpen: WebSocket opened. Client: {}:{}.", _address, _port));
    std::lock_guard<std::mutex> guard(_connectionHandlesMutex);
    _connectionHandles.insert(hdl);
}

void WebSocket::onClose(websocketpp::connection_hdl hdl) {
    LDEBUG(fmt::format("onClose: WebSocket closing. Client: {}:{}.", _address, _port));
    _shouldDisconnect = true;

    std::lock_guard<std::mutex> guard(_connectionHandlesMutex);
    _connectionHandles.erase(hdl);
}

bool WebSocket::waitForOutput(size_t nBytes) {
    if (nBytes == 0) {
        return false;
    }

    auto receivedRequestedOutputOrDisconnected = [this, nBytes]() {
        if (_shouldDisconnect || (!_isConnected && !_isConnecting)) {
            return true;
        }
        return static_cast<size_t>(outputStreamSize()) >= nBytes;
    };

    // Block execution until enough data has come into the output queue.
    if (!receivedRequestedOutputOrDisconnected()) {
        std::unique_lock<std::mutex> lock(_outputBufferMutex);
        _outputNotifier.wait(lock, receivedRequestedOutputOrDisconnected);
    }

    return !_error;
}

bool WebSocket::waitForInput(size_t) {
    auto hasInputOrDisconnected = [this]() {
        if (_shouldDisconnect || (!_isConnected && !_isConnecting)) {
            return true;
        }
        return _inputQueue.size() > 0;
    };

    if (!hasInputOrDisconnected()) {
        std::unique_lock<std::mutex> lock(_inputQueueMutex);
        _inputNotifier.wait(lock, hasInputOrDisconnected);
    }

    return !_error;
}

}  // namespace ghoul::io
