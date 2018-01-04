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

#include <cstring>

namespace {
    const char DefaultDelimiter = '\n';
}

namespace ghoul {
namespace io {

std::atomic<bool> TcpSocket::_initializedNetworkApi{false};

TcpSocket::TcpSocketError::TcpSocketError(std::string message, std::string component)
    : RuntimeError(message, component) {}

TcpSocket::TcpSocket(std::string address, int port)
    : Socket()
    , _address(address)
    , _port(port)
    , _isConnected(false)
    , _isConnecting(false)
    , _shouldDisconnect(false)
    , _error(false)
    , _socket(INVALID_SOCKET)
    , _inputThread(std::thread())
    , _outputThread(std::thread())
    , _delimiter(DefaultDelimiter)
{}

TcpSocket::TcpSocket(std::string address, int port, _SOCKET socket)
    : Socket()
    , _address(address)
    , _port(port)
    , _isConnected(true)
    , _isConnecting(false)
    , _shouldDisconnect(false)
    , _error(false)
    , _socket(socket)
    , _delimiter(DefaultDelimiter) {}

TcpSocket::~TcpSocket() {
    if (_isConnected) {
        disconnect();
    }
}

void TcpSocket::startStreams() {
    _inputThread = std::thread(
        [this]() { streamInput(); }
    );
    _outputThread = std::thread(
        [this]() { streamOutput(); }
    );
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
    std::memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    int result = getaddrinfo(_address.c_str(), std::to_string(_port).c_str(), &hints, &addresult);
    if (result != 0) {
        return;
    }

    _isConnecting = true;

    _outputThread = std::thread([this, addresult]() {
        establishConnection(addresult);
        _inputThread = std::thread(
            [this]() { streamInput(); }
        );
        streamOutput();
    });

    _error = false;
}

void TcpSocket::disconnect(const int reason) {
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

    if (_inputThread.joinable()) {
        _inputThread.join();
    }
    if (_outputThread.joinable()) {
        _outputThread.join();
    }
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

bool TcpSocket::getMessage(std::string& message) {
    int delimiterIndex = waitForDelimiter();
    if (delimiterIndex == -1) {
        return false;
    }
    std::lock_guard<std::mutex> inputLock(_inputQueueMutex);
    message = std::string(_inputQueue.begin(), _inputQueue.begin() + delimiterIndex);
    _inputQueue.erase(_inputQueue.begin(), _inputQueue.begin() + delimiterIndex + 1);
    return true;
}

bool TcpSocket::putMessage(const std::string& message) {
    if (!putBytes(message.data(), message.size())) {
        return false;
    }
    char delimiter = _delimiter;
    return putBytes(&delimiter);   
}

void TcpSocket::setDelimiter(char delimiter) {
    _delimiter = delimiter;
}

void TcpSocket::establishConnection(addrinfo *info) {
    _socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

    if (_socket == INVALID_SOCKET) {
        freeaddrinfo(info);
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
        return;
    }
    // Keep alive
    result = setsockopt(_socket, SOL_SOCKET, SO_KEEPALIVE, &trueFlag, sizeof(trueFlag));
    if (result == SOCKET_ERROR) {
        return;
    }

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
            _inputNotifier.notify_one();
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
    while (_isConnected && !_shouldDisconnect) {
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
                _inputNotifier.notify_one();
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

int TcpSocket::waitForDelimiter() {
    char delimiter = _delimiter;
    int currentIndex = 0;
    auto receivedRequestedInputOrDisconnected = [this, &currentIndex, delimiter]() {
        if (_shouldDisconnect || (!_isConnected && !_isConnecting)) {
            return true;
        }
        std::lock_guard<std::mutex> queueMutex(_inputQueueMutex);
        auto it = std::find(_inputQueue.begin() + currentIndex, _inputQueue.end(), delimiter);
        currentIndex = it - _inputQueue.begin();
        return it != _inputQueue.end();
    };

    // Block execution until the delimiter character was found in the input queue.
    if (!receivedRequestedInputOrDisconnected()) {
        std::unique_lock<std::mutex> lock(_inputBufferMutex);
        _inputNotifier.wait(lock, receivedRequestedInputOrDisconnected);
    }
    if (_error) {
        return -1;
    } else {
        return currentIndex;
    }
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


bool TcpSocket::getBytes(char* getBuffer, size_t size) {
    waitForInput(size);
    if (_shouldDisconnect) {
        disconnect();
    }
    if (!_isConnected && !_isConnecting) {
        return false;
    }
    std::lock_guard<std::mutex> inputLock(_inputQueueMutex);
    std::copy_n(_inputQueue.begin(), size, getBuffer);
    _inputQueue.erase(_inputQueue.begin(), _inputQueue.begin() + size);
    return true;
}

bool TcpSocket::peekBytes(char* peekBuffer, size_t size) {
    waitForInput(size);
    if (_shouldDisconnect) {
        disconnect();
    }
    if (!_isConnected && !_isConnecting) {
        return false;
    }
    std::lock_guard<std::mutex> inputLock(_inputQueueMutex);
    std::copy_n(_inputQueue.begin(), size, peekBuffer);
    return true;
}

bool TcpSocket::skipBytes(size_t size) {
    waitForInput(size);
    if (_shouldDisconnect) {
        disconnect();
    }
    if (!_isConnected && !_isConnecting) {
        return false;
    }
    std::lock_guard<std::mutex> inputLock(_inputQueueMutex);
    _inputQueue.erase(_inputQueue.begin(), _inputQueue.begin() + size);
    return true;
}

bool TcpSocket::putBytes(const char* buffer, size_t size) {
    if (_shouldDisconnect) {
        disconnect();
    }
    std::lock_guard<std::mutex> outputLock(_outputQueueMutex);
    _outputQueue.insert(
        _outputQueue.end(),
        buffer,
        buffer + size
    );
    _outputNotifier.notify_one();
    return _isConnected || _isConnecting;
}


}
}
