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

#include <ghoul/io/socket/websocketserver.h>
#include <cstring>

namespace {
    std::string _loggerCat = "WebSocketServer";
}

namespace ghoul {
namespace io {

WebSocketServer::WebSocketServer() {}

WebSocketServer::~WebSocketServer() {
    if (_listening) {
        close();
    }
}

std::string WebSocketServer::address() const {
    return _address;
}

int WebSocketServer::port() const {
    return _port;
}

void WebSocketServer::close() {
    std::lock_guard<std::mutex> guard(_settingsMutex);
    _listening = false;

    // Flush out pending connections
    _clientConnectionNotifier.notify_all();
    closeSocket(_serverSocket);

    // Wait for server thread to exit
    _serverThread->join();
}

void WebSocketServer::listen(std::string address, int port) {
    if (_listening) {
        throw WebSocket::WebSocketError("WebSocket is already listening");
    }
    if (!TcpSocket::initializedNetworkApi()) {
        TcpSocket::initializeNetworkApi();
    }

    std::lock_guard<std::mutex> settingsLock(_settingsMutex);
    _address = address;
    _port = port;

    struct addrinfo* result = nullptr;
    struct addrinfo hints;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM; // TODO(klas): verify correct socktype
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Find the local address and port to be used by the server
    int iResult;
    iResult = getaddrinfo(nullptr, std::to_string(_port).c_str(), &hints, &result);
    if (iResult != 0) {
#if defined(WIN32)
        WSACleanup();
#endif
        throw WebSocket::WebSocketError("Failed to parse hints for web socket connection");
    }

    // Create a socket for the server to listen for client connections
    _serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (_serverSocket == INVALID_SOCKET) {
        freeaddrinfo(result);
#if defined(WIN32)
        WSACleanup();
#endif
        throw WebSocket::WebSocketError("Failed to initiate server web socket");
    }

    setOptions(_serverSocket);

    // Setup listening on the TCP socket
    iResult = bind(_serverSocket, result->ai_addr, static_cast<int>(result->ai_addrlen));
    if (iResult == SOCKET_ERROR) {
        freeaddrinfo(result);
        closeSocket(_serverSocket);
#if defined(WIN32)
        WSACleanup();
#endif
        throw WebSocket::WebSocketError("Listen failed with error: " + std::to_string(_ERRNO));
    }

    // Cleanup
    freeaddrinfo(result);

    if (::listen(_serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        closeSocket(_serverSocket);
#if defined(WIN32)
        WSACleanup();
#endif
        WebSocket::WebSocketError("Listen failed with error: " + std::to_string(_ERRNO));
    }

    // Mark as active and start separate thread for incoming connections
    _listening = true;
    _serverThread = std::make_unique<std::thread>(
        [this]() { waitForConnections(); }
    );
}

bool WebSocketServer::isListening() const {
    return _listening;
}

bool WebSocketServer::hasPendingSockets() const {
    if (!_listening) {
        return false;
    }

    std::lock_guard<std::mutex> guard(_connectionMutex);
    return !_pendingClientConnections.empty();
}

std::unique_ptr<WebSocket> WebSocketServer::nextPendingWebSocket() {
    if (!isListening()) {
        return nullptr;
    }

    std::lock_guard<std::mutex> guard(_connectionMutex);
    if (!_pendingClientConnections.empty()) {
        std::unique_ptr<WebSocket> connection = std::move(_pendingClientConnections.front());
        _pendingClientConnections.pop_front();
        return connection;
    }

    return nullptr;
}

std::unique_ptr<Socket> WebSocketServer::nextPendingSocket() {
    return static_cast<std::unique_ptr<Socket>>(nextPendingWebSocket());
}

std::unique_ptr<WebSocket> WebSocketServer::awaitPendingWebSocket() {
    if (!_listening) {
        return nullptr;
    }

    // Block execution until we have a pending connection
    std::unique_lock<std::mutex> lock(_connectionNotificationMutex);
    _clientConnectionNotifier.wait(lock, [this]() {
        return hasPendingSockets() || !_listening;
    });

    // No longer blocked: There might be a waiting connection for us
    return nextPendingWebSocket();
}

std::unique_ptr<Socket> WebSocketServer::awaitPendingSocket() {
    return std::unique_ptr<Socket>(awaitPendingWebSocket());
}

void WebSocketServer::waitForConnections() {
    while (_listening) {
        sockaddr_in clientInfo = { 0 };
        _SOCKLEN clientInfoSize = sizeof(clientInfo);

        _SOCKET socketHandle = accept((int)_serverSocket, (sockaddr*)&clientInfo, &clientInfoSize);
        if (socketHandle == INVALID_SOCKET) {
            // no client wanted this socket -- continue loop
#if defined(WIN32) 
            LERROR(fmt::format("Could not start socket: ERROR {}", WSAGetLastError()));
            char val;
            socklen_t len = sizeof(val);
            if (getsockopt(_serverSocket, SOL_SOCKET, SO_ACCEPTCONN, &val, &len) == -1)
                LERROR(fmt::format("_serverSocket {} is not a socket", _serverSocket));
            else if (val)
                LERROR(fmt::format("_serverSocket {} is a listening socket", _serverSocket));
            else
                LERROR(fmt::format("_serverSocket {} is a non-listening socket", _serverSocket));
#endif
            continue;
        }

        // get client address and port
        char addressBuffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientInfo.sin_addr), addressBuffer, INET_ADDRSTRLEN);
        std::string address = addressBuffer;
        auto port = static_cast<int>(clientInfo.sin_port);

        // create client socket
        std::unique_ptr<WebSocket> socket = std::make_unique<WebSocket>(address, port, socketHandle);

        // store client
        std::lock_guard<std::mutex> guard(_connectionMutex);
        _pendingClientConnections.push_back(std::move(socket));

        // Notify `awaitPendingConnection` to return the acquired connection.
        _clientConnectionNotifier.notify_one();
    }
}

void WebSocketServer::setOptions(_SOCKET socket) {
    char trueFlag = 1;
    int iResult;

    // Set no delay
    iResult = setsockopt(socket,
                         IPPROTO_TCP,
                         TCP_NODELAY,
                         &trueFlag,
                         sizeof(trueFlag));

    // Set send timeout
    char timeout = 0; // infinite
    iResult = setsockopt(
            socket,
            SOL_SOCKET,
            SO_SNDTIMEO,
            &timeout,
            sizeof(timeout));

    // Set receive timeout
    iResult = setsockopt(
            socket,
            SOL_SOCKET,
            SO_RCVTIMEO,
            &timeout,
            sizeof(timeout));

    iResult = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(int));
    if (iResult == SOCKET_ERROR) {
        LERROR("Failed to set reuse address with error: " << _ERRNO);
    }

    iResult = setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &trueFlag, sizeof(int));
    if (iResult == SOCKET_ERROR) {
        LERROR("Failed to set keep alive with error: " << _ERRNO);
    }
}

void WebSocketServer::closeSocket(_SOCKET socket) {
    if (socket != INVALID_SOCKET) {
#ifdef WIN32
        shutdown(socket, SD_BOTH);
        closesocket(socket);
#else
        shutdown(socket, SHUT_RDWR);
        ::close(socket);
#endif
    }
}

}
}
