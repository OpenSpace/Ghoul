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

#include <ghoul/opengl/debugcontext.h>

#include <ghoul/misc/assert.h>
#include <map>
#include <type_traits>

namespace ghoul::opengl::debug {

namespace {
void internalCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei,
                      const GLchar* message, const GLvoid* userParam)
{
    const CallbackFunction& cb = *reinterpret_cast<const CallbackFunction*>(userParam);
    cb(Source(source), Type(type), Severity(severity), id, std::string(message));
}
} // namespace

void setDebugOutput(DebugOutput debug, SynchronousOutput synchronous) {
    if (debug) {
        glEnable(GL_DEBUG_OUTPUT);
    }
    else {
        glDisable(GL_DEBUG_OUTPUT);
    }

    if (synchronous) {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    }
    else {
        glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    }
}

void setDebugMessageControl(Source source, Type type, Severity severity, Enabled enabled)
{
    glDebugMessageControl(
        static_cast<GLenum>(source),
        static_cast<GLenum>(type),
        static_cast<GLenum>(severity),
        0,
        nullptr,
        enabled ? GL_TRUE : GL_FALSE
    );
}

void setDebugMessageControl(Source source, Type type,
                            std::vector<unsigned int> identifiers, Enabled enabled)
{
    ghoul_assert(source != Source::DontCare, "source must not be Source::Dontcare");
    ghoul_assert(type != Type::DontCare, "type must not be Type::Dontcare");

    static_assert(
        std::is_same<unsigned int, GLuint>::value,
        "We exploit that 'int' and 'GLsizei' are the same for glDebugMessageControl"
    );

    glDebugMessageControl(
        static_cast<GLenum>(source),
        static_cast<GLenum>(type),
        GL_DONT_CARE,
        static_cast<GLsizei>(identifiers.size()),
        identifiers.data(),
        enabled ? GL_TRUE : GL_FALSE
    );
}

//using Callback = void *(Source source, Type type, Severity severity, unsigned int id
//  std::string message);
void setDebugCallback(CallbackFunction callback) {
    // We have to store the function pointer that is passed into this function locally as
    // it might otherwise be destroyed (and still be referenced by \c internalCallback.
    // In a perfect world, we'd want to capture the function pointer, but the OpenGL
    // callback only allows pure C functions
    static CallbackFunction storage;
    storage = callback;
    glDebugMessageCallback(internalCallback, &storage);
}

} // namespace ghoul::opengl::debug

namespace ghoul {

template <>
opengl::debug::Source from_string(const std::string& string) {
    static const std::map<std::string, opengl::debug::Source> Map = {
        { "API",             opengl::debug::Source::API },
        { "Window System",   opengl::debug::Source::WindowSystem },
        { "Shader Compiler", opengl::debug::Source::ShaderCompiler },
        { "Third Party",     opengl::debug::Source::ThirdParty },
        { "Application",     opengl::debug::Source::Application },
        { "Other",           opengl::debug::Source::Other },
        { "Don't care",      opengl::debug::Source::DontCare }
    };

    // Throws a std::out_of_range exception if the source could not be found
    return Map.at(string);
}

template <>
opengl::debug::Type from_string(const std::string& string) {
    static const std::map<std::string, opengl::debug::Type> Map = {
        { "Error",          opengl::debug::Type::Error },
        { "Deprecated",     opengl::debug::Type::Deprecated },
        { "Undefined",      opengl::debug::Type::Undefined },
        { "Portability",    opengl::debug::Type::Portability },
        { "Performance",    opengl::debug::Type::Performance },
        { "Marker",         opengl::debug::Type::Marker },
        { "Push group",     opengl::debug::Type::PushGroup },
        { "Pop group",      opengl::debug::Type::PopGroup },
        { "Other",          opengl::debug::Type::Other },
        { "Don't care",     opengl::debug::Type::DontCare }
    };

    // Throws a std::out_of_range exception if the type could not be found
    return Map.at(string);
}

template <>
opengl::debug::Severity from_string(const std::string& string) {
    static const std::map<std::string, opengl::debug::Severity> Map = {
        { "High",           opengl::debug::Severity::High },
        { "Medium",         opengl::debug::Severity::Medium },
        { "Low",            opengl::debug::Severity::Low },
        { "Notification",   opengl::debug::Severity::Notification }
    };

    // Throws a std::out_of_range exception if the severity could not be found
    return Map.at(string);
}

template <>
std::string to_string(const ghoul::opengl::debug::Source& value) {
    switch (value) {
        case ghoul::opengl::debug::Source::API:             return "API";
        case ghoul::opengl::debug::Source::WindowSystem:    return "Window System";
        case ghoul::opengl::debug::Source::ShaderCompiler:  return "Shader Compiler";
        case ghoul::opengl::debug::Source::ThirdParty:      return "Third Party";
        case ghoul::opengl::debug::Source::Application:     return "Application";
        case ghoul::opengl::debug::Source::Other:           return "Other";
        case ghoul::opengl::debug::Source::DontCare:        return "Don't care";
        default:                                      throw ghoul::MissingCaseException();
    }
}

template <>
std::string to_string(const ghoul::opengl::debug::Type& value) {
    switch (value) {
        case ghoul::opengl::debug::Type::Error:         return "Error";
        case ghoul::opengl::debug::Type::Deprecated:    return "Deprecated";
        case ghoul::opengl::debug::Type::Undefined:     return "Undefined";
        case ghoul::opengl::debug::Type::Portability:   return "Portability";
        case ghoul::opengl::debug::Type::Performance:   return "Performance";
        case ghoul::opengl::debug::Type::Marker:        return "Marker";
        case ghoul::opengl::debug::Type::PushGroup:     return "Push group";
        case ghoul::opengl::debug::Type::PopGroup:      return "Pop group";
        case ghoul::opengl::debug::Type::Other:         return "Other";
        case ghoul::opengl::debug::Type::DontCare:      return "Don't care";
        default:                                      throw ghoul::MissingCaseException();
    }
}

template <>
std::string to_string(const ghoul::opengl::debug::Severity& value) {
    switch (value) {
        case ghoul::opengl::debug::Severity::High:          return "High";
        case ghoul::opengl::debug::Severity::Medium:        return "Medium";
        case ghoul::opengl::debug::Severity::Low:           return "Low";
        case ghoul::opengl::debug::Severity::Notification:  return "Notification";
        default:                                      throw ghoul::MissingCaseException();
    }
}

} // namespace ghoul
