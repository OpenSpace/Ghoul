/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2025                                                               *
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

#ifndef __GHOUL___GHOUL_GL___H__
#define __GHOUL___GHOUL_GL___H__

#ifdef __APPLE__
    #define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
    #undef __gl_h_
#endif

#ifdef __clang__
#pragma clang diagnostic push
#ifndef __APPLE__
// The Apple Clang compiler doesn't understand this warning yet
#pragma clang diagnostic ignored "-Wdeprecated-copy"
#endif // __APPLE__
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif

#include <glbinding/gl41core/gl.h>
#include <glbinding/Binding.h>
#define __GL_H__

#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// Evil 'using namespace' in the header to make the usage of OpenGL less painful
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wheader-hygiene"
#endif // __clang__

using namespace gl;

#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__

namespace ghoul {

struct GLDebugGroup {
    explicit GLDebugGroup(std::string_view name) {
        if (glbinding::Binding::PushDebugGroup.isResolved()) {
            glPushDebugGroup(
                GL_DEBUG_SOURCE_APPLICATION,
                0,
                static_cast<GLsizei>(name.length()),
                name.data()
            );
        }
    }

    ~GLDebugGroup() {
        if (glbinding::Binding::PopDebugGroup.isResolved()) {
            glPopDebugGroup();
        }
    }
};

} // namespace ghoul

#endif // __GHOUL___GHOUL_GL___H__
