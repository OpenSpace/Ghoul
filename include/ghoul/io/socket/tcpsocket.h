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

#ifndef __GHOUL___TCPSOCKET___H__
#define __GHOUL___TCPSOCKET___H__

#include <ghoul/io/socket/socket.h>

#include <ghoul/misc/exception.h>
#include <array>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

 // OS specific socket implementation normalization.
#ifdef WIN32
using _SOCKET = size_t;
using _SOCKLEN = int;
#else //linux & macOS

#include <sys/socket.h>
#include <sys/types.h>

using _SOCKET = int;
using _SOCKLEN = socklen_t;

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (_SOCKET)(~0)
#endif // INVALID_SOCKET

#endif // WIN32

struct addrinfo;

namespace ghoul::io {

class TcpSocketServer;

class TcpSocket : public Socket {
public:
    struct TcpSocketError : public ghoul::RuntimeError {
        explicit TcpSocketError(std::string message, std::string component = "");
    };

    TcpSocket(std::string address, int port);
    TcpSocket(std::string address, int port, _SOCKET socket);
    virtual ~TcpSocket();
    void connect();
    void startStreams();
    void disconnect(int reason = 0) override;
    bool isConnected() override;
    bool isConnecting() override;

    virtual std::string address() const;
    virtual int port() const;

    bool getMessage(std::string& message) override;
    bool putMessage(const std::string& message) override;
    void setDelimiter(char delim);

    static void initializeNetworkApi();
    static bool initializedNetworkApi();

protected:
    /*
    * Read size bytes from the socket, store them in buffer and dequeue them from input.
    * Block until size bytes have been read.
    * Return false if this fails. Use error() to get the socket error code.
    */
    virtual bool getBytes(char* buffer, size_t nItems = 1);

    /*
    * Read size bytes from the socket, store them in buffer.
    * Do NOT dequeue them from input.
    * Block until size bytes have been read.
    * Return false if this fails. Use error() to get the socket error code.
    */
    virtual bool peekBytes(char* buffer, size_t nItems);

    /*
    * Skip size bytes from the socket.
    * Block until size bytes have been read.
    * Return false if this fails. Use error() to get the socket error code.
    */
    virtual bool skipBytes(size_t nItems);

    /**
    * Write size bytes from buffer into the socket.
    * Return false if this fails. Use error() to get the socket error code.
    */
    virtual bool putBytes(const char* buffer, size_t size = 1);

    const std::string _address;
    const int _port;
    std::atomic<bool> _isConnected;
    std::atomic<bool> _isConnecting;
    std::atomic<bool> _shouldDisconnect;
    std::atomic<bool> _error;

    _SOCKET _socket;
    TcpSocketServer* _server;
    std::thread _inputThread;
    std::thread _outputThread;

private:
    void establishConnection(addrinfo* info);
    void streamInput();
    void streamOutput();
    int waitForDelimiter();
    bool waitForInput(size_t nBytes);
    bool waitForOutput(size_t nBytes);

    std::mutex _inputBufferMutex;
    std::mutex _inputQueueMutex;
    std::condition_variable _inputNotifier;
    std::deque<char> _inputQueue;
    std::array<char, 4096> _inputBuffer;

    std::mutex _outputBufferMutex;
    std::mutex _outputQueueMutex;
    std::condition_variable _outputNotifier;
    std::deque<char> _outputQueue;
    std::array<char, 4096> _outputBuffer;

    std::atomic<char> _delimiter;

    static std::atomic<bool> _initializedNetworkApi;
};

} // namespace ghoul::io

#endif // __GHOUL___TCPSOCKET___H__
