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

#include <ghoul/io/socket/tcpsocket.h>


namespace ghoul {
namespace io {

std::atomic<bool> TcpSocket::_initializedNetworkApi = false;
std::atomic<int> TcpSocket::_nextSocketId = 0;

TcpSocket::TcpSocketError::TcpSocketError(std::string message, std::string component)
    : RuntimeError(message, component) {}

TcpSocket::TcpSocket(std::string address, int port)
    : _address(address)
    , _port(port)
    , _isConnected(false)
    , _isConnecting(false)
    , _shouldDisconnect(false)
    , _error(false)
    , _socket(INVALID_SOCKET)
    , _inputThread(nullptr)
    , _outputThread(nullptr)
    , _socketId(++_nextSocketId)
{
}

TcpSocket::TcpSocket(std::string address, int port, _SOCKET socket)
    : _address(address)
    , _port(port)
    , _isConnected(true)
    , _isConnecting(false)
    , _error(false)
    , _socket(socket) 
    , _socketId(++_nextSocketId)
{
    _inputThread = std::make_unique<std::thread>(
        [this]() { streamInput(); }
    );
    _outputThread = std::make_unique<std::thread>(
        [this]() { streamOutput(); }
    );
}

TcpSocket::~TcpSocket() {
    if (_isConnected) {
        disconnect();
    }
}

void TcpSocket::connect() {
    if (_isConnected) {
        throw TcpSocket::TcpSocketError("Socket is already connected.");
    }
    if (_isConnecting) {
        throw TcpSocket::TcpSocketError("Socket is already trying to connect.");
    }
    if (!_initializedNetworkApi) {
        initializeNetworkApi();
    }

    struct addrinfo *addresult = NULL, hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int result = getaddrinfo(_address.c_str(), std::to_string(_port).c_str(), &hints, &addresult);
    if (result != 0)
    {
        //LERROR("Failed to parse hints for Parallel Connection");
        return;
    }

    _isConnecting = true;

    //start output thread. First 
    _outputThread = std::make_unique<std::thread>([this, addresult]() {
        establishConnection(addresult);
        _inputThread = std::make_unique<std::thread>(
            [this]() { streamInput(); }
        );
        streamOutput();
    });

    _error = false;
    // todo: connect.
}

void TcpSocket::disconnect() {
    if (!_isConnected) return;

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
    _outputNotifier.notify_all();

    if (_inputThread && _inputThread->joinable()) {
        _inputThread->join();
    }
    _inputThread = nullptr;

    if (_outputThread && _outputThread->joinable()) {
        _outputThread->join();
    }
    _outputThread = nullptr;
}

bool TcpSocket::isConnected() {
    if (_shouldDisconnect) {
        disconnect();
    }
    return _isConnected;
}

bool TcpSocket::isConnecting() {
    if (_shouldDisconnect) {
        disconnect();
    }
    return _isConnecting;
}

int TcpSocket::socketId() const {
    return _socketId;
}

void TcpSocket::establishConnection(addrinfo *info) {
    _socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

    if (_socket == INVALID_SOCKET) {
        freeaddrinfo(info);
        //LERROR("Failed to create client socket, disconnecting.");
        _error = true;
        _shouldDisconnect = true;
        return;
    }

    char trueFlag = 1;
    char falseFlag = 0;
    int result;

    // Disable Nagle's algorithm.
    result = setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, &trueFlag, sizeof(trueFlag));

    // Set send timeout
    char timeout = 0;
    result = setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    // Set receive timeout
    result = setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    // Disable address reuse
    result = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &falseFlag, sizeof(falseFlag));
    if (result == SOCKET_ERROR) {
        //LERROR("Failed to set socket option 'reuse address'. Error code: " << _ERRNO);
    }

    result = setsockopt(_socket, SOL_SOCKET, SO_KEEPALIVE, &trueFlag, sizeof(trueFlag));
    if (result == SOCKET_ERROR) {
        //LERROR("Failed to set socket option 'keep alive'. Error code: " << _ERRNO);
    }
    //LINFO("Attempting to connect to server " << _address << " on port " << _port);

    // Try to connect
    result = ::connect(_socket, info->ai_addr, static_cast<int>(info->ai_addrlen));
    _isConnected = true;
    _isConnecting = false;
}

void TcpSocket::streamInput() {
    int nReadBytes = 0;

    while (_isConnected && !_shouldDisconnect) {
        nReadBytes = recv(
            _socket,
            _inputBuffer.data(),
            static_cast<int>(_inputBuffer.size()),
            0);

        if (nReadBytes <= 0) {
            _error = true;
            _shouldDisconnect = true;
            return;
        }

        std::lock_guard<std::mutex> inputGuard(_inputQueueMutex);
        _inputQueue.insert(
            _inputQueue.end(),
            _inputBuffer.begin(),
            _inputBuffer.begin() + nReadBytes
        );
        _inputNotifier.notify_one();
    }
}

void TcpSocket::streamOutput() {
    while (_isConnected  && !_shouldDisconnect) {
        waitForOutput(1);

        int nBytesToSend = 0;
        std::lock_guard<std::mutex> outputGuard(_outputQueueMutex);
        while ((nBytesToSend = std::min(_outputQueue.size(), _outputBuffer.size())) > 0)
        {
            std::copy_n(
                _outputQueue.begin(),
                nBytesToSend,
                _outputBuffer.begin()
            );
            int nSentBytes = send(_socket, _outputBuffer.data(), nBytesToSend, 0);
            if (nSentBytes <= 0) {
                _error = true;
                _shouldDisconnect = true;
                return;
            }
            _outputQueue.erase(
                _outputQueue.begin(),
                _outputQueue.begin() + nBytesToSend
            );
        }
    }
}

bool TcpSocket::waitForInput(size_t nBytes) {
    if (nBytes == 0) {
        return true;
    }

    auto receivedRequestedInputOrDisconnected = [this, nBytes]() {
        if (_shouldDisconnect || (!_isConnected && !_isConnecting)) {
            return true;
        }
        std::lock_guard<std::mutex> queueMutex(_inputQueueMutex);
        return _inputQueue.size() >= nBytes;
    };

    // Block execution until enough data has come into the input queue.
    if (!receivedRequestedInputOrDisconnected()) {
        std::unique_lock<std::mutex> lock(_inputBufferMutex);
        _inputNotifier.wait(lock, receivedRequestedInputOrDisconnected);
    }

    return !_error;
}

bool TcpSocket::waitForOutput(size_t nBytes) {
    if (nBytes == 0) {
        return true;
    }

    auto receivedRequestedOutputOrDisconnected = [this, nBytes]() {
        if (_shouldDisconnect || (!_isConnected && !_isConnecting)) {
            return true;
        }
        std::lock_guard<std::mutex> queueMutex(_outputQueueMutex);
        return _outputQueue.size() >= nBytes;
    };

    // Block execution until enough data has come into the output queue.
    if (!receivedRequestedOutputOrDisconnected()) {
        std::unique_lock<std::mutex> lock(_outputBufferMutex);
        _outputNotifier.wait(lock, receivedRequestedOutputOrDisconnected);
    }

    return !_error;
}

void TcpSocket::initializeNetworkApi() {
#ifdef WIN32
    WSADATA wsaData;
    WORD version;
    int error;

    version = MAKEWORD(2, 2);

    error = WSAStartup(version, &wsaData);

    if (error != 0 ||
        LOBYTE(wsaData.wVersion) != 2 ||
        HIBYTE(wsaData.wVersion) != 2)
    {
        /* incorrect WinSock version */
        WSACleanup();

        throw std::runtime_error("Failed to init WinSock API");
    }
#else
    //No init needed on unix
#endif
    _initializedNetworkApi = true;
}

bool TcpSocket::initializedNetworkApi() {
    return _initializedNetworkApi;
}

}
}
