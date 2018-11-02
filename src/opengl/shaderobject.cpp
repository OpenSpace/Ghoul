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

#include <ghoul/opengl/shaderobject.h>

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/filesystem/cachemanager.h>
#include <ghoul/misc/crc32.h>
#include <algorithm>
#include <functional>
#include <fstream>

namespace ghoul::opengl {

ShaderObject::ShaderObjectError::ShaderObjectError(std::string msg)
    : RuntimeError(std::move(msg), "ShaderObject")
{}

ShaderObject::ShaderCompileError::ShaderCompileError(std::string error,
                                                     std::string ident,
                                                     std::string name)
    : ShaderObjectError(
        name.empty() ?
        "Error linking program object: " + error + "\n" + ident :
        "Error linking program object [" + name + "]: " + error + "\n" + ident
    )
    , compileError(std::move(error))
    , fileIdentifiers(std::move(ident))
    , shaderName(std::move(name))
{}

ShaderObject::ShaderObject(ShaderType shaderType, Dictionary dictionary)
    : _type(shaderType)
    , _loggerCat("ShaderObject")
{
    _id = glCreateShader(static_cast<GLenum>(_type));
    _preprocessor.setDictionary(std::move(dictionary));
    _preprocessor.setFilename("");

    if (_id == 0) {
        throw ShaderObjectError("glCreateShader returned 0");
    }
}

ShaderObject::ShaderObject(ShaderType shaderType, std::string filename,
                           Dictionary dictionary)
    : _type(shaderType)
    , _loggerCat("ShaderObject")
{
    ghoul_assert(!filename.empty(), "Filename must not be empty");

    _id = glCreateShader(static_cast<GLenum>(_type));
    if (_id == 0) {
        throw ShaderObjectError("glCreateShader returned 0");
    }
    _preprocessor.setDictionary(std::move(dictionary));
    _preprocessor.setFilename(std::move(filename));

    rebuildFromFile();
}

ShaderObject::ShaderObject(ShaderType shaderType, std::string filename,
                           std::string name, Dictionary dictionary)
    : _type(shaderType)
    , _shaderName(std::move(name))
    , _loggerCat("ShaderObject('" + _shaderName + "')")
{
    ghoul_assert(!filename.empty(), "Filename must not be empty");

    _id = glCreateShader(static_cast<GLenum>(_type));
    if (_id == 0) {
        throw ShaderObjectError("glCreateShader returned 0");
    }

#ifdef GL_VERSION_4_3
    if (glbinding::Binding::ObjectLabel.isResolved()) {
        glObjectLabel(
            GL_SHADER,
            _id,
            GLsizei(_shaderName.length() + 1),
            _shaderName.c_str()
        );
    }
#endif
    _preprocessor.setFilename(std::move(filename));
    _preprocessor.setDictionary(std::move(dictionary));

    rebuildFromFile();
}

ShaderObject::ShaderObject(const ShaderObject& cpy)
    : _id(0)
    , _type(cpy._type)
    , _shaderName(cpy._shaderName)
    , _loggerCat(cpy._loggerCat)
    , _onChangeCallback(cpy._onChangeCallback)
    , _preprocessor(cpy._preprocessor)
{
    _id = glCreateShader(static_cast<GLenum>(_type));
    if (_id == 0) {
        throw ShaderObjectError("glCreateShader returned 0");
    }
#ifdef GL_VERSION_4_3
    if (glbinding::Binding::ObjectLabel.isResolved()) {
        glObjectLabel(
            GL_SHADER,
            _id,
            GLsizei(_shaderName.length() + 1),
            _shaderName.c_str()
        );
    }
#endif
    setShaderObjectCallback(_onChangeCallback);
    rebuildFromFile();
}

ShaderObject::ShaderObject(ShaderObject&& rhs) {
    if (this != &rhs) {
        _id = std::move(rhs._id);

        _type = rhs._type;
        _shaderName = std::move(rhs._shaderName);
        _loggerCat = std::move(rhs._loggerCat);
        _onChangeCallback = std::move(rhs._onChangeCallback);
        _preprocessor = std::move(rhs._preprocessor);
        setShaderObjectCallback(rhs._onChangeCallback);
    }
}

ShaderObject::~ShaderObject() {
    glDeleteShader(_id);
    _id = 0;
}

ShaderObject::operator GLuint() const {
    return _id;
}

ShaderObject& ShaderObject::operator=(const ShaderObject& rhs) {
    if (this != &rhs) {
        _type = rhs._type;
        _shaderName = rhs._shaderName;
        _loggerCat = rhs._loggerCat;
        _onChangeCallback = rhs._onChangeCallback;
        _preprocessor = rhs._preprocessor;
        setShaderObjectCallback(rhs._onChangeCallback);

        glDeleteShader(_id);
        _id = glCreateShader(static_cast<GLenum>(_type));
        if (_id == 0) {
            throw ShaderObjectError("glCreateShader returned 0");
        }
#ifdef GL_VERSION_4_3
        if (glbinding::Binding::ObjectLabel.isResolved()) {
            glObjectLabel(
                GL_SHADER,
                _id,
                GLsizei(_shaderName.length() + 1),
                _shaderName.c_str()
            );
        }
#endif
        rebuildFromFile();
    }
    return *this;
}

ShaderObject& ShaderObject::operator=(ShaderObject&& rhs) {
    if (this != &rhs) {
        _id = std::move(rhs._id);
        _type = rhs._type;
        _shaderName = std::move(rhs._shaderName);
        _loggerCat = std::move(rhs._loggerCat);
        _onChangeCallback = std::move(rhs._onChangeCallback);
        _preprocessor = std::move(rhs._preprocessor);
        // Take ownership of the moved preprocessor
        setShaderObjectCallback(_onChangeCallback);

    }
    return *this;
}

void ShaderObject::setName(std::string name) {
    _shaderName = std::move(name);
    _loggerCat = "ShaderObject['" + _shaderName + "']";
#ifdef GL_VERSION_4_3
    if (glbinding::Binding::ObjectLabel.isResolved()) {
        glObjectLabel(
            GL_SHADER,
            _id,
            GLsizei(_shaderName.length() + 1),
            _shaderName.c_str()
        );
    }
#endif
}

const std::string& ShaderObject::name() const {
    return _shaderName;
}

void ShaderObject::setShaderObjectCallback(ShaderObjectCallback changeCallback) {
    _onChangeCallback = std::move(changeCallback);
    // The ShaderPreprocessor will take care to call the callback whenever the underlying
    // file changes, an included file changes, or the dictionary changes.
    _preprocessor.setCallback(_onChangeCallback);
}

void ShaderObject::setFilename(const std::string& filename) {
    ghoul_assert(!filename.empty(), "Filename must not be empty");
    if (!FileSys.fileExists(filename)) {
        throw FileNotFoundError(filename);
    }

    _preprocessor.setFilename(filename);
}

std::string ShaderObject::filename() {
    return _preprocessor.filename();
}

void ShaderObject::setDictionary(Dictionary dictionary) {
    _preprocessor.setDictionary(std::move(dictionary));
}

Dictionary ShaderObject::dictionary() {
    return _preprocessor.dictionary();
}

void ShaderObject::rebuildFromFile() {
    std::string contents;
    _preprocessor.process(contents);

    // If in debug mode, output the source to file
#ifdef GHL_DEBUG
    std::string generatedFilename;

    std::string baseName;
    if (_shaderName.empty()) {
        filesystem::File ghlFile(filename());
        baseName = ghlFile.baseName();
    } else {
        baseName = _shaderName;
    }

    if (FileSys.cacheManager()) {
        // we use the .baseName() version because otherwise we get a new file
        // every time we reload the shader
        generatedFilename = FileSys.cacheManager()->cachedFilename(
            baseName,
            "",
            filesystem::CacheManager::Persistent::Yes
        );
    }
    else {
        // Either the cachemanager wasn't initialized or the
        // filename could not be fetched
        generatedFilename += ".GhoulGenerated.glsl";
    }

    std::ofstream os;
    os.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    os.open(generatedFilename);
    os << contents;
    os.close();
#endif // GHL_DEBUG

    const char* contentPtr =  contents.c_str();
    glShaderSource(_id, 1, &contentPtr, nullptr);
}

void ShaderObject::deleteShader() {
    glDeleteShader(_id);
    _id = 0;
}

void ShaderObject::compile() {
    glCompileShader(_id);

    GLint compilationStatus;
    glGetShaderiv(_id, GL_COMPILE_STATUS, &compilationStatus);
    if (static_cast<GLboolean>(compilationStatus) == GL_FALSE) {
        GLint logLength;
        glGetShaderiv(_id, GL_INFO_LOG_LENGTH, &logLength);

        if (logLength == 0) {
            throw ShaderCompileError(
                "Unknown error",
                _preprocessor.getFileIdentifiersString(),
                name()
            );
        }

        std::vector<GLchar> log(logLength);
        glGetShaderInfoLog(_id, logLength, nullptr, log.data());
        std::string logMessage(log.data());
        throw ShaderCompileError(
            logMessage,
            _preprocessor.getFileIdentifiersString(),
            name()
        );
    }
}

std::string ShaderObject::typeAsString() const {
    return ShaderObject::stringForShaderType(_type);
}

std::string ShaderObject::stringForShaderType(ShaderType type) {
    switch (type) {
        case ShaderType::Vertex:                return "Vertex shader";
        case ShaderType::TesselationControl:    return "Tesselation Control shader";
        case ShaderType::TesselationEvaluation: return "Tesselation Evaluation shader";
        case ShaderType::Geometry:              return "Geometry shader";
        case ShaderType::Fragment:              return "Fragment shader";
        case ShaderType::Compute:               return "Compute shader";
        default:                                throw MissingCaseException();
    }
}

} // namespace ghoul::opengl
