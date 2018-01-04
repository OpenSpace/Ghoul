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

#include <ghoul/io/socket/tcpsocketserver.h>

#include <cstring>

namespace ghoul {
namespace io {

TcpSocketServer::TcpSocketServer()
    : _address("localhost")
    , _port(0)
    , _listening(false)
{
}

TcpSocketServer::~TcpSocketServer() {
    if (_listening) {
        close();
    }
}

std::string TcpSocketServer::address() const {
    std::lock_guard<std::mutex> settingsLock(_settingsMutex);
    return _address;
}

int TcpSocketServer::port() const {
    std::lock_guard<std::mutex> settingsLock(_settingsMutex);
    return _port;
}

void TcpSocketServer::closeSocket(_SOCKET socket) {
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

void TcpSocketServer::close() {
    std::lock_guard<std::mutex> settingsLock(_settingsMutex);
    _listening = false;
    
    // Notify all threads waiting for connections.
    _connectionNotifier.notify_all();
    closeSocket(_serverSocket);
    _serverThread->join();
}

void TcpSocketServer::listen(std::string address, int port) {
    if (_listening) {
        throw TcpSocket::TcpSocketError("Socket is already listening.");
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
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int iResult;

    // Resolve the local address and port to be used by the server
    iResult = getaddrinfo(nullptr, std::to_string(_port).c_str(), &hints, &result);
    if (iResult != 0) {
#if defined(WIN32)
        WSACleanup();
#endif
        throw TcpSocket::TcpSocketError("Failed to parse hints for connection!");
    }

    // Create a socket for the server to listen for client connections
    _serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (_serverSocket == INVALID_SOCKET) {
        freeaddrinfo(result);
#if defined(WIN32)
        WSACleanup();
#endif
        throw TcpSocket::TcpSocketError("Failed to init server socket!");
    }

    setOptions(_serverSocket);

    // Setup the TCP listening socket
    iResult = bind(_serverSocket, result->ai_addr, static_cast<int>(result->ai_addrlen));
    if (iResult == SOCKET_ERROR) {
        freeaddrinfo(result);
        closeSocket(_serverSocket);
#if defined(WIN32)
        WSACleanup();
#endif
        throw TcpSocket::TcpSocketError("Bind failed with error: " + std::to_string(_ERRNO));
    }

    // Clean up
    freeaddrinfo(result);

    if (::listen(_serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        closeSocket(_serverSocket);
#if defined(WIN32)
        WSACleanup();
#endif
        TcpSocket::TcpSocketError("Listen failed with error: " + std::to_string(_ERRNO));
    }

    _listening = true;
    _serverThread = std::make_unique<std::thread>(
        [this]() { waitForConnections(); }
    );
}

bool TcpSocketServer::isListening() const {
    std::lock_guard<std::mutex> settingsLock(_settingsMutex);
    return _listening;
}

bool TcpSocketServer::hasPendingSockets() const {
    if (!_listening) {
        return false;
    }
    std::lock_guard<std::mutex> connectionLock(_connectionMutex);
    return _pendingConnections.size() > 0;
}

std::unique_ptr<TcpSocket> TcpSocketServer::nextPendingTcpSocket() {
    if (!_listening) {
        return nullptr;
    }
    std::lock_guard<std::mutex> connectionLock(_connectionMutex);
    if (_pendingConnections.size() > 0) {
        std::unique_ptr<TcpSocket> connection = std::move(_pendingConnections.front());
        _pendingConnections.pop_front();
        return connection;
    }
    return nullptr;
}

std::unique_ptr<Socket> TcpSocketServer::nextPendingSocket() {
    return static_cast<std::unique_ptr<Socket>>(nextPendingTcpSocket());
}

std::unique_ptr<TcpSocket> TcpSocketServer::awaitPendingTcpSocket() {
    if (!_listening) {
        return nullptr;
    }
    std::unique_lock<std::mutex> lock(_connectionNotificationMutex);

    // Block execution until there is a pending connection or until the server stops listening.
    _connectionNotifier.wait(lock, [this]() {
        return hasPendingSockets() || !_listening;
    });

    return nextPendingTcpSocket();
}

std::unique_ptr<Socket> TcpSocketServer::awaitPendingSocket() {
    return static_cast<std::unique_ptr<Socket>>(awaitPendingTcpSocket());
}

void TcpSocketServer::waitForConnections() {
    while (_listening) {
        sockaddr_in clientInfo = { 0 };
        _SOCKLEN clientInfoSize = sizeof(clientInfo);

        _SOCKET socketHandle = accept((int)_serverSocket, (sockaddr*)&clientInfo, &clientInfoSize);
        if (socketHandle == INVALID_SOCKET) {
            continue;
        }

        char addressBuffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientInfo.sin_addr), addressBuffer, INET_ADDRSTRLEN);
        std::string address = addressBuffer;
        int port = static_cast<int>(clientInfo.sin_port);

        std::unique_ptr<TcpSocket> socket = std::make_unique<TcpSocket>(address, port, socketHandle);
        socket->startStreams();
        
        std::lock_guard<std::mutex> lock(_connectionMutex);
        _pendingConnections.push_back(std::move(socket));

        // Notify `awaitPendingConnection` to return the acquired connection.
        _connectionNotifier.notify_one();
    }
}

void TcpSocketServer::setOptions(_SOCKET socket) {
    char trueFlag = 1;
    int iResult;

    //Set no delay
    setsockopt(socket,
        IPPROTO_TCP,
        TCP_NODELAY,
        &trueFlag,
        sizeof(trueFlag));

    // Set send timeout
    char timeout = 0; //infinite
    setsockopt(
        socket,
        SOL_SOCKET,
        SO_SNDTIMEO,
        &timeout,
        sizeof(timeout));

    // Set receive timeout
    setsockopt(
        socket,
        SOL_SOCKET,
        SO_RCVTIMEO,
        &timeout,
        sizeof(timeout));

    setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(int));
    setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &trueFlag, sizeof(int));
}

}
}
