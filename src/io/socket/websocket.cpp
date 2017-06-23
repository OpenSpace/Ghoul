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
    server.set_message_handler(bind(&WebSocket::onMessage, this, ::_1, ::_2));
    server.set_open_handler(bind(&WebSocket::onOpen,this,::_1));
    server.set_open_handler(bind(&WebSocket::onClose,this,::_1));

    server.register_ostream(&_outputStream);
    socketConnection = server.get_connection();
    socketConnection->start();

    startStreams();
    LDEBUG(fmt::format("WebSocket started. Client: {}:{}.", address, port));
}

WebSocket::~WebSocket() {
    LDEBUG("Destroying socket connection");
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
    std::lock_guard<std::mutex> guard(_outputStreamMutex);
    _outputStream << message;
    _outputNotifier.notify_one();
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
			LDEBUG("Received graceful close request.");
            _shouldDisconnect = true;
            _inputNotifier.notify_one();
            return;
        }

        socketConnection->read_some(_inputBuffer.data(), nReadBytes);

        // Also poke output notifier, as the received message
        // might trigger something like a handshake or similar.
        _outputNotifier.notify_one();
    }
}

void WebSocket::streamOutput() {
    while (_isConnected && !_shouldDisconnect) {
        waitForOutput(1);

        int bytesToSend = 0;
        std::lock_guard<std::mutex> streamGuard(_outputStreamMutex);
        std::lock_guard<std::mutex> queueGuard(_outputQueueMutex);
        while ((bytesToSend = outputStreamSize()) > 0) {
            // copy from outputStream to outputBuffer

//            int sentBytes = send(_socket, _outputBuffer.data(), bytesToSend, 0);
            std::string output = _outputStream.str();
            LDEBUG(fmt::format("Sending \"{}\" to client {}:{}.", output, _address, _port));
            int sentBytes = send(_socket, output.c_str(), bytesToSend, 0);

            if (sentBytes <= 0) {
                LDEBUG("Sent 0 bytes, disconnecting!");
                _error = true;
                _shouldDisconnect = true;
                _inputNotifier.notify_one(); // ????
                return;
            }

            // empty outputqueue?

            // empty outputStream
            _outputStream.clear();
            _outputStream.str(std::string());
        }
    }
}

int WebSocket::outputStreamSize() {
	if (!_outputStream) {
		return 0;
	}

//    std::lock_guard<std::mutex> guard(_outputStreamMutex); // causes crash??
    _outputStream.seekg(0, std::ios::end);
    int size = _outputStream.tellg();
    _outputStream.seekg(0, std::ios::beg);
    return size;
}

/**
 * Callback for incoming messages
 * @param hdl - A handle to uniquely identify a connection.
 * @param msg - the message
 */
void WebSocket::onMessage(websocketpp::connection_hdl hdl, WsServer::message_ptr msg) {
    std::string msgContent = msg->get_payload();
    LDEBUG(fmt::format("Message received: {}", msgContent));
    std::lock_guard<std::mutex> guard(_inputQueueMutex);
    // store message
    _inputNotifier.notify_one();
}

void WebSocket::onOpen(websocketpp::connection_hdl hdl) {
    LDEBUG(fmt::format("WebSocket opened. Client: {}:{}.", _address, _port));
}

void WebSocket::onClose(websocketpp::connection_hdl hdl) {
    LDEBUG(fmt::format("WebSocket closing. Client: {}:{}.", _address, _port));
    _shouldDisconnect = true;
}

bool WebSocket::waitForOutput(size_t nBytes) {
    if (nBytes == 0) {
        return false;
    }

    auto receivedRequestedOutputOrDisconnected = [this, nBytes]() {
        if (_shouldDisconnect || (!_isConnected && !_isConnecting)) {
            return true;
        }
        std::lock_guard<std::mutex> streamGuard(_outputStreamMutex);
        std::lock_guard<std::mutex> queueGuard(_outputQueueMutex);
        return _outputQueue.size() >= nBytes || outputStreamSize() >= nBytes;
    };

    // Block execution until enough data has come into the output queue.
    if (!receivedRequestedOutputOrDisconnected()) {
        std::unique_lock<std::mutex> lock(_outputBufferMutex);
        _outputNotifier.wait(lock, receivedRequestedOutputOrDisconnected);
    }

    return !_error;
}

}
}
