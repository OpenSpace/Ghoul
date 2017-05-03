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

TcpSocket::TcpSocket()
    : _connected(false)
    , _error(false)
    , _socket(INVALID_SOCKET)
    , _inputThread(nullptr)
    , _outputThread(nullptr)
    , _socketId(++_nextSocketId)
{
    if (!_initializedNetworkApi) {
        initializeNetworkApi();
    }
}

TcpSocket::TcpSocket( _SOCKET socket, std::string address, int port)
    : _connected(true)
    , _error(false)
    , _socket(socket) 
    , _socketId(++_nextSocketId)
{
    if (!_initializedNetworkApi) {
        initializeNetworkApi();
    }
    _inputThread = std::make_unique<std::thread>(
        [this]() { streamInput(); }
    );
    _outputThread = std::make_unique<std::thread>(
        [this]() { streamOutput(); }
    );
}

TcpSocket::~TcpSocket() {
    if (_connected) {
        disconnect();
    }
}

void TcpSocket::connect(std::string address, int port) {
    if (_connected) {
        throw TcpSocket::TcpSocketError("Socket is already connected.");
    }

    _error = false;
    // todo: connect.
}

void TcpSocket::disconnect() {

#ifdef WIN32
    shutdown(_socket, SD_BOTH);
    closesocket(_socket);
#else
    shutdown(_socket, SHUT_RDWR);
    close(_socket);
#endif
    _socket = INVALID_SOCKET;

    _connected = false;
    _inputNotifier.notify_all();
    _outputNotifier.notify_all();

    if (_inputThread) {
        _inputThread->join();
        _inputThread = nullptr;
    }
    if (_outputThread) {
        _outputThread->join();
        _outputThread = nullptr;
    }
}

bool TcpSocket::connected() const {
    return _connected;
}

int TcpSocket::socketId() const {
    return _socketId;
}

void TcpSocket::streamInput() {
    int nReadBytes = 0;
    while (_connected) {
        nReadBytes = recv(
            _socket,
            _inputBuffer.data(),
            static_cast<int>(_inputBuffer.size()),
            0);

        if (nReadBytes <= 0) {
            _connected = false;
            _error = true;
            return;
        }

        std::lock_guard<std::mutex> _inputGuard(_inputQueueMutex);
        _inputQueue.insert(
            _inputQueue.end(),
            _inputBuffer.begin(),
            _inputBuffer.begin() + nReadBytes
        );
        _inputNotifier.notify_one();
    }
}

void TcpSocket::streamOutput() {
    while (_connected) {
        waitForOutput(1);

        int nBytesToSend = 0;
        while (nBytesToSend =
            std::min(_outputQueue.size(), _outputBuffer.size()) > 0)
        {
            std::copy_n(
                _outputQueue.begin(),
                nBytesToSend,
                _outputBuffer.begin()
            );
            int nSentBytes = send(_socket, _outputBuffer.data(), nBytesToSend, 0);
            if (nSentBytes <= 0) {
                _connected = false;
                _error = true;
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
        if (!_connected) {
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
        if (!_connected) {
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
