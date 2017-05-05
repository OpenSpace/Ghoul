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

namespace ghoul {
namespace io {

template <typename T>
bool TcpSocket::get(T* getBuffer, size_t nItems) {
    size_t nBytesToRead = sizeof(T) * nItems;
    waitForInput(nBytesToRead);
    if (_shouldDisconnect) {
        disconnect();
    }
    if (!_isConnected && !_isConnecting) {
        return false;
    }
    std::lock_guard<std::mutex> inputLock(_inputQueueMutex);
    std::copy_n(_inputQueue.begin(), nBytesToRead, reinterpret_cast<char*>(getBuffer));
    _inputQueue.erase(_inputQueue.begin(), _inputQueue.begin() + nBytesToRead);
    return true;
}

template <typename T>
bool TcpSocket::peek(T* peekBuffer, size_t nItems) {
    size_t nBytesToRead = sizeof(T) * nItems;
    waitForInput(nBytesToRead);
    if (_shouldDisconnect) {
        disconnect();
    }
    if (!_isConnected && !_isConnecting) {
        return false;
    }
    std::lock_guard<std::mutex> inputLock(_inputQueueMutex);
    std::copy_n(_inputQueue.begin(), nBytesToRead, reinterpret_cast<char*>(peekBuffer));
    return true;
}

template <typename T>
bool TcpSocket::skip(size_t nItems) {
    size_t nBytesToSkip = sizeof(T) * nItems;
    waitForInput(nBytesToSkip);
    if (_shouldDisconnect) {
        disconnect();
    }
    if (!_isConnected && !_isConnecting) {
        return false;
    }
    std::lock_guard<std::mutex> inputLock(_inputQueueMutex);
    _inputQueue.erase(_inputQueue.begin(), _inputQueue.begin() + nBytesToSkip);
    return true;
}

template <typename T>
bool TcpSocket::put(const T* buffer, size_t nItems) {
    if (_shouldDisconnect) {
        disconnect();
    }
    std::lock_guard<std::mutex> outputLock(_outputQueueMutex);
    _outputQueue.insert(
        _outputQueue.end(),
        reinterpret_cast<const char*>(buffer),
        reinterpret_cast<const char*>(buffer + nItems)
    );
    _outputNotifier.notify_one();
    return _isConnected || _isConnecting;
}

} // namespace io
} // namespace ghoul
