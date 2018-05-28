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

#include <ghoul/opengl/programobject.h>

#include <ghoul/fmt.h>
#include <ghoul/filesystem/filesystem.h>
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

namespace ghoul::opengl {

ProgramObject::ProgramObjectError::ProgramObjectError(std::string msg)
    : RuntimeError(std::move(msg), "ProgramObject")
{}

ProgramObject::ProgramObjectLinkingError::ProgramObjectLinkingError(std::string msg,
                                                                         std::string name)
    : ProgramObjectError(
        name.empty() ?
        "Error linking program object: " + msg :
        "Error linking program object [" + name + "]: " + msg
      )
    , linkerError(std::move(msg))
    , programName(std::move(name))
{}

ProgramObject::ProgramObject()
    : _id(0)
    , _programName("")
    , _loggerCat("ProgramObject")
{
    _id = glCreateProgram();
    if (_id == 0) {
        throw ProgramObjectError("glCreateProgram returned 0");
    }
}

ProgramObject::ProgramObject(std::string name)
    : _id(0)
    , _programName(std::move(name))
    , _loggerCat("ProgramObject('" + _programName + "')")
{
    _id = glCreateProgram();
    if (_id == 0) {
        throw ProgramObjectError("glCreateProgram returned 0");
    }
    if (glbinding::Binding::ObjectLabel.isResolved()) {
        glObjectLabel(
            GL_PROGRAM,
            _id,
            static_cast<GLsizei>(_programName.length() + 1),
            _programName.c_str()
        );
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
    if (_id == 0) {
        throw ProgramObjectError("glCreateProgram returned 0");
    }

    if (glbinding::Binding::ObjectLabel.isResolved()) {
        glObjectLabel(
            GL_PROGRAM,
            _id,
            static_cast<GLsizei>(_programName.length() + 1),
            _programName.c_str()
        );
    }

    for (const std::shared_ptr<ShaderObject>& obj : cpy._shaderObjects) {
        std::shared_ptr<ShaderObject> shaderCopy = std::make_shared<ShaderObject>(*obj);
        glAttachShader(_id, *shaderCopy);
        _shaderObjects.push_back(shaderCopy);
    }
}

ProgramObject::~ProgramObject() {
    glDeleteProgram(_id);
}

ProgramObject::operator GLuint() const {
    return _id;
}

ProgramObject& ProgramObject::operator=(const ProgramObject& rhs) {
    if (this != &rhs) {
        _programName = rhs._programName;

        if (glbinding::Binding::ObjectLabel.isResolved()) {
            glObjectLabel(
                GL_PROGRAM,
                _id,
                static_cast<GLsizei>(rhs._programName.length() + 1),
                rhs._programName.c_str()
            );
        }
        _loggerCat = rhs._loggerCat;
        _ignoreUniformLocationError = rhs._ignoreUniformLocationError;
        _ignoreAttributeLocationError = rhs._ignoreAttributeLocationError;
        _ignoreSubroutineLocationError = rhs._ignoreSubroutineLocationError;
        _ignoreSubroutineUniformLocationError = rhs._ignoreSubroutineUniformLocationError;
        _programIsDirty = rhs._programIsDirty;

        glDeleteProgram(_id);

        _id = glCreateProgram();
        if (_id == 0) {
            throw ProgramObjectError("glCreateProgram returned 0");
        }
        if (glbinding::Binding::ObjectLabel.isResolved()) {
            glObjectLabel(
                GL_PROGRAM,
                _id,
                static_cast<GLsizei>(_programName.length() + 1),
                _programName.c_str()
            );
        }

        for (const std::shared_ptr<ShaderObject>& obj : rhs._shaderObjects) {
            std::shared_ptr<ShaderObject> shaderCopy =
                std::make_shared<ShaderObject>(*obj);
            glAttachShader(_id, *shaderCopy);
            _shaderObjects.push_back(shaderCopy);
        }
    }
    return *this;
}

ProgramObject& ProgramObject::operator=(ProgramObject&& rhs) {
    if (this != &rhs) {
        glDeleteProgram(_id);
        _id = rhs._id;
        rhs._id = 0;

        _programName = std::move(rhs._programName);
        if (glbinding::Binding::ObjectLabel.isResolved()) {
            glObjectLabel(
                GL_PROGRAM,
                _id,
                static_cast<GLsizei>(_programName.length() + 1),
                _programName.c_str()
            );
        }
        _loggerCat = std::move(rhs._loggerCat);
        _ignoreUniformLocationError = rhs._ignoreUniformLocationError;
        _ignoreAttributeLocationError = rhs._ignoreAttributeLocationError;
        _ignoreSubroutineLocationError = rhs._ignoreSubroutineLocationError;
        _ignoreSubroutineUniformLocationError = rhs._ignoreSubroutineUniformLocationError;
        _programIsDirty = std::move(rhs._programIsDirty);

        _shaderObjects.clear();
        _shaderObjects = std::move(rhs._shaderObjects);
    }
    return *this;
}

void ProgramObject::setName(string name) {
    _programName = std::move(name);
    _loggerCat = "ProgramObject['" + _programName + "']";
    if (glbinding::Binding::ObjectLabel.isResolved()) {
        glObjectLabel(
            GL_PROGRAM,
            _id,
            static_cast<GLsizei>(_programName.length() + 1),
            _programName.c_str()
        );
    }
}

const string& ProgramObject::name() const{
    return _programName;
}

void ProgramObject::setDictionary(const Dictionary& dictionary) {
    for (const std::shared_ptr<ShaderObject>& shaderObject : _shaderObjects) {
        shaderObject->setDictionary(dictionary);
    }
}

Dictionary ProgramObject::dictionary() {
    if (_shaderObjects.empty()) {
        throw ProgramObjectError("No shader object attached");
    }
    return _shaderObjects[0]->dictionary();
}

void ProgramObject::setProgramObjectCallback(ProgramObjectCallback changeCallback) {
    ShaderObject::ShaderObjectCallback c = [this, changeCallback](){
        _programIsDirty = true;
        changeCallback(this);
    };
    for (const std::shared_ptr<ShaderObject>& shaderObject : _shaderObjects) {
        shaderObject->setShaderObjectCallback(c);
    }
}

void ProgramObject::attachObject(std::shared_ptr<ShaderObject> shaderObject) {
    ghoul_assert(shaderObject, "ShaderObject must not be nullptr");
    auto it = std::find(_shaderObjects.begin(), _shaderObjects.end(), shaderObject);
    ghoul_assert(it == _shaderObjects.end(), "ShaderObject was already registered");

    shaderObject->setShaderObjectCallback([this]() { _programIsDirty = true; });

    glAttachShader(_id, *shaderObject);
    _shaderObjects.push_back(shaderObject);
}

void ProgramObject::detachObject(std::shared_ptr<ShaderObject> shaderObject) {
    ghoul_assert(shaderObject, "ShaderObject must not be nullptr");
    auto it = std::find(_shaderObjects.begin(), _shaderObjects.end(), shaderObject);
    ghoul_assert(it != _shaderObjects.end(), "ShaderObject must have been registered");

    glDetachShader(_id, *shaderObject);
    _shaderObjects.erase(it);
}

void ProgramObject::compileShaderObjects() {
    for (const std::shared_ptr<ShaderObject>& obj : _shaderObjects) {
        obj->compile();
    }
}

void ProgramObject::linkProgramObject() {
    glLinkProgram(_id);

    GLint linkStatus;
    glGetProgramiv(_id, GL_LINK_STATUS, &linkStatus);
    if (static_cast<GLboolean>(linkStatus) == GL_FALSE) {
        GLint logLength;
        glGetProgramiv(_id, GL_INFO_LOG_LENGTH, &logLength);

        if (logLength == 0) {
            throw ProgramObjectLinkingError("Unknown error", name());
        }

        std::vector<GLchar> rawLog(logLength);
        glGetProgramInfoLog(_id, logLength, nullptr, rawLog.data());
        string log(rawLog.data());
        throw ProgramObjectLinkingError(log, name());
    }
    _programIsDirty = false;
}

void ProgramObject::rebuildFromFile() {
    // The copy constructor of ShaderObject (called by the copy constructor of
    // ProgramObject) will take care of the reloading from file
    ProgramObject p(*this);

    p.compileShaderObjects();
    p.linkProgramObject();

    LINFO("Successfully rebuilt ProgramObject");

    *this = std::move(p);
}

bool ProgramObject::isDirty() const {
    return _programIsDirty;
}

void ProgramObject::activate() {
    glUseProgram(_id);
}

void ProgramObject::deactivate() {
    glUseProgram(0);
}

std::unique_ptr<ProgramObject> ProgramObject::Build(const std::string& name,
                                                    const std::string& vertexShaderPath,
                                                    const std::string& fragmentShaderPath,
                                                    Dictionary dictionary)
{
    ghoul_assert(!vertexShaderPath.empty(), "VertexShaderPath must not be empty");
    ghoul_assert(
        FileSys.fileExists(vertexShaderPath),
        "VertexShaderPath file must exist"
    );
    ghoul_assert(!fragmentShaderPath.empty(), "FragmentShaderPath must not be empty");
    ghoul_assert(
        FileSys.fileExists(fragmentShaderPath),
        "FragmentShaderPath file must exist"
    );

    std::unique_ptr<ProgramObject> program = std::make_unique<ProgramObject>(name);
    program->attachObject(std::make_unique<ShaderObject>(
        ShaderObject::ShaderType::Vertex,
        vertexShaderPath,
        name + " Vertex",
        dictionary
    ));
    program->attachObject(std::make_unique<ShaderObject>(
        ShaderObject::ShaderType::Fragment,
        fragmentShaderPath,
        name + " Fragment",
        dictionary
    ));

    program->compileShaderObjects();
    program->linkProgramObject();
    return program;
}

std::unique_ptr<ProgramObject> ProgramObject::Build(const std::string& name,
                                                    const std::string& vertexShaderPath,
                                                    const std::string& fragmentShaderPath,
                                                    const std::string& geometryShaderPath,
                                                    Dictionary dictionary)
{
    ghoul_assert(!vertexShaderPath.empty(), "VertexShaderPath must not be empty");
    ghoul_assert(
        FileSys.fileExists(vertexShaderPath),
        "VertexShaderPath file must exist"
    );
    ghoul_assert(!fragmentShaderPath.empty(), "FragmentShaderPath must not be empty");
    ghoul_assert(
        FileSys.fileExists(fragmentShaderPath),
        "FragmentShaderPath file must exist"
    );
    ghoul_assert(!geometryShaderPath.empty(), "GeometryShaderPath must not be empty");
    ghoul_assert(
        FileSys.fileExists(geometryShaderPath),
        "GeometryShaderPath file must exist"
    );

    std::unique_ptr<ProgramObject> program = std::make_unique<ProgramObject>(name);
    program->attachObject(std::make_unique<ShaderObject>(
        ShaderObject::ShaderType::Vertex,
        vertexShaderPath,
        name + " Vertex",
        dictionary
    ));
    program->attachObject(std::make_unique<ShaderObject>(
        ShaderObject::ShaderType::Geometry,
        geometryShaderPath,
        name + " Geometry",
        dictionary
    ));
    program->attachObject(std::make_unique<ShaderObject>(
        ShaderObject::ShaderType::Fragment,
        fragmentShaderPath,
        name + " Fragment",
        dictionary
    ));

    program->compileShaderObjects();
    program->linkProgramObject();
    return program;
}

std::unique_ptr<ProgramObject> ProgramObject::Build(const std::string& name,
                                                    const std::string& vertexShaderPath,
                                                    const std::string& fragmentShaderPath,
                                                    const std::string& geometryShaderPath,
                                      const std::string& tessellationEvaluationShaderPath,
                                         const std::string& tessellationControlShaderPath,
                                                    Dictionary dictionary)
{
    ghoul_assert(!vertexShaderPath.empty(), "VertexShaderPath must not be empty");
    ghoul_assert(
        FileSys.fileExists(vertexShaderPath),
        "VertexShaderPath file must exist"
    );
    ghoul_assert(!fragmentShaderPath.empty(), "FragmentShaderPath must not be empty");
    ghoul_assert(
        FileSys.fileExists(fragmentShaderPath),
        "FragmentShaderPath file must exist"
    );
    ghoul_assert(!geometryShaderPath.empty(), "GeometryShaderPath must not be empty");
    ghoul_assert(
        FileSys.fileExists(geometryShaderPath),
        "GeometryShaderPath file must exist"
    );
    ghoul_assert(
        !tessellationEvaluationShaderPath.empty(),
        "Tessellation evaluation shader must not be empty"
    );
    ghoul_assert(
        FileSys.fileExists(tessellationEvaluationShaderPath),
        "Tessellation evaluation shader file must exist"
    );
    ghoul_assert(
        !tessellationControlShaderPath.empty(),
        "Tessellation control shader must not be empty"
    );
    ghoul_assert(
        FileSys.fileExists(tessellationControlShaderPath),
        "Tessellation control shader file must exist"
    );

    std::unique_ptr<ProgramObject> program = std::make_unique<ProgramObject>(name);
    program->attachObject(std::make_unique<ShaderObject>(
        ShaderObject::ShaderType::Vertex,
        vertexShaderPath,
        name + " Vertex",
        dictionary
    ));
    program->attachObject(std::make_unique<ShaderObject>(
        ShaderObject::ShaderType::Geometry,
        geometryShaderPath,
        name + " Geometry",
        dictionary
    ));
    program->attachObject(std::make_unique<ShaderObject>(
        ShaderObject::ShaderType::TesselationEvaluation,
        tessellationEvaluationShaderPath,
        name + " Tessellation Evaluation",
        dictionary
    ));
    program->attachObject(std::make_unique<ShaderObject>(
        ShaderObject::ShaderType::TesselationControl,
        tessellationControlShaderPath,
        name + " Tessellation Control",
        dictionary
    ));
    program->attachObject(std::make_unique<ShaderObject>(
        ShaderObject::ShaderType::Fragment,
        fragmentShaderPath,
        name + " Fragment",
        dictionary
    ));

    program->compileShaderObjects();
    program->linkProgramObject();
    return program;
}

void ProgramObject::setIgnoreUniformLocationError(IgnoreError ignoreError) {
    _ignoreUniformLocationError = ignoreError;
}

bool ProgramObject::ignoreUniformLocationError() const {
    return _ignoreUniformLocationError;
}

GLint ProgramObject::uniformLocation(const std::string& name) const {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = glGetUniformLocation(_id, name.c_str());
    if (!_ignoreUniformLocationError && location == -1) {
        LWARNING("Failed to locate uniform location for: " + name);
    }
    return location;
}

bool ProgramObject::setUniform(const std::string& name, bool value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, bool v1, bool v2) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, bool v1, bool v2, bool v3) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2, v3);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, bool v1, bool v2, bool v3,
                               bool v4)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::bvec2& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::bvec3& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::bvec4& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const bool* values, int count) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, glm::bvec2* values, int count) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, glm::bvec3* values, int count) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, glm::bvec4* values, int count) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLuint value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLuint v1, GLuint v2) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLuint v1, GLuint v2, GLuint v3) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2, v3);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLuint v1, GLuint v2, GLuint v3,
                               GLuint v4)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::uvec2& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::uvec3& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::uvec4& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const GLuint* values, int count) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, glm::uvec2* values, int count) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, glm::uvec3* values, int count) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, glm::uvec4* values, int count) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLint value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLint v1, GLint v2) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLint v1, GLint v2, GLint v3) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2, v3);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLint v1, GLint v2, GLint v3,
                               GLint v4)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::ivec2& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::ivec3& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::ivec4& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const GLint* values, int count) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, glm::ivec2* values, int count) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, glm::ivec3* values, int count) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, glm::ivec4* values, int count) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLfloat value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLfloat v1, GLfloat v2) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLfloat v1, GLfloat v2,
                               GLfloat v3)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2, v3);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLfloat v1, GLfloat v2,
                               GLfloat v3, GLfloat v4)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::vec2& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::vec3& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::vec4& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const GLfloat* values,
                               int count)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, glm::vec2* values, int count) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, glm::vec3* values, int count) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, glm::vec4* values, int count) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLdouble value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLdouble v1, GLdouble v2) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLdouble v1, GLdouble v2,
                               GLdouble v3)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2, v3);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLdouble v1, GLdouble v2,
                               GLdouble v3, GLdouble v4)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::dvec2& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::dvec3& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::dvec4& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const GLdouble* values,
                               int count)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, glm::dvec2* values, int count) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, glm::dvec3* values, int count) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, glm::dvec4* values, int count) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values, count);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::mat2x2& value,
                               Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::mat2x3& value,
                               Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::mat2x4& value,
                               Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::mat3x2& value,
                               Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::mat3x3& value,
                               Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::mat3x4& value,
                               Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::mat4x2& value,
                               Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::mat4x3& value,
                               Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::mat4x4& value,
                               Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::dmat2x2& value,
                               Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::dmat2x3& value,
                               Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::dmat2x4& value,
                               Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::dmat3x2& value,
                               Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::dmat3x3& value,
                               Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::dmat3x4& value,
                               Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::dmat4x2& value,
                               Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::dmat4x3& value,
                               Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value, transpose);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::dmat4x4& value,
                               Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value, transpose);
    return true;
}

void ProgramObject::setUniform(GLint location, bool value) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform1i.isResolved()) {
        glProgramUniform1i(_id, location, value);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1i(location, value);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, bool v1, bool v2) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2i.isResolved()) {
        glProgramUniform2i(_id, location, v1, v2);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2i(location, v1, v2);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, bool v1, bool v2, bool v3) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3i.isResolved()) {
        glProgramUniform3i(_id, location, v1, v2, v3);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3i(location, v1, v2, v3);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, bool v1, bool v2, bool v3, bool v4) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4i.isResolved()) {
        glProgramUniform4i(_id, location, v1, v2, v3, v4);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4i(location, v1, v2, v3, v4);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::bvec2& value) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2iv.isResolved()) {
        glProgramUniform2iv(_id, location, 1, glm::value_ptr(glm::ivec2(value)));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2iv(location, 1, glm::value_ptr(glm::ivec2(value)));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::bvec3& value) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3iv.isResolved()) {
        glProgramUniform3iv(_id, location, 1, glm::value_ptr(glm::ivec3(value)));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3iv(location, 1, glm::value_ptr(glm::ivec3(value)));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::bvec4& value) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4iv.isResolved()) {
        glProgramUniform4iv(_id, location, 1, glm::value_ptr(glm::ivec4(value)));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4iv(location, 1, glm::value_ptr(glm::ivec4(value)));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const bool* values, int count) {
    ghoul_assert(location != -1, "Location must not be -1");

    std::vector<GLint> castValues(count);
    for (int i = 0; i < count; ++i) {
        castValues[i] = values[i];
    }
    if (glbinding::Binding::ProgramUniform1iv.isResolved()) {
        glProgramUniform1iv(_id, location, count, castValues.data());
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1iv(location, count, castValues.data());
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, glm::bvec2* values, int count) {
    ghoul_assert(location != -1, "Location must not be -1");

    std::vector<GLint> castValues(2 * count);
    for (int i = 0; i < count; ++i) {
        castValues[2 * i] = values[i].x;
        castValues[2 * i + 1] = values[i].y;
    }
    if (glbinding::Binding::ProgramUniform2iv.isResolved()) {
        glProgramUniform2iv(_id, location, count, castValues.data());
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2iv(location, count, castValues.data());
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, glm::bvec3* values, int count) {
    ghoul_assert(location != -1, "Location must not be -1");

    std::vector<GLint> castValues(3 * count);
    for (int i = 0; i < count; ++i) {
        castValues[3 * i] = values[i].x;
        castValues[3 * i + 1] = values[i].y;
        castValues[3 * i + 2] = values[i].z;
    }
    if (glbinding::Binding::ProgramUniform3iv.isResolved()) {
        glProgramUniform3iv(_id, location, count, castValues.data());
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3iv(location, count, castValues.data());
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, glm::bvec4* values, int count) {
    ghoul_assert(location != -1, "Location must not be -1");

    std::vector<GLint> castValues(4 * count);
    for (int i = 0; i < count; ++i) {
        castValues[4 * i] = values[i].x;
        castValues[4 * i + 1] = values[i].y;
        castValues[4 * i + 2] = values[i].z;
        castValues[4 * i + 3] = values[i].w;
    }
    if (glbinding::Binding::ProgramUniform4iv.isResolved()) {
        glProgramUniform4iv(_id, location, count, castValues.data());
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4iv(location, count, castValues.data());
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLuint value) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform1ui.isResolved()) {
        glProgramUniform1ui(_id, location, value);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1ui(location, value);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLuint v1, GLuint v2) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2ui.isResolved()) {
        glProgramUniform2ui(_id, location, v1, v2);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2ui(location, v1, v2);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLuint v1, GLuint v2, GLuint v3) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3ui.isResolved()) {
        glProgramUniform3ui(_id, location, v1, v2, v3);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3ui(location, v1, v2, v3);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location,
                               GLuint v1, GLuint v2, GLuint v3, GLuint v4)
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4ui.isResolved()) {
        glProgramUniform4ui(_id, location, v1, v2, v3, v4);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4ui(location, v1, v2, v3, v4);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::uvec2& value) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2uiv.isResolved()) {
        glProgramUniform2uiv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2uiv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::uvec3& value) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3uiv.isResolved()) {
        glProgramUniform3uiv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3uiv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::uvec4& value) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4uiv.isResolved()) {
        glProgramUniform4uiv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4uiv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const GLuint* values, int count) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform1uiv.isResolved()) {
        glProgramUniform1uiv(_id, location, count, values);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1uiv(location, count, values);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, glm::uvec2* values, int count) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2uiv.isResolved()) {
        glProgramUniform2uiv(_id, location, count,
                             reinterpret_cast<unsigned int*>(values));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2uiv(location, count, reinterpret_cast<unsigned int*>(values));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, glm::uvec3* values, int count) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3uiv.isResolved()) {
        glProgramUniform3uiv(_id, location, count,
                             reinterpret_cast<unsigned int*>(values));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3uiv(location, count, reinterpret_cast<unsigned int*>(values));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, glm::uvec4* values, int count) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4uiv.isResolved()) {
        glProgramUniform4uiv(_id, location, count,
                             reinterpret_cast<unsigned int*>(values));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4uiv(location, count, reinterpret_cast<unsigned int*>(values));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLint value) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform1i.isResolved()) {
        glProgramUniform1i(_id, location, value);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1i(location, value);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLint v1, GLint v2) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2i.isResolved()) {
        glProgramUniform2i(_id, location, v1, v2);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2i(location, v1, v2);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLint v1, GLint v2, GLint v3) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3i.isResolved()) {
        glProgramUniform3i(_id, location, v1, v2, v3);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3i(location, v1, v2, v3);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLint v1, GLint v2, GLint v3, GLint v4) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4i.isResolved()) {
        glProgramUniform4i(_id, location, v1, v2, v3, v4);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4i(location, v1, v2, v3, v4);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::ivec2& value) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2iv.isResolved()) {
        glProgramUniform2iv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2iv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::ivec3& value) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3iv.isResolved()) {
        glProgramUniform3iv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3iv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::ivec4& value) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4iv.isResolved()) {
        glProgramUniform4iv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4iv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const GLint* values, int count) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform1iv.isResolved()) {
        glProgramUniform1iv(_id, location, count, values);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1iv(location, count, values);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, glm::ivec2* values, int count) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2iv.isResolved()) {
        glProgramUniform2iv(_id, location, count, reinterpret_cast<int*>(values));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2iv(location, count, reinterpret_cast<int*>(values));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, glm::ivec3* values, int count) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3iv.isResolved()) {
        glProgramUniform3iv(_id, location, count, reinterpret_cast<int*>(values));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3iv(location, count, reinterpret_cast<int*>(values));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, glm::ivec4* values, int count) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4iv.isResolved()) {
        glProgramUniform4iv(_id, location, count, reinterpret_cast<int*>(values));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4iv(location, count, reinterpret_cast<int*>(values));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLfloat value) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform1f.isResolved()) {
        glProgramUniform1f(_id, location, value);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1f(location, value);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLfloat v1, GLfloat v2) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2f.isResolved()) {
        glProgramUniform2f(_id, location, v1, v2);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2f(location, v1, v2);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLfloat v1, GLfloat v2, GLfloat v3) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3f.isResolved()) {
        glProgramUniform3f(_id, location, v1, v2, v3);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3f(location, v1, v2, v3);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location,
                               GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4)
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4f.isResolved()) {
        glProgramUniform4f(_id, location, v1, v2, v3, v4);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4f(location, v1, v2, v3, v4);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::vec2& value) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2fv.isResolved()) {
        glProgramUniform2fv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2fv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::vec3& value) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3fv.isResolved()) {
        glProgramUniform3fv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3fv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::vec4& value) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4fv.isResolved()) {
        glProgramUniform4fv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4fv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const GLfloat* values, int count) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform1fv.isResolved()) {
        glProgramUniform1fv(_id, location, count, values);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1fv(location, count, values);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, glm::vec2* values, int count) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2fv.isResolved()) {
        glProgramUniform2fv(_id, location, count, reinterpret_cast<float*>(values));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2fv(location, count, reinterpret_cast<float*>(values));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, glm::vec3* values, int count) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3fv.isResolved()) {
        glProgramUniform3fv(_id, location, count, reinterpret_cast<float*>(values));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3fv(location, count, reinterpret_cast<float*>(values));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, glm::vec4* values, int count) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4fv.isResolved()) {
        glProgramUniform4fv(_id, location, count, reinterpret_cast<float*>(values));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4fv(location, count, reinterpret_cast<float*>(values));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLdouble value) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform1d.isResolved()) {
        glProgramUniform1d(_id, location, value);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1d(location, value);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLdouble v1, GLdouble v2) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2d.isResolved()) {
        glProgramUniform2d(_id, location, v1, v2);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2d(location, v1, v2);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLdouble v1, GLdouble v2, GLdouble v3) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3d.isResolved()) {
        glProgramUniform3d(_id, location, v1, v2, v3);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3d(location, v1, v2, v3);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLdouble v1, GLdouble v2, GLdouble v3,
                               GLdouble v4)
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4d.isResolved()) {
        glProgramUniform4d(_id, location, v1, v2, v3, v4);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4d(location, v1, v2, v3, v4);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::dvec2& value) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2dv.isResolved()) {
        glProgramUniform2dv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2dv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::dvec3& value) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3dv.isResolved()) {
        glProgramUniform3dv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3dv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::dvec4& value) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4dv.isResolved()) {
        glProgramUniform4dv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4dv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const GLdouble* values, int count) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform1dv.isResolved()) {
        glProgramUniform1dv(_id, location, count, values);
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1dv(location, count, values);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, glm::dvec2* values, int count) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2dv.isResolved()) {
        glProgramUniform2dv(_id, location, count, reinterpret_cast<double*>(values));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2dv(location, count, reinterpret_cast<double*>(values));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, glm::dvec3* values, int count) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3dv.isResolved()) {
        glProgramUniform3dv(_id, location, count, reinterpret_cast<double*>(values));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3dv(location, count, reinterpret_cast<double*>(values));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, glm::dvec4* values, int count) {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4dv.isResolved()) {
        glProgramUniform4dv(_id, location, count, reinterpret_cast<double*>(values));
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4dv(location, count, reinterpret_cast<double*>(values));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::mat2x2& value,
                               Transpose transpose)
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniformMatrix2fv.isResolved()) {
        glProgramUniformMatrix2fv(
            _id,
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix2fv(
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::mat2x3& value,
                               Transpose transpose)
{

    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniformMatrix2x3fv.isResolved()) {
        glProgramUniformMatrix2x3fv(
            _id,
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix2x3fv(
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::mat2x4& value,
                               Transpose transpose)
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniformMatrix2x4fv.isResolved()) {
        glProgramUniformMatrix2x4fv(
            _id,
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix2x4fv(
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::mat3x2& value,
                               Transpose transpose)
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniformMatrix3x2fv.isResolved()) {
        glProgramUniformMatrix3x2fv(
            _id,
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix3x2fv(
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::mat3x3& value,
                               Transpose transpose)
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniformMatrix3fv.isResolved()) {
        glProgramUniformMatrix3fv(
            _id,
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix3fv(
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::mat3x4& value,
                               Transpose transpose)
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniformMatrix3x4fv.isResolved()) {
        glProgramUniformMatrix3x4fv(
            _id,
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix3x4fv(
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::mat4x2& value,
                               Transpose transpose)
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniformMatrix4x2fv.isResolved()) {
        glProgramUniformMatrix4x2fv(
            _id,
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix4x2fv(
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::mat4x3& value,
                               Transpose transpose)
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniformMatrix4x3fv.isResolved()) {
        glProgramUniformMatrix4x3fv(
            _id,
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix4x3fv(
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::mat4x4& value,
                               Transpose transpose)
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniformMatrix4fv.isResolved()) {
        glProgramUniformMatrix4fv(
            _id,
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix4fv(
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::dmat2x2& value,
                               Transpose transpose)
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniformMatrix2dv.isResolved()) {
        glProgramUniformMatrix2dv(
            _id,
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix2dv(
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::dmat2x3& value,
                               Transpose transpose)
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniformMatrix2x3dv.isResolved()) {
        glProgramUniformMatrix2x3dv(
            _id,
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix2x3dv(
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::dmat2x4& value,
                               Transpose transpose)
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniformMatrix2x4dv.isResolved()) {
        glProgramUniformMatrix2x4dv(
            _id,
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix2x4dv(
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::dmat3x2& value,
                               Transpose transpose)
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniformMatrix3x2dv.isResolved()) {
        glProgramUniformMatrix3x2dv(
            _id,
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix3x2dv(
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::dmat3x3& value,
                               Transpose transpose)
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniformMatrix3dv.isResolved()) {
        glProgramUniformMatrix3dv(
            _id,
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix3dv(
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::dmat3x4& value,
                               Transpose transpose)
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniformMatrix3x4dv.isResolved()) {
        glProgramUniformMatrix3x4dv(
            _id,
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix3x4dv(
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::dmat4x2& value,
                               Transpose transpose)
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniformMatrix4x2dv.isResolved()) {
        glProgramUniformMatrix4x2dv(
            _id,
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix4x2dv(
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::dmat4x3& value,
                               Transpose transpose)
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniformMatrix4x3dv.isResolved()) {
        glProgramUniformMatrix4x3dv(
            _id,
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix4x3dv(
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::dmat4x4& value,
                               Transpose transpose)
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniformMatrix4dv.isResolved()) {
        glProgramUniformMatrix4dv(
            _id,
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
    }
    else {
        GLint oldProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniformMatrix4dv(
            location,
            1,
            transpose ? GL_TRUE : GL_FALSE,
            value_ptr(value)
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

bool ProgramObject::setSsboBinding(const std::string& name, GLuint binding) {
    GLuint index = glGetProgramResourceIndex(_id, GL_SHADER_STORAGE_BLOCK, name.c_str());
    if (index == GL_INVALID_INDEX) {
        return false;
    }
    setSsboBinding(index, binding);
    return true;
}

void ProgramObject::setSsboBinding(GLuint index, GLuint binding) {
    glShaderStorageBlockBinding(_id, index, binding);
}

GLuint ProgramObject::attributeLocation(const std::string& name) const {
    const GLint location = glGetAttribLocation(_id, name.c_str());
    if (!_ignoreAttributeLocationError && location == -1) {
        LWARNING("Failed to locate attribute location for: " + name);
        return GL_INVALID_INDEX;
    }
    return static_cast<GLuint>(location);
}

void ProgramObject::bindAttributeLocation(const std::string& name, GLuint index) {
    glBindAttribLocation(_id, index, name.c_str());
}

bool ProgramObject::ignoreAttributeLocationError() const {
    return _ignoreAttributeLocationError;
}

void ProgramObject::setIgnoreAttributeLocationError(IgnoreError ignoreError) {
    _ignoreAttributeLocationError = ignoreError;
}

bool ProgramObject::setAttribute(const std::string& name, bool value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, bool v1, bool v2) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, v1, v2);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, bool v1, bool v2, bool v3) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, v1, v2, v3);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, bool v1, bool v2, bool v3,
                                 bool v4)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::bvec2& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::bvec3& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::bvec4& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, GLint value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, GLint v1, GLint v2) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, v1, v2);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, GLint v1, GLint v2, GLint v3) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, v1, v2, v3);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, GLint v1, GLint v2, GLint v3,
                                 GLint v4)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::ivec2& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::ivec3& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::ivec4& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, GLfloat value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, GLfloat v1, GLfloat v2) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, v1, v2);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, GLfloat v1, GLfloat v2,
                                 GLfloat v3)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, v1, v2, v3);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, GLfloat v1, GLfloat v2,
                                 GLfloat v3, GLfloat v4)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::vec2& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::vec3& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::vec4& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, GLdouble value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, GLdouble v1, GLdouble v2) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, v1, v2);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, GLdouble v1, GLdouble v2,
                                 GLdouble v3)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, v1, v2, v3);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, GLdouble v1, GLdouble v2,
                                 GLdouble v3, GLdouble v4)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::dvec2& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::dvec3& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::dvec4& value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::mat2x2& value,
                                 Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::mat2x3& value,
                                 Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::mat2x4& value,
                                 Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::mat3x2& value,
                                 Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::mat3x3& value,
                                 Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::mat3x4& value,
                                 Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::mat4x2& value,
                                 Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::mat4x3& value,
                                 Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::mat4x4& value,
                                 Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::dmat2x2& value,
                                 Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::dmat2x3& value,
                                 Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::dmat2x4& value,
                                 Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::dmat3x2& value,
                                 Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::dmat3x3& value,
                                 Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::dmat3x4& value,
                                 Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::dmat4x2& value,
                                 Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::dmat4x3& value,
                                 Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value, transpose);
    return true;
}

bool ProgramObject::setAttribute(const std::string& name, const glm::dmat4x4& value,
                                 Transpose transpose)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
    if (location == GL_INVALID_INDEX) {
        return false;
    }
    setAttribute(location, value, transpose);
    return true;
}

void ProgramObject::setAttribute(GLuint location, bool value) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttribI1i(location, value);
}

void ProgramObject::setAttribute(GLuint location, bool v1, bool v2) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttribI2i(location, v1, v2);
}

void ProgramObject::setAttribute(GLuint location, bool v1, bool v2, bool v3) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttribI3i(location, v1, v2, v3);
}

void ProgramObject::setAttribute(GLuint location, bool v1, bool v2, bool v3, bool v4) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttribI4i(location, v1, v2, v3, v4);
}

void ProgramObject::setAttribute(GLuint location, const glm::bvec2& value) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttribI2iv(location, value_ptr(glm::ivec2(value)));
}

void ProgramObject::setAttribute(GLuint location, const glm::bvec3& value) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttribI3iv(location, value_ptr(glm::ivec3(value)));
}

void ProgramObject::setAttribute(GLuint location, const glm::bvec4& value) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttribI4iv(location, value_ptr(glm::ivec4(value)));
}

void ProgramObject::setAttribute(GLuint location, GLint value) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttribI1i(location, value);
}

void ProgramObject::setAttribute(GLuint location, GLint v1, GLint v2) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttribI2i(location, v1, v2);
}

void ProgramObject::setAttribute(GLuint location, GLint v1, GLint v2, GLint v3) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttribI3i(location, v1, v2, v3);
}

void ProgramObject::setAttribute(GLuint location, GLint v1, GLint v2, GLint v3, GLint v4)
{
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttribI4i(location, v1, v2, v3, v4);
}

void ProgramObject::setAttribute(GLuint location, const glm::ivec2& value) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttribI2iv(location, value_ptr(value));
}

void ProgramObject::setAttribute(GLuint location, const glm::ivec3& value) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttribI3iv(location, value_ptr(value));
}

void ProgramObject::setAttribute(GLuint location, const glm::ivec4& value) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttribI4iv(location, value_ptr(value));
}

void ProgramObject::setAttribute(GLuint location, GLfloat value) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttrib1f(location, value);
}

void ProgramObject::setAttribute(GLuint location, GLfloat v1, GLfloat v2) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttrib2f(location, v1, v2);
}

void ProgramObject::setAttribute(GLuint location, GLfloat v1, GLfloat v2, GLfloat v3) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttrib3f(location, v1, v2, v3);
}

void ProgramObject::setAttribute(GLuint location,
                                 GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4)
{
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttrib4f(location, v1, v2, v3, v4);
}

void ProgramObject::setAttribute(GLuint location, const glm::vec2& value) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttrib2fv(location, value_ptr(value));
}

void ProgramObject::setAttribute(GLuint location, const glm::vec3& value) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttrib3fv(location, value_ptr(value));
}

void ProgramObject::setAttribute(GLuint location, const glm::vec4& value) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttrib4fv(location, value_ptr(value));
}

void ProgramObject::setAttribute(GLuint location, GLdouble value) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttribL1d(location, value);
}

void ProgramObject::setAttribute(GLuint location, GLdouble v1, GLdouble v2) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttribL2d(location, v1, v2);
}

void ProgramObject::setAttribute(GLuint location, GLdouble v1, GLdouble v2, GLdouble v3) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttribL3d(location, v1, v2, v3);
}

void ProgramObject::setAttribute(GLuint location,
                                 GLdouble v1, GLdouble v2, GLdouble v3, GLdouble v4)
{
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttribL4d(location, v1, v2, v3, v4);
}

void ProgramObject::setAttribute(GLuint location, const glm::dvec2& value) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttribL2dv(location, value_ptr(value));
}

void ProgramObject::setAttribute(GLuint location, const glm::dvec3& value) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttribL3dv(location, value_ptr(value));
}

void ProgramObject::setAttribute(GLuint location, const glm::dvec4& value) {
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    glVertexAttribL4dv(location, value_ptr(value));
}

void ProgramObject::setAttribute(GLuint location, const glm::mat2x2& value,
                                 Transpose transpose)
{
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    if (transpose == Transpose::Yes) {
        setAttribute(location, glm::transpose(value));
    }
    else {
        glVertexAttrib2fv(location, value_ptr(value[0]));
        glVertexAttrib2fv(location + 1, value_ptr(value[1]));
    }
}

void ProgramObject::setAttribute(GLuint location, const glm::mat2x3& value,
                                 Transpose transpose)
{
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    if (transpose == Transpose::Yes) {
        setAttribute(location, glm::transpose(value));
    }
    else {
        glVertexAttrib3fv(location, value_ptr(value[0]));
        glVertexAttrib3fv(location + 1, value_ptr(value[1]));
    }
}

void ProgramObject::setAttribute(GLuint location, const glm::mat2x4& value,
                                 Transpose transpose)
{
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    if (transpose == Transpose::Yes) {
        setAttribute(location, glm::transpose(value));
    }
    else {
        glVertexAttrib4fv(location, value_ptr(value[0]));
        glVertexAttrib4fv(location + 1, value_ptr(value[1]));
    }
}

void ProgramObject::setAttribute(GLuint location, const glm::mat3x2& value,
                                 Transpose transpose)
{
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    if (transpose == Transpose::Yes) {
        setAttribute(location, glm::transpose(value));
    }
    else {
        glVertexAttrib2fv(location, value_ptr(value[0]));
        glVertexAttrib2fv(location + 1, value_ptr(value[1]));
        glVertexAttrib2fv(location + 2, value_ptr(value[2]));
    }
}

void ProgramObject::setAttribute(GLuint location, const glm::mat3x3& value,
                                 Transpose transpose)
{
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    if (transpose == Transpose::Yes) {
        setAttribute(location, glm::transpose(value));
    }
    else {
        glVertexAttrib3fv(location, value_ptr(value[0]));
        glVertexAttrib3fv(location + 1, value_ptr(value[1]));
        glVertexAttrib3fv(location + 2, value_ptr(value[2]));
    }
}

void ProgramObject::setAttribute(GLuint location, const glm::mat3x4& value,
                                 Transpose transpose)
{
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    if (transpose == Transpose::Yes) {
        setAttribute(location, glm::transpose(value));
    }
    else {
        glVertexAttrib4fv(location, value_ptr(value[0]));
        glVertexAttrib4fv(location + 1, value_ptr(value[1]));
        glVertexAttrib4fv(location + 2, value_ptr(value[2]));
    }
}

void ProgramObject::setAttribute(GLuint location, const glm::mat4x2& value,
                                 Transpose transpose)
{
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    if (transpose == Transpose::Yes) {
        setAttribute(location, glm::transpose(value));
    }
    else {
        glVertexAttrib2fv(location, value_ptr(value[0]));
        glVertexAttrib2fv(location + 1, value_ptr(value[1]));
        glVertexAttrib2fv(location + 2, value_ptr(value[2]));
        glVertexAttrib2fv(location + 3, value_ptr(value[3]));
    }
}

void ProgramObject::setAttribute(GLuint location, const glm::mat4x3& value,
                                 Transpose transpose)
{
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    if (transpose == Transpose::Yes) {
        setAttribute(location, glm::transpose(value));
    }
    else {
        glVertexAttrib3fv(location, value_ptr(value[0]));
        glVertexAttrib3fv(location + 1, value_ptr(value[1]));
        glVertexAttrib3fv(location + 2, value_ptr(value[2]));
        glVertexAttrib3fv(location + 3, value_ptr(value[3]));
    }
}

void ProgramObject::setAttribute(GLuint location, const glm::mat4x4& value,
                                 Transpose transpose)
{
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    if (transpose == Transpose::Yes) {
        setAttribute(location, glm::transpose(value));
    }
    else {
        glVertexAttrib4fv(location, value_ptr(value[0]));
        glVertexAttrib4fv(location + 1, value_ptr(value[1]));
        glVertexAttrib4fv(location + 2, value_ptr(value[2]));
        glVertexAttrib4fv(location + 3, value_ptr(value[3]));
    }
}

void ProgramObject::setAttribute(GLuint location, const glm::dmat2x2& value,
                                 Transpose transpose)
{
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    if (transpose == Transpose::Yes) {
        setAttribute(location, glm::transpose(value));
    }
    else {
        glVertexAttribL2dv(location, value_ptr(value[0]));
        glVertexAttribL2dv(location + 1, value_ptr(value[1]));
    }
}

void ProgramObject::setAttribute(GLuint location, const glm::dmat2x3& value,
                                 Transpose transpose)
{
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    if (transpose == Transpose::Yes) {
        setAttribute(location, glm::transpose(value));
    }
    else {
        glVertexAttribL3dv(location, value_ptr(value[0]));
        glVertexAttribL3dv(location + 1, value_ptr(value[1]));
    }
}

void ProgramObject::setAttribute(GLuint location, const glm::dmat2x4& value,
                                 Transpose transpose)
{
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    if (transpose == Transpose::Yes) {
        setAttribute(location, glm::transpose(value));
    }
    else {
        glVertexAttribL4dv(location, value_ptr(value[0]));
        glVertexAttribL4dv(location + 1, value_ptr(value[1]));
    }
}

void ProgramObject::setAttribute(GLuint location, const glm::dmat3x2& value,
                                 Transpose transpose)
{
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    if (transpose == Transpose::Yes) {
        setAttribute(location, glm::transpose(value));
    }
    else {
        glVertexAttribL2dv(location, value_ptr(value[0]));
        glVertexAttribL2dv(location + 1, value_ptr(value[1]));
        glVertexAttribL2dv(location + 2, value_ptr(value[2]));
    }
}

void ProgramObject::setAttribute(GLuint location, const glm::dmat3x3& value,
                                 Transpose transpose)
{
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    if (transpose == Transpose::Yes) {
        setAttribute(location, glm::transpose(value));
    }
    else {
        glVertexAttribL3dv(location, value_ptr(value[0]));
        glVertexAttribL3dv(location + 1, value_ptr(value[1]));
        glVertexAttribL3dv(location + 2, value_ptr(value[2]));
    }
}

void ProgramObject::setAttribute(GLuint location, const glm::dmat3x4& value,
                                 Transpose transpose)
{
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    if (transpose == Transpose::Yes) {
        setAttribute(location, glm::transpose(value));
    }
    else {
        glVertexAttribL4dv(location, value_ptr(value[0]));
        glVertexAttribL4dv(location + 1, value_ptr(value[1]));
        glVertexAttribL4dv(location + 2, value_ptr(value[2]));
    }
}

void ProgramObject::setAttribute(GLuint location, const glm::dmat4x2& value,
                                 Transpose transpose)
{
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    if (transpose == Transpose::Yes) {
        setAttribute(location, glm::transpose(value));
    }
    else {
        glVertexAttribL2dv(location, value_ptr(value[0]));
        glVertexAttribL2dv(location + 1, value_ptr(value[1]));
        glVertexAttribL2dv(location + 2, value_ptr(value[2]));
        glVertexAttribL2dv(location + 3, value_ptr(value[3]));
    }
}

void ProgramObject::setAttribute(GLuint location, const glm::dmat4x3& value,
                                 Transpose transpose)
{
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    if (transpose == Transpose::Yes) {
        setAttribute(location, glm::transpose(value));
    }
    else {
        glVertexAttribL3dv(location, value_ptr(value[0]));
        glVertexAttribL3dv(location + 1, value_ptr(value[1]));
        glVertexAttribL3dv(location + 2, value_ptr(value[2]));
        glVertexAttribL3dv(location + 3, value_ptr(value[3]));
    }
}

void ProgramObject::setAttribute(GLuint location, const glm::dmat4x4& value,
                                 Transpose transpose)
{
    ghoul_assert(location != GL_INVALID_INDEX, "Location must not be GL_INVALID_INDEX");

    if (transpose == Transpose::Yes) {
        setAttribute(location, glm::transpose(value));
    }
    else {
        glVertexAttribL4dv(location, value_ptr(value[0]));
        glVertexAttribL4dv(location + 1, value_ptr(value[1]));
        glVertexAttribL4dv(location + 2, value_ptr(value[2]));
        glVertexAttribL4dv(location + 3, value_ptr(value[3]));
    }
}

void ProgramObject::setIgnoreSubroutineLocationError(IgnoreError ignoreError) {
    _ignoreSubroutineLocationError = ignoreError;
}

bool ProgramObject::ignoreSubroutineLocationError() const {
    return _ignoreSubroutineLocationError;
}

void ProgramObject::setIgnoreSubroutineUniformLocationError(IgnoreError ignoreError) {
    _ignoreSubroutineUniformLocationError = ignoreError;
}

bool ProgramObject::ignoreSubroutineUniformLocationError() const {
    return _ignoreSubroutineUniformLocationError;
}

GLuint ProgramObject::subroutineIndex(ShaderObject::ShaderType shaderType,
                                      const std::string& name)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint index = glGetSubroutineIndex(
        _id,
        static_cast<GLenum>(shaderType),
        name.c_str()
    );
    if (!_ignoreSubroutineLocationError && index == GL_INVALID_INDEX) {
        LWARNING("Failed to locate subroutine index for: " + name);
    }
    return index;
}

GLint ProgramObject::subroutineUniformLocation(ShaderObject::ShaderType shaderType,
                                               const std::string& name)
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = glGetSubroutineUniformLocation(
        _id,
        static_cast<GLenum>(shaderType),
        name.c_str()
    );
    if (!_ignoreSubroutineUniformLocationError && location == -1) {
        LWARNING("Failed to locate subroutine uniform location for: " + name);
    }
    return location;
}

vector<string> ProgramObject::activeSubroutineUniformNames(
                                                      ShaderObject::ShaderType shaderType)
{
    GLint maximumUniformNameLength;
    glGetProgramStageiv(
        _id,
        static_cast<GLenum>(shaderType),
        GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH,
        &maximumUniformNameLength
    );

    int countActiveSubroutineUniforms;
    std::vector<char> buffer(maximumUniformNameLength);
    glGetProgramStageiv(
        _id,
        static_cast<GLenum>(shaderType),
        GL_ACTIVE_SUBROUTINE_UNIFORMS,
        &countActiveSubroutineUniforms
    );
    vector<string> result(static_cast<size_t>(countActiveSubroutineUniforms));
    for (GLuint i = 0; i < static_cast<GLuint>(countActiveSubroutineUniforms); ++i) {
        glGetActiveSubroutineUniformName(
            _id,
            static_cast<GLenum>(shaderType),
            i,
            maximumUniformNameLength,
            nullptr,
            buffer.data()
        );
        result[i] = string(buffer.data());
    }

    return result;
}

vector<string> ProgramObject::compatibleSubroutineNames(
                                                      ShaderObject::ShaderType shaderType,
                                                      GLuint subroutineUniformLocation)
{
    ghoul_assert(
        subroutineUniformLocation != GL_INVALID_INDEX,
        "Location must not be GL_INVALID_INDEX"
    );

    GLint maximumUniformNameLength;
    glGetProgramStageiv(
        _id,
        static_cast<GLenum>(shaderType),
        GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH,
        &maximumUniformNameLength
    );

    GLint numCompatibleSubroutines;
    glGetActiveSubroutineUniformiv(
        _id,
        static_cast<GLenum>(shaderType),
        subroutineUniformLocation,
        GL_NUM_COMPATIBLE_SUBROUTINES,
        &numCompatibleSubroutines
    );
    if (numCompatibleSubroutines == 0) {
        return vector<string>();
    }

    vector<string> result(static_cast<size_t>(numCompatibleSubroutines));
    std::vector<GLint> indices(numCompatibleSubroutines);
    std::vector<char> buffer(maximumUniformNameLength);
    glGetActiveSubroutineUniformiv(
        _id,
        static_cast<GLenum>(shaderType),
        subroutineUniformLocation,
        GL_COMPATIBLE_SUBROUTINES,
        indices.data()
    );
    for (GLuint i = 0; i < static_cast<GLuint>(numCompatibleSubroutines); ++i) {
        glGetActiveSubroutineName(
            _id,
            static_cast<GLenum>(shaderType),
            static_cast<GLuint>(indices[i]),
            maximumUniformNameLength,
            nullptr,
            buffer.data()
        );
        result[i] = string(buffer.data());
    }
    return result;
}

vector<string> ProgramObject::compatibleSubroutineNames(
                                                      ShaderObject::ShaderType shaderType,
                                                 const std::string& subroutineUniformName)
{
    ghoul_assert(!subroutineUniformName.empty(), "Name must not be empty");

    const GLint index = subroutineUniformLocation(shaderType, subroutineUniformName);
    if (index == -1) {
        return vector<string>();
    }
    else {
        return compatibleSubroutineNames(shaderType, static_cast<GLuint>(index));
    }
}

bool ProgramObject::setUniformSubroutines(ShaderObject::ShaderType shaderType,
                                          const std::vector<GLuint>& indices)
{
    ghoul_assert(!indices.empty(), "Values must not be empty");

#ifdef GHL_DEBUG
    int countActiveSubroutineUniforms;
    glGetProgramStageiv(
        _id,
        static_cast<GLenum>(shaderType),
        GL_ACTIVE_SUBROUTINE_UNIFORMS,
        &countActiveSubroutineUniforms
    );
    if (static_cast<size_t>(countActiveSubroutineUniforms) != indices.size()) {
        LWARNING(fmt::format(
            "Number of active subroutine uniforms ({}) is different from passed uniform "
            "subroutine indices ({})",
            countActiveSubroutineUniforms,
            indices.size()
        ));
        return false;
    }
#endif
    glUniformSubroutinesuiv(
        static_cast<GLenum>(shaderType),
        static_cast<GLsizei>(indices.size()),
        indices.data()
    );
    return true;
}

bool ProgramObject::setUniformSubroutines(ShaderObject::ShaderType shaderType,
                                         const std::map<std::string, std::string>& values)
{
    ghoul_assert(!values.empty(), "Values must not be empty");

#ifdef GHL_DEBUG
    int countActiveSubroutineUniforms;
    glGetProgramStageiv(
        _id,
        static_cast<GLenum>(shaderType),
        GL_ACTIVE_SUBROUTINE_UNIFORMS,
        &countActiveSubroutineUniforms
    );
    if (static_cast<size_t>(countActiveSubroutineUniforms) != values.size()) {
        LWARNING(fmt::format(
            "Number of active subroutine uniforms ({}) is different from passed uniform "
            "subroutine indices ({})",
            countActiveSubroutineUniforms,
            values.size()
        ));
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
            LWARNING(fmt::format(
                "Uniform subroutine name '{}' was not present in map", uniformSubroutine
            ));
            return false;
        }
#endif
        const string& nameSubroutine = subroutine->second;
        GLuint idxSubroutine = subroutineIndex(shaderType, nameSubroutine);
#ifdef GHL_DEBUG
        if (idxSubroutine == GL_INVALID_INDEX) {
            LWARNING(fmt::format(
                "Subroutine name '{}' was not found in shader object", nameSubroutine
            ));
            return false;
        }
#endif
        uniformIndices[i] = idxSubroutine;
    }
    glUniformSubroutinesuiv(
        static_cast<GLenum>(shaderType),
        static_cast<GLsizei>(uniformIndices.size()),
        &uniformIndices[0]);
    return true;
}

void ProgramObject::bindFragDataLocation(const std::string& name, GLuint colorNumber) {
    ghoul_assert(!name.empty(), "Name must not be empty");
    ghoul_assert(
        colorNumber != GL_INVALID_INDEX,
        "Location must not be GL_INVALID_INDEX"
    );

#ifdef GHL_DEBUG
    GLint maxBuffers;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxBuffers);
    if (colorNumber >= static_cast<GLuint>(maxBuffers)) {
        LWARNING(fmt::format(
            "ColorNumber '{}' is bigger than the maximum of simultaneous outputs '{}'",
            colorNumber, maxBuffers
        ));
        return;
    }
#endif

    glBindFragDataLocation(_id, colorNumber, name.c_str());
}

} // namespace ghoul::opengl
