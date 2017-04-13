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

#ifndef __GHOUL___DEBUGCONTEXT___H__
#define __GHOUL___DEBUGCONTEXT___H__

#include <ghoul/misc/boolean.h>
#include <ghoul/misc/fromstring.h>
#include <ghoul/opengl/ghoul_gl.h>

#include <string>
#include <vector>

namespace ghoul {
namespace opengl {
namespace debug {

using DebugOutput = ghoul::Boolean;
using SynchronousOutput = ghoul::Boolean;
using Enabled = ghoul::Boolean;

enum class Source : GLenum {
    API = GL_DEBUG_SOURCE_API,
    WindowSystem = GL_DEBUG_SOURCE_WINDOW_SYSTEM,
    ShaderCompiler = GL_DEBUG_SOURCE_SHADER_COMPILER,
    ThirdParty = GL_DEBUG_SOURCE_THIRD_PARTY,
    Application = GL_DEBUG_SOURCE_APPLICATION,
    Other = GL_DEBUG_SOURCE_OTHER,
    DontCare = GL_DONT_CARE
};


enum class Type : GLenum {
    Error = GL_DEBUG_TYPE_ERROR,
    Deprecated = GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR,
    Undefined = GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR,
    Portability = GL_DEBUG_TYPE_PORTABILITY,
    Performance = GL_DEBUG_TYPE_PERFORMANCE,
    Marker = GL_DEBUG_TYPE_MARKER,
    PushGroup = GL_DEBUG_TYPE_PUSH_GROUP,
    PopGroup = GL_DEBUG_TYPE_POP_GROUP,
    Other = GL_DEBUG_TYPE_OTHER,
    DontCare = GL_DONT_CARE
};

enum class Severity : GLenum {
    High = GL_DEBUG_SEVERITY_HIGH,
    Medium = GL_DEBUG_SEVERITY_MEDIUM,
    Low = GL_DEBUG_SEVERITY_LOW,
    Notification = GL_DEBUG_SEVERITY_NOTIFICATION,
    Dontcare = GL_DONT_CARE
};



void setDebugOutput(DebugOutput debug, SynchronousOutput synchronous);

void setDebugMessageControl(Source source, Type type, Severity severity, Enabled enabled);
void setDebugMessageControl(Source source, Type type,
    std::vector<unsigned int> identifiers, Enabled enabled);

} // namespace debug
} // namespace opengl

template <>
opengl::debug::Source from_string(const std::string& source);

template <>
opengl::debug::Type from_string(const std::string& source);

template <>
opengl::debug::Severity from_string(const std::string& source);

} // namespace ghoul

namespace std {

std::string to_string(ghoul::opengl::debug::Source source);
std::string to_string(ghoul::opengl::debug::Type type);
std::string to_string(ghoul::opengl::debug::Severity severity);

} // namespace std

#endif // __GHOUL___DEBUGCONTEXT___H__
