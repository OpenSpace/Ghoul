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

#include <ghoul/io/socket/websocket.h>
#include <fmt/format.h>

namespace {
    const std::string _loggerCat = "WebSocket";
}

namespace ghoul {
namespace io {

WebSocket::WebSocket(int portNumber) : port(portNumber) {
    // protocol types for WebSockets
    struct lws_protocols protocols[] = {
            {
                "http-only",
                (lws_callback_function*) WebSocket::callbackHttp,
                0
            }, {
                "sgct",
                nullptr,
                0
            }, {
                "webgui",
                (lws_callback_function*) WebSocket::callbackWS,
                0
            }, {
                NULL, NULL, 0
            }
        };

    // server url will be ws://localhost:<port>
    const char *interface = NULL;

    // we're not using ssl
    char *cert_path;
    char *key_path;
    if (use_ssl) {
        // TODO: we really should use ssl
    } else {
        cert_path = NULL;
        key_path = NULL;
    }

    //lws_set_log_level(7, lwsl_emit_syslog);
    //lws_set_log_level(1, lwsl_emit_syslog);

    // no special options
    unsigned int opts = 0;

    // initiate connection struct
    memset(&info, 0, sizeof info);

    info.port = port;
    info.iface = interface;
    info.protocols = protocols;
    info.extensions = NULL;
    info.ssl_cert_filepath = cert_path;
    info.ssl_private_key_filepath = key_path;
    info.options = opts;
    info.gid = -1;
    info.uid = -1;
}

bool WebSocket::initialize() {
    struct lws_context *context = NULL;
    context = lws_create_context(&info);

    if (context == NULL) {
        LERROR("Could not create LibWebSocket context.");
        return false;
    }

    LDEBUG(fmt::format("WebSocket available on port {}.", port));

    return true;
}

bool WebSocket::getMessage(std::string& message)
{
    return false;
}

bool WebSocket::putMessage(const std::string& message)
{
    return false;
}

bool WebSocket::isConnected() {
    return clientCount > 0;
}

int WebSocket::callbackHttp(struct lws* wsi,
                            enum lws_callback_reasons reason, void *user,
                            void *in, size_t len) {
    LERROR("HTTP connections not supported.");
    return 0;
}

int WebSocket::callbackWS(struct lws* wsi,
                          enum lws_callback_reasons reason, void *user,
                          void *in, size_t len) {
    LERROR("Received poke at callbackWS");

    switch(reason) {
        case LWS_CALLBACK_CLIENT_WRITEABLE:
//            clientCount++;
//            LDEBUG(fmt::format("Client connected. Client count: {}", clientCount));
            LDEBUG("Client connected.");
            break;
        case LWS_CALLBACK_RECEIVE:
            LDEBUG("Receiving message from WS Client.");
//            auto msg = (char*) in;
            break;
        default:
            LWARNING(fmt::format("Unhandled callback in callbackWS: {}", reason));
    }

    return 0;
}

}
}
