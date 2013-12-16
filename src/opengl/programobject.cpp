/**************************************************************************************************
 * GHOUL                                                                                          *
 * General Helpful Open Utility Library                                                           *
 *                                                                                                *
 * Copyright (c) 2012 Alexander Bock                                                              *
 *                                                                                                *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software  *
 * and associated documentation files (the "Software"), to deal in the Software without           *
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,     *
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the  *
 * Software is furnished to do so, subject to the following conditions:                           *
 *                                                                                                *
 * The above copyright notice and this permission notice shall be included in all copies or       *
 * substantial portions of the Software.                                                          *
 *                                                                                                *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING  *
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND     *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 *************************************************************************************************/

#include <ghoul/opengl/programobject.h>

#include <ghoul/opengl/ghoul_gl.h>
#include <ghoul/opengl/shaderobject.h>
#include <ghoul/logging/logmanager.h>
#include <glm/gtc/type_ptr.hpp>

using std::map;
using std::string;
using std::vector;
using glm::bvec2;
using glm::bvec3;
using glm::bvec4;
using glm::ivec2;
using glm::ivec3;
using glm::ivec4;
using glm::uvec2;
using glm::uvec3;
using glm::uvec4;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::dvec2;
using glm::dvec3;
using glm::dvec4;
using glm::mat2x2;
using glm::mat2x3;
using glm::mat2x4;
using glm::mat3x2;
using glm::mat3x3;
using glm::mat3x4;
using glm::mat4x2;
using glm::mat4x3;
using glm::mat4x4;
using glm::dmat2x2;
using glm::dmat2x3;
using glm::dmat2x4;
using glm::dmat3x2;
using glm::dmat3x3;
using glm::dmat3x4;
using glm::dmat4x2;
using glm::dmat4x3;
using glm::dmat4x4;
using glm::value_ptr;

namespace ghoul {
namespace opengl {

ProgramObject::ProgramObject()
    : _id(0)
    , _programName("")
    , _loggerCat("ProgramObject")
    , _ignoreUniformLocationError(false)
    , _ignoreAttributeLocationError(false)
    , _ignoreSubroutineLocationError(false)
    , _ignoreSubroutineUniformLocationError(false)
{
    _id = glCreateProgram();
    if (_id == 0)
        LERROR_SAFE("glCreateProgram returned 0");
}

ProgramObject::ProgramObject(const string& name)
    : _id(0)
    , _programName(name)
    , _loggerCat("ProgramObject('" + name + "')")
    , _ignoreUniformLocationError(false)
    , _ignoreAttributeLocationError(false)
    , _ignoreSubroutineLocationError(false)
    , _ignoreSubroutineUniformLocationError(false)
{
    _id = glCreateProgram();
    if (_id == 0)
        LERROR_SAFE("glCreateProgram returned 0");
    if (glObjectLabel) {
        glObjectLabel(
                      GL_PROGRAM,
                      _id,
                      static_cast<GLsizei>(_programName.length() + 1),
                      _programName.c_str());
    }
}

ProgramObject::ProgramObject(const ProgramObject& cpy)
    : _id(0)
    , _programName(cpy._programName)
    , _loggerCat(cpy._loggerCat)
    , _ignoreUniformLocationError(cpy._ignoreUniformLocationError)
    , _ignoreAttributeLocationError(cpy._ignoreAttributeLocationError)
    , _ignoreSubroutineLocationError(cpy._ignoreSubroutineLocationError)
    , _ignoreSubroutineUniformLocationError(cpy._ignoreSubroutineUniformLocationError)
{
    _id = glCreateProgram();
    if (_id == 0)
        LERROR_SAFE("glCreateProgram returned 0");
    if (glObjectLabel) {
        glObjectLabel(
                      GL_PROGRAM,
                      _id,
                      static_cast<GLsizei>(_programName.length() + 1),
                      _programName.c_str());
    }

    for (auto it = _shaderObjects.cbegin(); it != _shaderObjects.cend(); ++it) {
        if (it->second) {
            // ProgramObject owns ShaderObjects
            ShaderObject* shaderCopy = new ShaderObject(*(it->first));
            _shaderObjects.push_back(std::make_pair(shaderCopy, true));
        }
        else
            _shaderObjects.push_back(*it);
    }
}

ProgramObject::~ProgramObject() {
    for (auto it = _shaderObjects.begin(); it != _shaderObjects.end(); ++it) {
        // Only delete ShaderObjects that belong to this ProgramObject
        if (it->second)
            delete it->first;
    }
    _shaderObjects.clear();
    glDeleteProgram(_id);
}

ProgramObject::operator GLuint() const {
    return _id;
}

ProgramObject& ProgramObject::operator=(const ProgramObject& rhs) {
    if (this != &rhs) {
        _programName = rhs._programName;
        if (glObjectLabel) {
            glObjectLabel(
                          GL_PROGRAM,
                          _id,
                          static_cast<GLsizei>(rhs._programName.length() + 1),
                          rhs._programName.c_str());
        }
        _loggerCat = rhs._loggerCat;
        _ignoreUniformLocationError = rhs._ignoreUniformLocationError;
        _ignoreAttributeLocationError = rhs._ignoreAttributeLocationError;
        _ignoreSubroutineLocationError = rhs._ignoreSubroutineLocationError;
        _ignoreSubroutineUniformLocationError = rhs._ignoreSubroutineUniformLocationError;

        for (auto it = _shaderObjects.begin(); it != _shaderObjects.end(); ++it) {
            // Only delete ShaderObjects that belong to this ProgramObject
            if (it->second)
                delete it->first;
        } 
        _shaderObjects.clear();
        glDeleteProgram(_id);

        _id = glCreateProgram();
        if (_id == 0)
            LERROR_SAFE("glCreateProgram returned 0");
        if (glObjectLabel) {
            glObjectLabel(
                          GL_PROGRAM,
                          _id,
                          static_cast<GLsizei>(_programName.length() + 1),
                          _programName.c_str());
        }

        for (auto it = rhs._shaderObjects.cbegin(); it != rhs._shaderObjects.end(); ++it) {
            if (it->second) {
                // ProgramObject owns ShaderObjects
                ShaderObject* shaderCopy = new ShaderObject(*(it->first));
                _shaderObjects.push_back(std::make_pair(shaderCopy, true));
            }
            else
                _shaderObjects.push_back(*it);
        }
    }
    return *this;
}

void ProgramObject::setName(const string& name) {
    _programName = name;
    _loggerCat = "ProgramObject['" + name + "']";
    if (glObjectLabel) {
        glObjectLabel(
                      GL_PROGRAM,
                      _id,
                      static_cast<GLsizei>(_programName.length() + 1),
                      _programName.c_str());
    }
}

const string& ProgramObject::name() const{
    return _programName;
}

bool ProgramObject::hasName() const {
    return !_programName.empty();
}

void ProgramObject::attachObject(ShaderObject* shaderObject, bool transferOwnership) {
    // Check for null pointer
    if (shaderObject == nullptr) {
        LWARNING_SAFE("Passed ShaderObject is nullptr");
        return;
    }
#ifdef GHL_DEBUG
    // Check if ShaderObject is already attached
    for (auto it = _shaderObjects.begin(); it != _shaderObjects.end(); ++it) {
        if (it->first == shaderObject) {
            if (it->first->hasName())
                LWARNING_SAFE("Shader object [" + it->first->name() + "] already attached");
            else
                LWARNING_SAFE("ShaderObject already attached");
            return;
        }
    }
#endif
    glAttachShader(_id, *shaderObject);
    _shaderObjects.push_back(std::make_pair(shaderObject, transferOwnership));
}

void ProgramObject::detachObject(ShaderObject* shaderObject) {
    // Check for null pointer
    if (shaderObject == nullptr) {
        LWARNING_SAFE("Passed ShaderObject is nullptr");
        return;
    }

    // Check if ShaderObject is attached
    auto it = _shaderObjects.begin();
    for (; it != _shaderObjects.end(); ++it) {
        if (it->first == shaderObject)
            break;
    }
    if (it == _shaderObjects.end()) {
        if (shaderObject->hasName())
            LWARNING_SAFE("ShaderObject [" + shaderObject->name() + 
            "] not attached to ProgramObject");
        else
            LWARNING_SAFE("ShaderObject not attached to ProgramObject");
        return;
    }
    else {
        glDetachShader(_id, *shaderObject);

        if (it->second)
            // ShaderObject belongs to this ProgramObject
            delete shaderObject;
        _shaderObjects.erase(it);
    }
}

bool ProgramObject::compileShaderObjects() {
#ifdef GHL_DEBUG
    if (_shaderObjects.size() == 0) {
        LWARNING_SAFE("No ShaderObjects present for compiling");
        return false;
    }
#endif
    bool success = true;
    for (auto it = _shaderObjects.cbegin(); it != _shaderObjects.cend(); ++it)
        success &= it->first->compile();

    return success;
}

bool ProgramObject::linkProgramObject() {
#ifdef GHL_DEBUG
    if (_shaderObjects.size() == 0) {
        LWARNING_SAFE("No ShaderObjects present while linking");
        return false;
    }
#endif
    glLinkProgram(_id);

    GLint linkStatus;
    glGetProgramiv(_id, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE) {
        GLint logLength;
        glGetProgramiv(_id, GL_INFO_LOG_LENGTH, &logLength);

        if (logLength == 0) {
            LERROR_SAFE("ProgramObject linking error: Unknown error");
            return false;
        }

        GLchar* rawLog = new GLchar[logLength];
        glGetProgramInfoLog(_id, logLength, NULL, rawLog);
        string log(rawLog);
        delete[] rawLog;
        LERROR_SAFE("ProgramObject linking error:\n" + log);

        return false;
    }
    return linkStatus == GL_TRUE;
}

bool ProgramObject::rebuildFromFile() {
#ifdef GHL_DEBUG
    if (_shaderObjects.size() == 0) {
        LWARNING_SAFE("No ShaderObjects present while rebuilding");
        return false;
    }
#endif
    bool success = true;
    for (auto it = _shaderObjects.cbegin(); it != _shaderObjects.cend(); ++it)
        success &= it->first->rebuildFromFile();

    return success;
}

void ProgramObject::activate() {
#ifdef GHL_DEBUG
    if (_shaderObjects.size() == 0) {
        LWARNING_SAFE("No ShaderObjects present while activating");
        return;
    }
#endif
    glUseProgram(_id);
}

void ProgramObject::deactivate() {
    glUseProgram(0);
}

void ProgramObject::setIgnoreUniformLocationError(bool ignoreError) {
    _ignoreUniformLocationError = ignoreError;
}

bool ProgramObject::ignoreUniformLocationError() const {
    return _ignoreUniformLocationError;
}

GLint ProgramObject::uniformLocation(const string& name) const {
    GLint location = glGetUniformLocation(_id, name.c_str());
    if (!_ignoreUniformLocationError && location == -1)
        LWARNING_SAFE("Failed to locate uniform location for: " + name);
    return location;
}

bool ProgramObject::setUniform(const string& name, bool value) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const string& name, bool v1, bool v2) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, v1, v2);
    return true;
}

bool ProgramObject::setUniform(const string& name, bool v1, bool v2, bool v3) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, v1, v2, v3);
    return true;
}

bool ProgramObject::setUniform(const string& name, bool v1, bool v2, bool v3, bool v4) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setUniform(const string& name, const glm::bvec2& value) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const string& name, const glm::bvec3& value) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const string& name, const glm::bvec4& value) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const string& name, const bool* values, int count) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const string& name, glm::bvec2* values, int count) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const string& name, glm::bvec3* values, int count) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const string& name, glm::bvec4* values, int count) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const string& name, GLuint value) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const string& name, GLuint v1, GLuint v2) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, v1, v2);
    return true;
}

bool ProgramObject::setUniform(const string& name, GLuint v1, GLuint v2, GLuint v3) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, v1, v2, v3);
    return true;
}

bool ProgramObject::setUniform(const string& name, GLuint v1, GLuint v2, GLuint v3, GLuint v4) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setUniform(const string& name, const glm::uvec2& value) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const string& name, const glm::uvec3& value) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const string& name, const glm::uvec4& value) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const string& name, const GLuint* values, int count) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const string& name, glm::uvec2* values, int count) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const string& name, glm::uvec3* values, int count) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const string& name, glm::uvec4* values, int count) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const string& name, GLint value) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const string& name, GLint v1, GLint v2) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, v1, v2);
    return true;
}

bool ProgramObject::setUniform(const string& name, GLint v1, GLint v2, GLint v3) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, v1, v2, v3);
    return true;
}

bool ProgramObject::setUniform(const string& name, GLint v1, GLint v2, GLint v3, GLint v4) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setUniform(const string& name, const glm::ivec2& value) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const string& name, const glm::ivec3& value) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const string& name, const glm::ivec4& value) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const string& name, const GLint* values, int count) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const string& name, glm::ivec2* values, int count) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const string& name, glm::ivec3* values, int count) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const string& name, glm::ivec4* values, int count) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const string& name, GLfloat value) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const string& name, GLfloat v1, GLfloat v2) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, v1, v2);
    return true;
}

bool ProgramObject::setUniform(const string& name, GLfloat v1, GLfloat v2, GLfloat v3) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, v1, v2, v3);
    return true;
}

bool ProgramObject::setUniform(const string& name, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setUniform(const string& name, const vec2& value) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const string& name, const vec3& value) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const string& name, const vec4& value) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const string& name, const GLfloat* values, int count) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const string& name, vec2* values, int count) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const string& name, vec3* values, int count) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const string& name, vec4* values, int count) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const string& name, GLdouble value) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const string& name, GLdouble v1, GLdouble v2) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, v1, v2);
    return true;
}

bool ProgramObject::setUniform(const string& name, GLdouble v1, GLdouble v2, GLdouble v3) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, v1, v2, v3);
    return true;
}

bool ProgramObject::setUniform(const string& name, GLdouble v1, GLdouble v2, GLdouble v3, GLdouble v4) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setUniform(const string& name, const dvec2& value) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const string& name, const dvec3& value) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const string& name, const dvec4& value) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const string& name, const GLdouble* values, int count) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const string& name, dvec2* values, int count) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const string& name, dvec3* values, int count) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const string& name, dvec4* values, int count) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const string& name, const mat2x2& value, bool transpose) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const string& name, const mat2x3& value, bool transpose) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const string& name, const mat2x4& value, bool transpose) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const string& name, const mat3x2& value, bool transpose) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const string& name, const mat3x3& value, bool transpose) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const string& name, const mat3x4& value, bool transpose) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const string& name, const mat4x2& value, bool transpose) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const string& name, const mat4x3& value, bool transpose) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const string& name, const mat4x4& value, bool transpose) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const string& name, const dmat2x2& value, bool transpose) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const string& name, const dmat2x3& value, bool transpose) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const string& name, const dmat2x4& value, bool transpose) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const string& name, const dmat3x2& value, bool transpose) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const string& name, const dmat3x3& value, bool transpose) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const string& name, const dmat3x4& value, bool transpose) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const string& name, const dmat4x2& value, bool transpose) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const string& name, const dmat4x3& value, bool transpose) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const string& name, const dmat4x4& value, bool transpose) {
    const GLint location = uniformLocation(name);
    if (location == -1)
        return false;
    setUniform(location, value, transpose);
    return true;
}

void ProgramObject::setUniform(GLint location, bool value) {
    if (glProgramUniform1i)
        glProgramUniform1i(_id, location, value);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1i(location, value);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, bool v1, bool v2) {
    if (glProgramUniform2i)
        glProgramUniform2i(_id, location, v1, v2);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2i(location, v1, v2);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, bool v1, bool v2, bool v3) {
    if (glProgramUniform3i)
        glProgramUniform3i(_id, location, v1, v2, v3);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3i(location, v1, v2, v3);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, bool v1, bool v2, bool v3, bool v4) {
    if (glProgramUniform4i)
        glProgramUniform4i(_id, location, v1, v2, v3, v4);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4i(location, v1, v2, v3, v4);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const bvec2& value) {
    if (glProgramUniform2iv)
        glProgramUniform2iv(_id, location, 1, glm::value_ptr(glm::ivec2(value)));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2iv(location, 1, glm::value_ptr(glm::ivec2(value)));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const bvec3& value) {
    if (glProgramUniform3iv)
        glProgramUniform3iv(_id, location, 1, glm::value_ptr(glm::ivec3(value)));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3iv(location, 1, glm::value_ptr(glm::ivec3(value)));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const bvec4& value) {
    if (glProgramUniform4iv)
        glProgramUniform4iv(_id, location, 1, glm::value_ptr(glm::ivec4(value)));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4iv(location, 1, glm::value_ptr(glm::ivec4(value)));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const bool* values, int count) {
    GLint* castValues = new GLint[count];
    for (int i = 0; i < count; ++i)
        castValues[i] = values[i];
    if (glProgramUniform1iv)
        glProgramUniform1iv(_id, location, count, castValues);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1iv(location, count, castValues);
        glUseProgram(oldProgram);
    }
    delete[] castValues;
}

void ProgramObject::setUniform(GLint location, bvec2* values, int count) {
    GLint* castValues = new GLint[2 * count];
    for (int i = 0; i < count; ++i) {
        castValues[2 * i] = values[i].x;
        castValues[2 * i + 1] = values[i].y;
    }
    if (glProgramUniform2iv)
        glProgramUniform2iv(_id, location, count, castValues);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2iv(location, count, castValues);
        glUseProgram(oldProgram);
    }
    delete[] castValues;
}

void ProgramObject::setUniform(GLint location, bvec3* values, int count) {
    GLint* castValues = new GLint[3 * count];
    for (int i = 0; i < count; ++i) {
        castValues[3 * i] = values[i].x;
        castValues[3 * i + 1] = values[i].y;
        castValues[3 * i + 2] = values[i].z;
    }
    if (glProgramUniform3iv)
        glProgramUniform3iv(_id, location, count, castValues);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3iv(location, count, castValues);
        glUseProgram(oldProgram);
    }
    delete[] castValues;
}

void ProgramObject::setUniform(GLint location, bvec4* values, int count /*= 1*/) {
    GLint* castValues = new GLint[4 * count];
    for (int i = 0; i < count; ++i) {
        castValues[4 * i] = values[i].x;
        castValues[4 * i + 1] = values[i].y;
        castValues[4 * i + 2] = values[i].z;
        castValues[4 * i + 3] = values[i].w;
    }
    if (glProgramUniform4iv)
        glProgramUniform4iv(_id, location, count, castValues);
    {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4iv(location, count, castValues);
        glUseProgram(oldProgram);
    }
    delete[] castValues;
}

void ProgramObject::setUniform(GLint location, GLuint value) {
    if (glProgramUniform1ui)
        glProgramUniform1ui(_id, location, value);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1ui(location, value);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, GLuint v1, GLuint v2) {
    if (glProgramUniform2ui)
        glProgramUniform2ui(_id, location, v1, v2);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2ui(location, v1, v2);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, GLuint v1, GLuint v2, GLuint v3) {
    if (glProgramUniform3ui)
        glProgramUniform3ui(_id, location, v1, v2, v3);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3ui(location, v1, v2, v3);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, GLuint v1, GLuint v2, GLuint v3, GLuint v4) {
    if (glProgramUniform4ui)
        glProgramUniform4ui(_id, location, v1, v2, v3, v4);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4ui(location, v1, v2, v3, v4);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const uvec2& value) {
    if (glProgramUniform2uiv)
        glProgramUniform2uiv(_id, location, 1, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2uiv(location, 1, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const uvec3& value) {
    if (glProgramUniform3uiv)
        glProgramUniform3uiv(_id, location, 1, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3uiv(location, 1, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const uvec4& value) {
    if (glProgramUniform4uiv)
        glProgramUniform4uiv(_id, location, 1, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4uiv(location, 1, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const GLuint* values, int count) {
    if (glProgramUniform1uiv)
        glProgramUniform1uiv(_id, location, count, values);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1uiv(location, count, values);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, uvec2* values, int count) {
    if (glProgramUniform2uiv)
        glProgramUniform2uiv(_id, location, count, reinterpret_cast<unsigned int*>(values));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2uiv(location, count, reinterpret_cast<unsigned int*>(values));
        glUseProgram(oldProgram);
    }
}   

void ProgramObject::setUniform(GLint location, uvec3* values, int count) {
    if (glProgramUniform3uiv)
        glProgramUniform3uiv(_id, location, count, reinterpret_cast<unsigned int*>(values));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3uiv(location, count, reinterpret_cast<unsigned int*>(values));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, uvec4* values, int count) {
    if (glProgramUniform4uiv)
        glProgramUniform4uiv(_id, location, count, reinterpret_cast<unsigned int*>(values));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4uiv(location, count, reinterpret_cast<unsigned int*>(values));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, GLint value) {
    if (glProgramUniform1i)
        glProgramUniform1i(_id, location, value);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1i(location, value);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, GLint v1, GLint v2) {
    if (glProgramUniform2i)
        glProgramUniform2i(_id, location, v1, v2);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2i(location, v1, v2);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, GLint v1, GLint v2, GLint v3) {
    if (glProgramUniform3i)
        glProgramUniform3i(_id, location, v1, v2, v3);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3i(location, v1, v2, v3);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, GLint v1, GLint v2, GLint v3, GLint v4) {
    if (glProgramUniform4i)
        glProgramUniform4i(_id, location, v1, v2, v3, v4);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4i(location, v1, v2, v3, v4);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const ivec2& value) {
    if (glProgramUniform2iv)
        glProgramUniform2iv(_id, location, 1, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2iv(location, 1, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const ivec3& value) {
    if (glProgramUniform3iv)
        glProgramUniform3iv(_id, location, 1, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3iv(location, 1, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const ivec4& value) {
    if (glProgramUniform4iv)
        glProgramUniform4iv(_id, location, 1, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4iv(location, 1, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const GLint* values, int count) {
    if (glProgramUniform1iv)
        glProgramUniform1iv(_id, location, count, values);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1iv(location, count, values);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, ivec2* values, int count) {
    if (glProgramUniform2iv)
        glProgramUniform2iv(_id, location, count, reinterpret_cast<int*>(values));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2iv(location, count, reinterpret_cast<int*>(values));
        glUseProgram(oldProgram);
    }
}   

void ProgramObject::setUniform(GLint location, ivec3* values, int count) {
    if (glProgramUniform3iv)
        glProgramUniform3iv(_id, location, count, reinterpret_cast<int*>(values));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3iv(location, count, reinterpret_cast<int*>(values));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, ivec4* values, int count) {
    if (glProgramUniform4iv)
        glProgramUniform4iv(_id, location, count, reinterpret_cast<int*>(values));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4iv(location, count, reinterpret_cast<int*>(values));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, GLfloat value) {
    if (glProgramUniform1f)
        glProgramUniform1f(_id, location, value);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1f(location, value);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, GLfloat v1, GLfloat v2) {
    if (glProgramUniform2f)
        glProgramUniform2f(_id, location, v1, v2);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2f(location, v1, v2);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, GLfloat v1, GLfloat v2, GLfloat v3) {
    if (glProgramUniform3f)
        glProgramUniform3f(_id, location, v1, v2, v3);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3f(location, v1, v2, v3);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4) {
    if (glProgramUniform4f)
        glProgramUniform4f(_id, location, v1, v2, v3, v4);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4f(location, v1, v2, v3, v4);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const vec2& value) {
    if (glProgramUniform2fv)
        glProgramUniform2fv(_id, location, 1, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2fv(location, 1, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const vec3& value) {
    if (glProgramUniform3fv)
        glProgramUniform3fv(_id, location, 1, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3fv(location, 1, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const vec4& value) {
    if (glProgramUniform4fv)
        glProgramUniform4fv(_id, location, 1, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4fv(location, 1, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const GLfloat* values, int count) {
    if (glProgramUniform1fv)
        glProgramUniform1fv(_id, location, count, values);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1fv(location, count, values);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, vec2* values, int count) {
    if (glProgramUniform2fv)
        glProgramUniform2fv(_id, location, count, reinterpret_cast<float*>(values));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2fv(location, count, reinterpret_cast<float*>(values));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, vec3* values, int count /*= 1*/) {
    if (glProgramUniform3fv)
        glProgramUniform3fv(_id, location, count, reinterpret_cast<float*>(values));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3fv(location, count, reinterpret_cast<float*>(values));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, vec4* values, int count /*= 1*/) {
    if (glProgramUniform4fv)
        glProgramUniform4fv(_id, location, count, reinterpret_cast<float*>(values));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4fv(location, count, reinterpret_cast<float*>(values));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, GLdouble value) {
    if (glProgramUniform1d)
        glProgramUniform1d(_id, location, value);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1d(location, value);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, GLdouble v1, GLdouble v2) {
    if (glProgramUniform2d)
        glProgramUniform2d(_id, location, v1, v2);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2d(location, v1, v2);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, GLdouble v1, GLdouble v2, GLdouble v3) {
    if (glProgramUniform3d)
        glProgramUniform3d(_id, location, v1, v2, v3);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3d(location, v1, v2, v3);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, GLdouble v1, GLdouble v2, GLdouble v3, GLdouble v4) {
    if (glProgramUniform4d)
        glProgramUniform4d(_id, location, v1, v2, v3, v4);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4d(location, v1, v2, v3, v4);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const dvec2& value) {
    if (glProgramUniform2dv)
        glProgramUniform2dv(_id, location, 1, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2dv(location, 1, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const dvec3& value) {
    if (glProgramUniform3dv)
        glProgramUniform3dv(_id, location, 1, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3dv(location, 1, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const dvec4& value) {
    if (glProgramUniform4dv)
        glProgramUniform4dv(_id, location, 1, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4dv(location, 1, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const GLdouble* values, int count) {
    if (glProgramUniform1dv)
        glProgramUniform1dv(_id, location, count, values);
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1dv(location, count, values);
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, dvec2* values, int count) {
    if (glProgramUniform2dv)
        glProgramUniform2dv(_id, location, count, reinterpret_cast<double*>(values));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2dv(location, count, reinterpret_cast<double*>(values));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, dvec3* values, int count) {
    if (glProgramUniform3dv)
        glProgramUniform3dv(_id, location, count, reinterpret_cast<double*>(values));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3dv(location, count, reinterpret_cast<double*>(values));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, dvec4* values, int count) {
    if (glProgramUniform4dv)
        glProgramUniform4dv(_id, location, count, reinterpret_cast<double*>(values));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4dv(location, count, reinterpret_cast<double*>(values));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const mat2x2& value, bool transpose) {
    if (glProgramUniformMatrix2fv)
        glProgramUniformMatrix2fv(_id, location, 1, transpose, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix2fv(location, 1, transpose, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const mat2x3& value, bool transpose) {
    if (glProgramUniformMatrix2x3fv)
        glProgramUniformMatrix2x3fv(_id, location, 1, transpose, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix2x3fv(location, 1, transpose, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const mat2x4& value, bool transpose) {
    if (glProgramUniformMatrix2x4fv)
        glProgramUniformMatrix2x4fv(_id, location, 1, transpose, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix2x4fv(location, 1, transpose, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const mat3x2& value, bool transpose) {
    if (glProgramUniformMatrix3x2fv)
        glProgramUniformMatrix3x2fv(_id, location, 1, transpose, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix3x2fv(location, 1, transpose, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const mat3x3& value, bool transpose) {
    if (glProgramUniformMatrix3fv)
        glProgramUniformMatrix3fv(_id, location, 1, transpose, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix3fv(location, 1, transpose, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const mat3x4& value, bool transpose) {
    if (glProgramUniformMatrix3x4fv)
        glProgramUniformMatrix3x4fv(_id, location, 1, transpose, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix3x4fv(location, 1, transpose, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const mat4x2& value, bool transpose) {
    if (glProgramUniformMatrix4x2fv)
        glProgramUniformMatrix4x2fv(_id, location, 1, transpose, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix4x2fv(location, 1, transpose, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const mat4x3& value, bool transpose) {
    if (glProgramUniformMatrix4x3fv)
        glProgramUniformMatrix4x3fv(_id, location, 1, transpose, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix4x3fv(location, 1, transpose, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const mat4x4& value, bool transpose) {
    if (glProgramUniformMatrix4fv)
        glProgramUniformMatrix4fv(_id, location, 1, transpose, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix4fv(location, 1, transpose, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const dmat2x2& value, bool transpose) {
    if (glProgramUniformMatrix2dv)
        glProgramUniformMatrix2dv(_id, location, 1, transpose, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix2dv(location, 1, transpose, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const dmat2x3& value, bool transpose) {
    if (glProgramUniformMatrix2x3dv)
        glProgramUniformMatrix2x3dv(_id, location, 1, transpose, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix2x3dv(location, 1, transpose, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const dmat2x4& value, bool transpose) {
    if (glProgramUniformMatrix2x4dv)
        glProgramUniformMatrix2x4dv(_id, location, 1, transpose, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix2x4dv(location, 1, transpose, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const dmat3x2& value, bool transpose) {
    if (glProgramUniformMatrix3x2dv)
        glProgramUniformMatrix3x2dv(_id, location, 1, transpose, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix3x2dv(location, 1, transpose, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const dmat3x3& value, bool transpose) {
    if (glProgramUniformMatrix3dv)
        glProgramUniformMatrix3dv(_id, location, 1, transpose, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix3dv(location, 1, transpose, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const dmat3x4& value, bool transpose) {
    if (glProgramUniformMatrix3x4dv)
        glProgramUniformMatrix3x4dv(_id, location, 1, transpose, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix3x4dv(location, 1, transpose, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const dmat4x2& value, bool transpose) {
    if (glProgramUniformMatrix4x2dv)
        glProgramUniformMatrix4x2dv(_id, location, 1, transpose, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix4x2dv(location, 1, transpose, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const dmat4x3& value, bool transpose) {
    if (glProgramUniformMatrix4x3dv)
        glProgramUniformMatrix4x3dv(_id, location, 1, transpose, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix4x3dv(location, 1, transpose, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

void ProgramObject::setUniform(GLint location, const dmat4x4& value, bool transpose) {
    if (glProgramUniformMatrix4dv)
        glProgramUniformMatrix4dv(_id, location, 1, transpose, value_ptr(value));
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix4dv(location, 1, transpose, value_ptr(value));
        glUseProgram(oldProgram);
    }
}

GLint ProgramObject::attributeLocation(const string& name) const {
    const GLint location = glGetAttribLocation(_id, name.c_str());
    if (!_ignoreAttributeLocationError && location == -1)
        LWARNING_SAFE("Failed to locate attribute location for: " + name);
    return location;
}

void ProgramObject::bindAttributeLocation(const string& name, GLint index) {
    glBindAttribLocation(_id, index, name.c_str());
}

bool ProgramObject::ignoreAttributeLocationError() const {
    return _ignoreAttributeLocationError;
}

void ProgramObject::setIgnoreAttributeLocationError(bool ignoreError) {
    _ignoreAttributeLocationError = ignoreError;
}

bool ProgramObject::setAttribute(const string& name, bool value) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const string& name, bool v1, bool v2) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, v1, v2);
    return true;
}

bool ProgramObject::setAttribute(const string& name, bool v1, bool v2, bool v3) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, v1, v2, v3);
    return true;
}

bool ProgramObject::setAttribute(const string& name, bool v1, bool v2, bool v3, bool v4) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const bvec2& value) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const bvec3& value) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const bvec4& value) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const string& name, GLint value) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const string& name, GLint v1, GLint v2) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, v1, v2);
    return true;
}

bool ProgramObject::setAttribute(const string& name, GLint v1, GLint v2, GLint v3) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, v1, v2, v3);
    return true;
}

bool ProgramObject::setAttribute(const string& name, GLint v1, GLint v2, GLint v3, GLint v4) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const ivec2& value) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const ivec3& value) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const ivec4& value) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const string& name, GLfloat value) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const string& name, GLfloat v1, GLfloat v2) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, v1, v2);
    return true;
}

bool ProgramObject::setAttribute(const string& name, GLfloat v1, GLfloat v2, GLfloat v3) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, v1, v2, v3);
    return true;
}

bool ProgramObject::setAttribute(const string& name, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const vec2& value) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const vec3& value) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const vec4& value) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const string& name, GLdouble value) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const string& name, GLdouble v1, GLdouble v2) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, v1, v2);
    return true;
}

bool ProgramObject::setAttribute(const string& name, GLdouble v1, GLdouble v2, GLdouble v3) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, v1, v2, v3);
    return true;
}

bool ProgramObject::setAttribute(const string& name, GLdouble v1, GLdouble v2, GLdouble v3, GLdouble v4) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const dvec2& value) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const dvec3& value) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const dvec4& value) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const mat2x2& value, bool transpose) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const mat2x3& value, bool transpose) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const mat2x4& value, bool transpose) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const mat3x2& value, bool transpose) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const mat3x3& value, bool transpose) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const mat3x4& value, bool transpose) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const mat4x2& value, bool transpose) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const mat4x3& value, bool transpose) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const mat4x4& value, bool transpose) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const dmat2x2& value, bool transpose) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const dmat2x3& value, bool transpose) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const dmat2x4& value, bool transpose) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const dmat3x2& value, bool transpose) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const dmat3x3& value, bool transpose) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const dmat3x4& value, bool transpose) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const dmat4x2& value, bool transpose) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const dmat4x3& value, bool transpose) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const string& name, const dmat4x4& value, bool transpose) {
    const GLint location = attributeLocation(name);
    if (location == -1)
        return false;
    setAttribute(location, value, transpose);
    return true;
}

void ProgramObject::setAttribute(GLint location, bool value) {
    glVertexAttribI1i(location, value);
}

void ProgramObject::setAttribute(GLint location, bool v1, bool v2) {
    glVertexAttribI2i(location, v1, v2);
}

void ProgramObject::setAttribute(GLint location, bool v1, bool v2, bool v3) {
    glVertexAttribI3i(location, v1, v2, v3);
}

void ProgramObject::setAttribute(GLint location, bool v1, bool v2, bool v3, bool v4) {
    glVertexAttribI4i(location, v1, v2, v3, v4);
}

void ProgramObject::setAttribute(GLint location, const bvec2& value) {
    glVertexAttribI2iv(location, value_ptr(glm::ivec2(value)));
}

void ProgramObject::setAttribute(GLint location, const bvec3& value) {
    glVertexAttribI3iv(location, value_ptr(glm::ivec3(value)));
}

void ProgramObject::setAttribute(GLint location, const bvec4& value) {
    glVertexAttribI4iv(location, value_ptr(glm::ivec4(value)));
}

void ProgramObject::setAttribute(GLint location, GLint value) {
    glVertexAttribI1i(location, value);
}

void ProgramObject::setAttribute(GLint location, GLint v1, GLint v2) {
    glVertexAttribI2i(location, v1, v2);
}

void ProgramObject::setAttribute(GLint location, GLint v1, GLint v2, GLint v3) {
    glVertexAttribI3i(location, v1, v2, v3);
}

void ProgramObject::setAttribute(GLint location, GLint v1, GLint v2, GLint v3, GLint v4) {
     glVertexAttribI4i(location, v1, v2, v3, v4);
}

void ProgramObject::setAttribute(GLint location, const ivec2& value) {
    glVertexAttribI2iv(location, value_ptr(value));
}

void ProgramObject::setAttribute(GLint location, const ivec3& value) {
    glVertexAttribI3iv(location, value_ptr(value));
}

void ProgramObject::setAttribute(GLint location, const ivec4& value) {
    glVertexAttribI4iv(location, value_ptr(value));
}

void ProgramObject::setAttribute(GLint location, GLfloat value) {
    glVertexAttrib1f(location, value);
}

void ProgramObject::setAttribute(GLint location, GLfloat v1, GLfloat v2) {
    glVertexAttrib2f(location, v1, v2);
}

void ProgramObject::setAttribute(GLint location, GLfloat v1, GLfloat v2, GLfloat v3) {
    glVertexAttrib3f(location, v1, v2, v3);
}

void ProgramObject::setAttribute(GLint location, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4) {
    glVertexAttrib4f(location, v1, v2, v3, v4);
}

void ProgramObject::setAttribute(GLint location, const vec2& value) {
    glVertexAttrib2fv(location, value_ptr(value));
}

void ProgramObject::setAttribute(GLint location, const vec3& value) {
    glVertexAttrib3fv(location, value_ptr(value));
}

void ProgramObject::setAttribute(GLint location, const vec4& value) {
    glVertexAttrib4fv(location, value_ptr(value));
}

void ProgramObject::setAttribute(GLint location, GLdouble value) {
    glVertexAttribL1d(location, value);
}

void ProgramObject::setAttribute(GLint location, GLdouble v1, GLdouble v2) {
    glVertexAttribL2d(location, v1, v2);
}

void ProgramObject::setAttribute(GLint location, GLdouble v1, GLdouble v2, GLdouble v3) {
    glVertexAttribL3d(location, v1, v2, v3);
}

void ProgramObject::setAttribute(GLint location, GLdouble v1, GLdouble v2, GLdouble v3, GLdouble v4) {
    glVertexAttribL4d(location, v1, v2, v3, v4);
}

void ProgramObject::setAttribute(GLint location, const dvec2& value) {
    glVertexAttribL2dv(location, value_ptr(value));
}

void ProgramObject::setAttribute(GLint location, const dvec3& value) {
    glVertexAttribL3dv(location, value_ptr(value));
}

void ProgramObject::setAttribute(GLint location, const dvec4& value) {
    glVertexAttribL4dv(location, value_ptr(value));
}

void ProgramObject::setAttribute(GLint location, const mat2x2& value, bool transpose) {
    if (transpose)
        setAttribute(location, glm::transpose(value));
    else {
        glVertexAttrib2fv(location, value_ptr(value[0]));
        glVertexAttrib2fv(location + 1, value_ptr(value[1]));
    }
}

void ProgramObject::setAttribute(GLint location, const mat2x3& value, bool transpose) {
    if (transpose)
        setAttribute(location, glm::transpose(value));
    else {
        glVertexAttrib3fv(location, value_ptr(value[0]));
        glVertexAttrib3fv(location + 1, value_ptr(value[1]));
    }
}

void ProgramObject::setAttribute(GLint location, const mat2x4& value, bool transpose) {
    if (transpose)
        setAttribute(location, glm::transpose(value));
    else {
        glVertexAttrib4fv(location, value_ptr(value[0]));
        glVertexAttrib4fv(location + 1, value_ptr(value[1]));
    }
}

void ProgramObject::setAttribute(GLint location, const mat3x2& value, bool transpose) {
    if (transpose)
        setAttribute(location, glm::transpose(value));
    else {
        glVertexAttrib2fv(location, value_ptr(value[0]));
        glVertexAttrib2fv(location + 1, value_ptr(value[1]));
        glVertexAttrib2fv(location + 2, value_ptr(value[2]));
    }
}

void ProgramObject::setAttribute(GLint location, const mat3x3& value, bool transpose) {
    if (transpose)
        setAttribute(location, glm::transpose(value));
    else {
        glVertexAttrib3fv(location, value_ptr(value[0]));
        glVertexAttrib3fv(location + 1, value_ptr(value[1]));
        glVertexAttrib3fv(location + 2, value_ptr(value[2]));
    }
}

void ProgramObject::setAttribute(GLint location, const mat3x4& value, bool transpose) {
    if (transpose)
        setAttribute(location, glm::transpose(value));
    else {
        glVertexAttrib4fv(location, value_ptr(value[0]));
        glVertexAttrib4fv(location + 1, value_ptr(value[1]));
        glVertexAttrib4fv(location + 2, value_ptr(value[2]));
    }
}

void ProgramObject::setAttribute(GLint location, const mat4x2& value, bool transpose) {
    if (transpose)
        setAttribute(location, glm::transpose(value));
    else {
        glVertexAttrib2fv(location, value_ptr(value[0]));
        glVertexAttrib2fv(location + 1, value_ptr(value[1]));
        glVertexAttrib2fv(location + 2, value_ptr(value[2]));
        glVertexAttrib2fv(location + 3, value_ptr(value[3]));
    }
}

void ProgramObject::setAttribute(GLint location, const mat4x3& value, bool transpose) {
    if (transpose)
        setAttribute(location, glm::transpose(value));
    else {
        glVertexAttrib3fv(location, value_ptr(value[0]));
        glVertexAttrib3fv(location + 1, value_ptr(value[1]));
        glVertexAttrib3fv(location + 2, value_ptr(value[2]));
        glVertexAttrib3fv(location + 3, value_ptr(value[3]));
    }
}

void ProgramObject::setAttribute(GLint location, const mat4x4& value, bool transpose) {
    if (transpose)
        setAttribute(location, glm::transpose(value));
    else {
        glVertexAttrib4fv(location, value_ptr(value[0]));
        glVertexAttrib4fv(location + 1, value_ptr(value[1]));
        glVertexAttrib4fv(location + 2, value_ptr(value[2]));
        glVertexAttrib4fv(location + 3, value_ptr(value[3]));
    }
}

void ProgramObject::setAttribute(GLint location, const dmat2x2& value, bool transpose) {
    if (transpose)
        setAttribute(location, glm::transpose(value));
    else {
        glVertexAttribL2dv(location, value_ptr(value[0]));
        glVertexAttribL2dv(location + 1, value_ptr(value[1]));
    }
}

void ProgramObject::setAttribute(GLint location, const dmat2x3& value, bool transpose) {
    if (transpose)
        setAttribute(location, glm::transpose(value));
    else {
        glVertexAttribL3dv(location, value_ptr(value[0]));
        glVertexAttribL3dv(location + 1, value_ptr(value[1]));
    }
}

void ProgramObject::setAttribute(GLint location, const dmat2x4& value, bool transpose) {
    if (transpose)
        setAttribute(location, glm::transpose(value));
    else {
        glVertexAttribL4dv(location, value_ptr(value[0]));
        glVertexAttribL4dv(location + 1, value_ptr(value[1]));
    }
}

void ProgramObject::setAttribute(GLint location, const dmat3x2& value, bool transpose) {
    if (transpose)
        setAttribute(location, glm::transpose(value));
    else {
        glVertexAttribL2dv(location, value_ptr(value[0]));
        glVertexAttribL2dv(location + 1, value_ptr(value[1]));
        glVertexAttribL2dv(location + 2, value_ptr(value[2]));
    }
}

void ProgramObject::setAttribute(GLint location, const dmat3x3& value, bool transpose) {
    if (transpose)
        setAttribute(location, glm::transpose(value));
    else {
        glVertexAttribL3dv(location, value_ptr(value[0]));
        glVertexAttribL3dv(location + 1, value_ptr(value[1]));
        glVertexAttribL3dv(location + 2, value_ptr(value[2]));
    }
}

void ProgramObject::setAttribute(GLint location, const dmat3x4& value, bool transpose) {
    if (transpose)
        setAttribute(location, glm::transpose(value));
    else {
        glVertexAttribL4dv(location, value_ptr(value[0]));
        glVertexAttribL4dv(location + 1, value_ptr(value[1]));
        glVertexAttribL4dv(location + 2, value_ptr(value[2]));
    }
}

void ProgramObject::setAttribute(GLint location, const dmat4x2& value, bool transpose) {
    if (transpose)
        setAttribute(location, glm::transpose(value));
    else {
        glVertexAttribL2dv(location, value_ptr(value[0]));
        glVertexAttribL2dv(location + 1, value_ptr(value[1]));
        glVertexAttribL2dv(location + 2, value_ptr(value[2]));
        glVertexAttribL2dv(location + 3, value_ptr(value[3]));
    }
}

void ProgramObject::setAttribute(GLint location, const dmat4x3& value, bool transpose) {
    if (transpose)
        setAttribute(location, glm::transpose(value));
    else {
        glVertexAttribL3dv(location, value_ptr(value[0]));
        glVertexAttribL3dv(location + 1, value_ptr(value[1]));
        glVertexAttribL3dv(location + 2, value_ptr(value[2]));
        glVertexAttribL3dv(location + 3, value_ptr(value[3]));
    }
}

void ProgramObject::setAttribute(GLint location, const dmat4x4& value, bool transpose) {
    if (transpose)
        setAttribute(location, glm::transpose(value));
    else {
        glVertexAttribL4dv(location, value_ptr(value[0]));
        glVertexAttribL4dv(location + 1, value_ptr(value[1]));
        glVertexAttribL4dv(location + 2, value_ptr(value[2]));
        glVertexAttribL4dv(location + 3, value_ptr(value[3]));
    }
}

void ProgramObject::setIgnoreSubroutineLocationError(bool ignoreError) {
    _ignoreSubroutineLocationError = ignoreError;
}

bool ProgramObject::ignoreSubroutineLocationError() const {
    return _ignoreSubroutineLocationError;
}

void ProgramObject::setIgnoreSubroutineUniformLocationError(bool ignoreError) {
    _ignoreSubroutineUniformLocationError = ignoreError;
}

bool ProgramObject::ignoreSubroutineUniformLocationError() const {
    return _ignoreSubroutineUniformLocationError;
}

GLuint ProgramObject::subroutineIndex(ShaderObject::ShaderType shaderType, const std::string& name) {
    const GLuint index = glGetSubroutineIndex(_id, shaderType, name.c_str());
    if (!_ignoreSubroutineLocationError && index == GL_INVALID_INDEX)
        LWARNING_SAFE("Failed to locate subroutine index for: " + name);
    return index;
}

GLint ProgramObject::subroutineUniformLocation(ShaderObject::ShaderType shaderType, const std::string& name) {
    const GLint location = glGetSubroutineUniformLocation(_id, shaderType, name.c_str());
    if (!_ignoreSubroutineUniformLocationError && location == -1)
        LWARNING_SAFE("Failed to locate subroutine uniform location for: " + name);
    return location;
}

vector<string> ProgramObject::activeSubroutineUniformNames(ShaderObject::ShaderType shaderType) {
#ifdef GHL_DEBUG
    GLint maximumUniformNameLength;
    glGetProgramStageiv(_id, shaderType, GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH, &maximumUniformNameLength);
    if (maximumUniformNameLength > 1024)
        LWARNING_SAFE("String buffer for Subroutine Uniform name (1024) is insufficient");
#endif
    int countActiveSubroutineUniforms;
    char buffer[1024];
    glGetProgramStageiv(_id, shaderType, GL_ACTIVE_SUBROUTINE_UNIFORMS, &countActiveSubroutineUniforms);
    vector<string> result(countActiveSubroutineUniforms);
    for (int i = 0; i < countActiveSubroutineUniforms; ++i) {
        glGetActiveSubroutineUniformName(_id, shaderType, i, 1024, NULL, buffer);
        result[i] = string(buffer);
    }

    return result;
}

vector<string> ProgramObject::compatibleSubroutineNames(ShaderObject::ShaderType shaderType, GLuint subroutineUniformLocation) {
#ifdef GHL_DEBUG
    GLint maximumUniformNameLength;
    glGetProgramStageiv(_id, shaderType, GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH, &maximumUniformNameLength);
    if (maximumUniformNameLength > 1024)
        LWARNING_SAFE("String buffer for Subroutine Uniform name (1024) is insufficient");
#endif

    GLint numCompatibleSubroutines;
    glGetActiveSubroutineUniformiv(_id, shaderType, subroutineUniformLocation, GL_NUM_COMPATIBLE_SUBROUTINES, &numCompatibleSubroutines);
    if (numCompatibleSubroutines == 0)
        return vector<string>();
    
    vector<string> result(numCompatibleSubroutines);
    GLint* indices = new GLint[numCompatibleSubroutines];
    char buffer[1024];
    glGetActiveSubroutineUniformiv(_id, shaderType, subroutineUniformLocation, GL_COMPATIBLE_SUBROUTINES, indices);
    for (int i = 0; i < numCompatibleSubroutines; ++i) {
       glGetActiveSubroutineName(_id, shaderType, indices[i], 1024, NULL, buffer);
       result[i] = string(buffer);
    }
    delete[] indices;
    return result;
}

vector<string> ProgramObject::compatibleSubroutineNames(ShaderObject::ShaderType shaderType, const string& subroutineUniformName) {
    const GLint index = subroutineUniformLocation(shaderType, subroutineUniformName);
    if (index == -1)
        return vector<string>();
    else
        return compatibleSubroutineNames(shaderType, index);
}

bool ProgramObject::setUniformSubroutines(ShaderObject::ShaderType shaderType, const vector<GLuint>& values) {
#ifdef GHL_DEBUG
    if (values.size() == 0) {
        LWARNING_SAFE("No values passed");
        return false;
    }

    int countActiveSubroutineUniforms;
    glGetProgramStageiv(_id, shaderType, GL_ACTIVE_SUBROUTINE_UNIFORMS, &countActiveSubroutineUniforms);
    if (static_cast<size_t>(countActiveSubroutineUniforms) != values.size()) {
        LWARNING_SAFE("Number of active subroutine uniform (" << countActiveSubroutineUniforms <<
            ") is different from passed uniform subroutine indices (" << values.size() << ")");
        return false;
    }
#endif
    glUniformSubroutinesuiv(shaderType, static_cast<GLsizei>(values.size()), &values[0]);
    return true;
}

bool ProgramObject::setUniformSubroutines(ShaderObject::ShaderType shaderType, const map<string, string>& values) {
#ifdef GHL_DEBUG
    if (values.size() == 0) {
        LWARNING_SAFE("No values passed");
        return false;
    }

    int countActiveSubroutineUniforms;
    glGetProgramStageiv(_id, shaderType, GL_ACTIVE_SUBROUTINE_UNIFORMS, &countActiveSubroutineUniforms);
    if (static_cast<size_t>(countActiveSubroutineUniforms) != values.size()) {
        LWARNING_SAFE("Number of active subroutine uniform (" << countActiveSubroutineUniforms <<
            ") is different from passed uniform subroutine indices (" << values.size() << ")");
        return false;
    }
#endif

    vector<GLuint> uniformIndices(values.size());
    const vector<string>& uniformSubroutines = activeSubroutineUniformNames(shaderType);
    for (size_t i = 0; i < uniformSubroutines.size(); ++i) {
        const string& uniformSubroutine = uniformSubroutines[i];
        map<string,string>::const_iterator subroutine = values.find(uniformSubroutine);
#ifdef GHL_DEBUG
        if (subroutine == values.end()) {
            LWARNING_SAFE("Uniform subroutine name '" + uniformSubroutine +
                "' was not present in map");
            return false;
        }
#endif
        const string& nameSubroutine = subroutine->second;
        GLuint idxSubroutine = subroutineIndex(shaderType, nameSubroutine);
#ifdef GHL_DEBUG
        if (idxSubroutine == GL_INVALID_INDEX) {
            LWARNING_SAFE("Subroutine name '" + nameSubroutine +
                "' was not found in shader object.");
            return false;
        }
#endif
        uniformIndices[i] = idxSubroutine;
    }
    glUniformSubroutinesuiv(
        shaderType,
        static_cast<GLsizei>(uniformIndices.size()),
        &uniformIndices[0]);
    return true;
}

void ProgramObject::bindFragDataLocation(const std::string& name, GLuint colorNumber) {
#ifdef GHL_DEBUG
    GLint maxDrawBuffers;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
    if (colorNumber >= maxDrawBuffers) {
        LWARNING_SAFE("ColorNumber '" + colorNumber +
            "' is bigger than the maximum of simultaneous outputs '" + maxDrawBuffers "'");
        return;
    }
#endif

    glBindFragDataLocation(_id, colorNumber, name.c_str());
}


} // namespace opengl
} // namespace ghoul
