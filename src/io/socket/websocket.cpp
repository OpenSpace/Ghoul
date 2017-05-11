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


namespace ghoul {
namespace io {



WebSocket::WebSocket() {

    // protocol types for websockets
    struct lws_protocols protocols[] =
    {
        {
            "http-only",
            nullptr,
            0
        },
        {
            "sgct",
            nullptr,
            0
        },
        {
            NULL, NULL, 0
        }
    };


    // server url will be ws://localhost:9000
    const char *interface = NULL;
    struct lws_context *context = NULL;

    // we're not using ssl
    const char *cert_path = NULL;
    const char *key_path = NULL;

    //lws_set_log_level(7, lwsl_emit_syslog);
    //lws_set_log_level(1, lwsl_emit_syslog);

    // no special options
    int opts = 0;

    // create connection struct
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof info);

    info.port = 8000;
    info.iface = interface;
    info.protocols = protocols;
    info.extensions = NULL;
    info.ssl_cert_filepath = cert_path;
    info.ssl_private_key_filepath = key_path;
    info.options = opts;
    info.gid = -1;
    info.uid = -1;

    context = lws_create_context(&info);
}

}
}
