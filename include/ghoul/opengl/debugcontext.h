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

namespace ghoul::opengl::debug {

/// A boolean value determining whether we want to enable or disable the debug output
using DebugOutput = ghoul::Boolean;
/// A boolean value determining whether we want to force synchronous output 
using SynchronousOutput = ghoul::Boolean;
/// A boolean value determining whether to enable individual or groups of messages
using Enabled = ghoul::Boolean;

/**
 * The different sources from which a debug message in OpenGL can originate.
 */
enum class Source : std::underlying_type_t<GLenum> {
    /// Originating from the OpenGL API itself
    API = static_cast<std::underlying_type_t<GLenum>>(GL_DEBUG_SOURCE_API),
    /// Originating from the windowing system
    WindowSystem = static_cast<std::underlying_type_t<GLenum>>(
        GL_DEBUG_SOURCE_WINDOW_SYSTEM
    ),
    /// Originating from the shader compiler
    ShaderCompiler = static_cast<std::underlying_type_t<GLenum>>(
        GL_DEBUG_SOURCE_SHADER_COMPILER
    ),
    /// Originating from a third party library that is not the application itself
    ThirdParty = static_cast<std::underlying_type_t<GLenum>>(GL_DEBUG_SOURCE_THIRD_PARTY),
    /// Originating from the application managing the OpenGL context
    Application = static_cast<std::underlying_type_t<GLenum>>(
        GL_DEBUG_SOURCE_APPLICATION
    ),
    /// Originating from other sources
    Other = static_cast<std::underlying_type_t<GLenum>>(GL_DEBUG_SOURCE_OTHER),
    /// Used in the setDebugMessageControl functions to refer to any source
    DontCare = static_cast<std::underlying_type_t<GLenum>>(GL_DONT_CARE)
};

/**
 * The different types of messages that can be raised by the OpenGL driver. Descriptions
 * for individual values are taken from:
 * https://www.khronos.org/registry/OpenGL/extensions/KHR/KHR_debug.txt
 */
enum class Type : std::underlying_type_t<GLenum> {
    /// Events that generated an error
    Error = static_cast<std::underlying_type_t<GLenum>>(GL_DEBUG_TYPE_ERROR),
    /// Behavior that has been marked for deprecation
    Deprecated = static_cast<std::underlying_type_t<GLenum>>(
        GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR
    ),
    /// Behavior that is undefined according to the specification
    Undefined = static_cast<std::underlying_type_t<GLenum>>(
        GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR
    ),
    /// Use of extensions or shaders in a way that is highly vendor-specific
    Portability = static_cast<std::underlying_type_t<GLenum>>(GL_DEBUG_TYPE_PORTABILITY),
    /// Implementation-dependent performance warnings
    Performance = static_cast<std::underlying_type_t<GLenum>>(GL_DEBUG_TYPE_PERFORMANCE),
    // These three values are not available on OpenGL prior to 4.3
    /// Annotation of the command stream
    Marker = static_cast<std::underlying_type_t<GLenum>>(GL_DEBUG_TYPE_MARKER),
    /// Entering a debug group
    PushGroup = static_cast<std::underlying_type_t<GLenum>>(GL_DEBUG_TYPE_PUSH_GROUP),
    /// Leaving a debug group
    PopGroup = static_cast<std::underlying_type_t<GLenum>>(GL_DEBUG_TYPE_POP_GROUP),
    /// Other type
    Other = static_cast<std::underlying_type_t<GLenum>>(GL_DEBUG_TYPE_OTHER),
    /// Used in the setDebugMessageControl functions to refer to any type
    DontCare = static_cast<std::underlying_type_t<GLenum>>(GL_DONT_CARE)
};

/**
 * The severity of the emitted message. Descriptions for individual values are taken from:
 * https://www.khronos.org/registry/OpenGL/extensions/KHR/KHR_debug.txt
 */
enum class Severity : std::underlying_type_t<GLenum> {
    /// Any GL error; dangerous undefined behavior; any GLSL or ARB shader compiler and
    /// linker errors
    High = static_cast<std::underlying_type_t<GLenum>>(GL_DEBUG_SEVERITY_HIGH),
    /// Severe performance warnings; GLSL or other shader compiler and linker warnings;
    ///use of currently deprecated behavior
    Medium = static_cast<std::underlying_type_t<GLenum>>(GL_DEBUG_SEVERITY_MEDIUM),
    /// Performance warnings from redundant state changes; trivial undefined behavior
    Low = static_cast<std::underlying_type_t<GLenum>>(GL_DEBUG_SEVERITY_LOW),
    /// Any message which is not an error or performance concern
    Notification = static_cast<std::underlying_type_t<GLenum>>(
        GL_DEBUG_SEVERITY_NOTIFICATION
    ),
    /// Used in the setDebugMessageControl functions to refer to any severity
    Dontcare = static_cast<std::underlying_type_t<GLenum>>(GL_DONT_CARE)
};

/**
 * Enables the debug context mode for the currently active OpenGL context. Calling this
 * function outside of a valid OpenGL context is undefined behavior.
 * \param debug Determines whether the context's debug mode should be enabled or disabled
 * \param synchronous Determines whether the debug messages are provided synchronously
 * (i.e. on the same thread as the OpenGL context and in the same stack frame), or whether
 * debug callbacks can occur at any time.
 */
void setDebugOutput(DebugOutput debug, SynchronousOutput synchronous);

/**
 * Enables or disables debug messages for the provided \p source, \p type, and 
 * \p severity. Any of these values can be their respective \c Dontcare values if any
 * value should be modified.
 * \param source The Source of debug messages that should be modified
 * \param type The Type of debug messages that should be modified
 * \param severity The Severity of debug messages that should be modified
 * \param enabled Whether messages of the specific \p source, \p type, and \p severity
 * should be enabled or disabled
 */
void setDebugMessageControl(Source source, Type type, Severity severity, Enabled enabled);

/**
 * Enables or disables a set of debug messages for the provided \p source, \p type 
 * identified by a set of \p identifiers.
 * \param source The Source of debug messages that should be modified
 * \param type The Type of debug messages that should be modified
 * \param identifiers A list of identifiers that should be modified
 * \param enabled Whether messages of the specific \p source, \p type, and \p severity
 * should be enabled or disabled
 * \pre \p source must not be Source::Dontcare
 * \pre \p type must not be Type::Dontcare
  */
void setDebugMessageControl(Source source, Type type,
    std::vector<unsigned int> identifiers, Enabled enabled);

/**
 * The callback function that is passed to the setDebugCallback function and that is
 * called whenever a valid message is reported by the GL.
 * \param source The source of the debug message
 * \param type The type of the debug message
 * \param severity The severity of the debug message
 * \param id The id of the message
 * \param message The actually message that was generated by the GL
 */
using CallbackFunction = void (*)(Source source, Type type, Severity severity,
    unsigned int id, std::string message);

/**
 * Sets the debug callback that is called whenever the GL reports a new debug message.
 * Only a C-style function pointer or a state-less lambda can be passed.
 * \param callback The CallbackFunction that is called
 */
void setDebugCallback(CallbackFunction callback);

} // namespace ghoul::opengl::debug

namespace ghoul {
    /**
    * Converts a string with a \p source into a opengl::debug::Source object.
    * The valid values are "API", "Window System", "Shader Compiler", "Third Party",
    * "Application", "Other", and "Don't care".
    * \param source The string containing the name of a opengl::debug::Source
    * \throw std::out_of_range if the \p source is not a valid name of an
    * opengl::debug::Source object
    */
    template <>
    opengl::debug::Source from_string(const std::string& source);

    /**
    * Converts a string with a \p type into a opengl::debug::Source object.
    * The valid values are "Error", "Deprecated", "Undefined", "Portability",
    * "Performance", "Marker", "Push group", "Pop group", "Other", and "Don't care".
    * \param type The string containing the name of a opengl::debug::Type
    * \throw std::out_of_range if the \p type is not a valid name of an
    * opengl::debug::Type object
    */
    template <>
    opengl::debug::Type from_string(const std::string& type);

    /**
    * Converts a string with a \p severity into a opengl::debug::Severity object.
    * The valid values are "High", "Medium", "Low", and "Notification".
    * \param severity The string containing the name of a opengl::debug::Severity
    * \throw std::out_of_range if the \p severity is not a valid name of an
    * opengl::debug::Severity object
    */
    template <>
    opengl::debug::Severity from_string(const std::string& severity);

} // namespace ghoul

namespace std {

/**
 * Converts the \p source object into its string representation.
 * The valid return values are: "API", "Window System", "Shader Compiler", "Third Party",
 * "Application", "Other", and "Don't care".
 * \param source The ghoul::opengl::debug::Source that is converted into a string
 * \return The string representation of the \p source
 */
std::string to_string(ghoul::opengl::debug::Source source);

/**
 * Converts the \p type object into its string representation.
 * The valid return values are: "Error", "Deprecated", "Undefined", "Portability",
 * "Performance", "Marker", "Push group", "Pop group", "Other", and "Don't care".
 * \param type The ghoul::opengl::debug::Type that is converted into a string
 * \return The string representation of the \p type
 */
std::string to_string(ghoul::opengl::debug::Type type);

/**
 * Converts the \p severity object into its string representation.
 * The valid return values are: "High", "Medium", "Low", and "Notification".
 * \param severity The ghoul::opengl::debug::Severity that is converted into a string
 * \return The string representation of the \p severity
 */
std::string to_string(ghoul::opengl::debug::Severity severity);

} // namespace std

#endif // __GHOUL___DEBUGCONTEXT___H__
