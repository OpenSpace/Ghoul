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

#ifndef __GHOUL___WEBSOCKET___H__
#define __GHOUL___WEBSOCKET___H__

#include <libwebsockets.h>
#include <ghoul/io/socket/socket.h>
#include <ghoul/logging/logmanager.h>

namespace ghoul {
namespace io {

class WebSocket : public Socket {
public:
    WebSocket(int portNumber = 8000);
    bool initialize();
    void disconnect() override;
    bool isConnected() override;
    bool isConnecting() override;
    bool getMessage(std::string& message) override;
    bool putMessage(const std::string& message) override;

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

private:
    struct lws_context_creation_info info;
    bool use_ssl = false;
    int port;
    int clientCount = 0;

    static int callbackHttp(struct lws* wsi,
                     enum lws_callback_reasons reason, void *user,
                     void *in, size_t len);
    static int callbackWS(struct lws* wsi,
                   enum lws_callback_reasons reason, void *user,
                   void *in, size_t len);
};


} // namespace io
} // namespace ghoul

#endif // __GHOUL___WEBSOCKET___H__
