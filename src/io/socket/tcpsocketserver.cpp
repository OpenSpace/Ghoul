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

namespace ghoul {
namespace io {

TcpSocketServer::TcpSocketServer()
    : _address("localhost")
    , _port(0)
    , _listening(false)
{
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
        close(socket);
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

    TcpSocket::initializeNetworkApi();

    struct addrinfo* result = nullptr;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
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

    // Create a SOCKET for the server to listen for client connections
    _serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (_serverSocket == INVALID_SOCKET) {
        freeaddrinfo(result);
#if defined(WIN32)
        WSACleanup();
#endif
        throw TcpSocket::TcpSocketError("Failed to init server socket!");
    }

    //set options
    setOptions(_serverSocket);

    // Setup the TCP listening socket
    iResult = bind(_serverSocket, result->ai_addr, static_cast<int>(result->ai_addrlen));
    if (iResult == SOCKET_ERROR) {
        freeaddrinfo(result);
        closeSocket(_serverSocket);
#if defined(WIN32)
        WSACleanup();
#endif
        TcpSocket::TcpSocketError("Bind failed with error: " + std::to_string(_ERRNO));
    }

    //cleanup
    freeaddrinfo(result);

    if (::listen(_serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        closeSocket(_serverSocket);
#if defined(WIN32)
        WSACleanup();
#endif
        TcpSocket::TcpSocketError("Listen failed with error: " + std::to_string(_ERRNO));
    }

    //LOG("Waiting for connections on port " << _port);
    _listening = true;
    _serverThread = std::make_unique<std::thread>(
        [this]() { waitForConnections(); }
    );
}

bool TcpSocketServer::listening() const {
    std::lock_guard<std::mutex> settingsLock(_settingsMutex);
    return _listening;
}

bool TcpSocketServer::hasPendingConnections() const {
    if (!_listening) {
        return false;
    }
    std::lock_guard<std::mutex> connectionLock(_connectionMutex);
    return _pendingConnections.size() > 0;
}

std::unique_ptr<TcpSocket> TcpSocketServer::nextPendingConnection() {
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

std::unique_ptr<TcpSocket> TcpSocketServer::awaitPendingConnection() {
    if (!_listening) {
        return nullptr;
    }
    std::unique_lock<std::mutex> lock(_connectionNotificationMutex);
    // Block execution until there is a pending connection or until the server stops listening.
    _connectionNotifier.wait(lock, [this]() {
        return hasPendingConnections() || !_listening;
    });

    if (!_listening) {
        return nullptr;
    }

    return nextPendingConnection();
}

void TcpSocketServer::waitForConnections() {
    while (_listening) {
        sockaddr_in clientInfo = { 0 };
        _SOCKLEN clientInfoSize = sizeof(clientInfo);

        _SOCKET socketHandle = accept((int)_serverSocket, (sockaddr*)&clientInfo, &clientInfoSize);
        if (socketHandle == INVALID_SOCKET) {
            // LERROR("Cannot accept socket connection.");
            continue;
        }

        char addressBuffer[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientInfo.sin_addr), addressBuffer, INET_ADDRSTRLEN);
        std::string address = addressBuffer;
        int port = static_cast<int>(clientInfo.sin_port);

        std::unique_ptr<TcpSocket> socket = std::make_unique<TcpSocket>(address, port, socketHandle);
        
        std::lock_guard<std::mutex> lock(_connectionMutex);
        _pendingConnections.push_back(std::move(socket));

        // Notify `awaitPendingConnection` to return the acquired connection.
        _connectionNotifier.notify_one();
    }
}

void TcpSocketServer::setOptions(_SOCKET socket) {
    char trueFlag = 1;
    int iResult;

    //set no delay
    iResult = setsockopt(socket, /* socket affected */
        IPPROTO_TCP,     /* set option at TCP level */
        TCP_NODELAY,     /* name of option */
        &trueFlag,       /* the cast is historical cruft */
        sizeof(int));    /* length of option value */

    //set send timeout
    char timeout = 0; //infinite
    iResult = setsockopt(
        socket,
        SOL_SOCKET,
        SO_SNDTIMEO,
        &timeout,
        sizeof(timeout));

    //set receive timeout
    iResult = setsockopt(
        socket,
        SOL_SOCKET,
        SO_RCVTIMEO,
        &timeout,
        sizeof(timeout));

    iResult = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(int));
    if (iResult == SOCKET_ERROR) {
        //LERROR("Failed to set reuse address with error: " << _ERRNO);
    }

    iResult = setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &trueFlag, sizeof(int));
    if (iResult == SOCKET_ERROR) {
        //LERROR("Failed to set keep alive with error: " << _ERRNO);
    }
}

}
}

