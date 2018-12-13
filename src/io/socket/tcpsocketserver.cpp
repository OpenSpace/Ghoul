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

#include <ghoul/io/socket/tcpsocketserver.h>

#include <ghoul/io/socket/tcpsocket.h>
#include <cstring>

#ifdef WIN32
#define NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#ifndef _ERRNO
#define _ERRNO WSAGetLastError()
#endif

#pragma warning(push)
#pragma warning (disable : 4996)
#else //Use BSD sockets
#ifdef _XCODE
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (_SOCKET)(~0)
#endif

// #ifndef NO_ERROR
// #define NO_ERROR 0L
// #endif

#ifndef _ERRNO
#define _ERRNO errno
#endif
#endif // !WIN32

namespace {
    void closeSocket(_SOCKET socket) {
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

    void setOptions(_SOCKET socket) {
        char trueFlag = 1;

        //Set no delay
        setsockopt(socket,IPPROTO_TCP, TCP_NODELAY, &trueFlag, sizeof(trueFlag));

        // Set send timeout
        char timeout = 0; //infinite
        setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

        // Set receive timeout
        setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(int));
        setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &trueFlag, sizeof(int));
    }
} // namespace

namespace ghoul::io {

TcpSocketServer::TcpSocketServer() {}

TcpSocketServer::~TcpSocketServer() {
    if (_listening) {
        close();
    }
}

int TcpSocketServer::port() const {
    std::lock_guard settingsLock(_settingsMutex);
    return _port;
}

void TcpSocketServer::close() {
    std::lock_guard settingsLock(_settingsMutex);
    _listening = false;

    // Notify all threads waiting for connections.
    _connectionNotifier.notify_all();
    const _SOCKET serverSocket = _serverSocket;
    _serverSocket = INVALID_SOCKET;
    closeSocket(serverSocket);
    _serverThread->join();
}

void TcpSocketServer::listen(int port) {
    if (_listening) {
        throw TcpSocket::TcpSocketError("Socket is already listening.");
    }
    if (!TcpSocket::initializedNetworkApi()) {
        TcpSocket::initializeNetworkApi();
    }

    std::lock_guard settingsLock(_settingsMutex);
    _port = port;

    struct addrinfo* result = nullptr;
    struct addrinfo hints {};

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
        throw TcpSocket::TcpSocketError(
            "Bind failed with error: " + std::to_string(_ERRNO)
        );
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
    _serverThread = std::make_unique<std::thread>([this]() { waitForConnections(); });
}

bool TcpSocketServer::isListening() const {
    std::lock_guard settingsLock(_settingsMutex);
    return _listening;
}

bool TcpSocketServer::hasPendingSockets() const {
    if (!_listening) {
        return false;
    }
    std::lock_guard<std::mutex> connectionLock(_connectionMutex);
    return !_pendingConnections.empty();
}

std::unique_ptr<TcpSocket> TcpSocketServer::nextPendingTcpSocket() {
    if (!_listening) {
        return nullptr;
    }
    std::lock_guard connectionLock(_connectionMutex);
    if (!_pendingConnections.empty()) {
        std::unique_ptr<TcpSocket> connection = std::move(_pendingConnections.front());
        _pendingConnections.pop_front();
        return connection;
    }
    return nullptr;
}

std::unique_ptr<Socket> TcpSocketServer::nextPendingSocket() {
    return nextPendingTcpSocket();
}

std::unique_ptr<TcpSocket> TcpSocketServer::awaitPendingTcpSocket() {
    if (!_listening) {
        return nullptr;
    }
    std::unique_lock lock(_connectionNotificationMutex);

    // Block execution until there is a pending connection or until the server stops
    // listening.
    _connectionNotifier.wait(lock, [this]() {
        return hasPendingSockets() || !_listening;
    });

    return nextPendingTcpSocket();
}

std::unique_ptr<Socket> TcpSocketServer::awaitPendingSocket() {
    return awaitPendingTcpSocket();
}

void TcpSocketServer::waitForConnections() {
    while (_listening) {
        sockaddr_in clientInfo {};
        std::memset(&clientInfo, 0, sizeof(clientInfo));
        _SOCKLEN clientInfoSize = sizeof(clientInfo);

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif // __GNUC__
        _SOCKET socketHandle = accept(
            static_cast<int>(_serverSocket),
            reinterpret_cast<sockaddr*>(&clientInfo),
            &clientInfoSize
        );
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif // __GNUC__

        if (socketHandle == INVALID_SOCKET) {
            continue;
        }

        char addressBuffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientInfo.sin_addr), addressBuffer, INET_ADDRSTRLEN);
        std::string address = addressBuffer;
        int port = static_cast<int>(clientInfo.sin_port);

        // @CLEANUP(abock): Can the _pendingConnections be moved to Socket instead of
        //                  unique_ptr?
        auto socket = std::make_unique<TcpSocket>(address, port, socketHandle);

        std::lock_guard lock(_connectionMutex);
        _pendingConnections.push_back(std::move(socket));

        // Notify `awaitPendingConnection` to return the acquired connection.
        _connectionNotifier.notify_one();
    }
}

} // namespace ghoul::io
