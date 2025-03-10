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

#include <ghoul/opengl/programobject.h>

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/format.h>
#include <ghoul/logging/logmanager.h>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>

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
        std::format("Error linking program object: {}", msg) :
        std::format("Error linking program object [{}]: {}", name, msg)
      )
    , linkerError(std::move(msg))
    , programName(std::move(name))
{}

ProgramObject::ProgramObject()
    : _id(glCreateProgram())
{
    if (_id == 0) {
        throw ProgramObjectError("glCreateProgram returned 0");
    }
}

ProgramObject::ProgramObject(std::string name)
    : _id(glCreateProgram())
    , _programName(std::move(name))
    , _loggerCat(std::format("ProgramObject('{}')", _programName))
{
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
    : _id(glCreateProgram())
    , _programName(cpy._programName)
    , _loggerCat(cpy._loggerCat)
    , _ignoreUniformLocationError(cpy._ignoreUniformLocationError)
    , _ignoreAttributeLocationError(cpy._ignoreAttributeLocationError)
    , _ignoreSubroutineLocationError(cpy._ignoreSubroutineLocationError)
    , _ignoreSubroutineUniformLocationError(cpy._ignoreSubroutineUniformLocationError)
{
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
        auto shaderCopy = std::make_shared<ShaderObject>(*obj);
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
            auto shaderCopy = std::make_shared<ShaderObject>(*obj);
            glAttachShader(_id, *shaderCopy);
            _shaderObjects.push_back(shaderCopy);
        }
    }
    return *this;
}

ProgramObject& ProgramObject::operator=(ProgramObject&& rhs) noexcept {
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
        _programIsDirty = rhs._programIsDirty;

        _shaderObjects.clear();
        _shaderObjects = std::move(rhs._shaderObjects);
    }
    return *this;
}

void ProgramObject::setName(std::string name) {
    _programName = std::move(name);
    _loggerCat = std::format("ProgramObject['{}']", _programName);
    if (glbinding::Binding::ObjectLabel.isResolved()) {
        glObjectLabel(
            GL_PROGRAM,
            _id,
            static_cast<GLsizei>(_programName.length() + 1),
            _programName.c_str()
        );
    }
}

const std::string& ProgramObject::name() const{
    return _programName;
}

void ProgramObject::setDictionary(const Dictionary& dictionary) {
    for (const std::shared_ptr<ShaderObject>& shaderObject : _shaderObjects) {
        shaderObject->setDictionary(dictionary);
    }
}

Dictionary ProgramObject::dictionary() const {
    if (_shaderObjects.empty()) {
        throw ProgramObjectError("No shader object attached");
    }
    return _shaderObjects[0]->dictionary();
}

void ProgramObject::setProgramObjectCallback(ProgramObjectCallback changeCallback) {
    const ShaderObject::ShaderObjectCallback c = [this, changeCallback](){
        _programIsDirty = true;
        changeCallback(this);
    };
    for (const std::shared_ptr<ShaderObject>& shaderObject : _shaderObjects) {
        shaderObject->setShaderObjectCallback(c);
    }
}

void ProgramObject::attachObject(std::shared_ptr<ShaderObject> shaderObject) {
    ghoul_assert(shaderObject, "ShaderObject must not be nullptr");
    auto it = std::find(_shaderObjects.cbegin(), _shaderObjects.cend(), shaderObject);
    ghoul_assert(it == _shaderObjects.cend(), "ShaderObject was already registered");

    shaderObject->setShaderObjectCallback([this]() { _programIsDirty = true; });

    glAttachShader(_id, *shaderObject);
    _shaderObjects.push_back(std::move(shaderObject));
}

void ProgramObject::detachObject(const std::shared_ptr<ShaderObject>& shaderObject) {
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

    GLint linkStatus = 0;
    glGetProgramiv(_id, GL_LINK_STATUS, &linkStatus);
    if (static_cast<GLboolean>(linkStatus) == GL_FALSE) {
        GLint logLength = 0;
        glGetProgramiv(_id, GL_INFO_LOG_LENGTH, &logLength);

        if (logLength == 0) {
            throw ProgramObjectLinkingError("Unknown error", name());
        }

        std::vector<GLchar> rawLog(logLength);
        glGetProgramInfoLog(_id, logLength, nullptr, rawLog.data());
        const std::string log = std::string(rawLog.data());
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

void ProgramObject::activate() const {
    glUseProgram(_id);
}

void ProgramObject::deactivate() const {
    glUseProgram(0);
}

std::unique_ptr<ProgramObject> ProgramObject::Build(const std::string& name,
                                            const std::filesystem::path& vertexShaderPath,
                                          const std::filesystem::path& fragmentShaderPath,
                                                             const Dictionary& dictionary)
{
    ghoul_assert(!vertexShaderPath.empty(), "VertexShaderPath must not be empty");
    ghoul_assert(
        std::filesystem::is_regular_file(vertexShaderPath),
        "VertexShaderPath file must exist"
    );
    ghoul_assert(!fragmentShaderPath.empty(), "FragmentShaderPath must not be empty");
    ghoul_assert(
        std::filesystem::is_regular_file(fragmentShaderPath),
        "FragmentShaderPath file must exist"
    );

    auto program = std::make_unique<ProgramObject>(name);
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
                                            const std::filesystem::path& vertexShaderPath,
                                          const std::filesystem::path& fragmentShaderPath,
                                          const std::filesystem::path& geometryShaderPath,
                                                             const Dictionary& dictionary)
{
    ghoul_assert(!vertexShaderPath.empty(), "VertexShaderPath must not be empty");
    ghoul_assert(
        std::filesystem::is_regular_file(vertexShaderPath),
        "VertexShaderPath file must exist"
    );
    ghoul_assert(!fragmentShaderPath.empty(), "FragmentShaderPath must not be empty");
    ghoul_assert(
        std::filesystem::is_regular_file(fragmentShaderPath),
        "FragmentShaderPath file must exist"
    );
    ghoul_assert(!geometryShaderPath.empty(), "GeometryShaderPath must not be empty");
    ghoul_assert(
        std::filesystem::is_regular_file(geometryShaderPath),
        "GeometryShaderPath file must exist"
    );

    auto program = std::make_unique<ProgramObject>(name);
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
                                            const std::filesystem::path& vertexShaderPath,
                                          const std::filesystem::path& fragmentShaderPath,
                                          const std::filesystem::path& geometryShaderPath,
                            const std::filesystem::path& tessellationEvaluationShaderPath,
                               const std::filesystem::path& tessellationControlShaderPath,
                                                             const Dictionary& dictionary)
{
    ghoul_assert(!vertexShaderPath.empty(), "VertexShaderPath must not be empty");
    ghoul_assert(
        std::filesystem::is_regular_file(vertexShaderPath),
        "VertexShaderPath file must exist"
    );
    ghoul_assert(!fragmentShaderPath.empty(), "FragmentShaderPath must not be empty");
    ghoul_assert(
        std::filesystem::is_regular_file(fragmentShaderPath),
        "FragmentShaderPath file must exist"
    );
    ghoul_assert(!geometryShaderPath.empty(), "GeometryShaderPath must not be empty");
    ghoul_assert(
        std::filesystem::is_regular_file(geometryShaderPath),
        "GeometryShaderPath file must exist"
    );
    ghoul_assert(
        !tessellationEvaluationShaderPath.empty(),
        "Tessellation evaluation shader must not be empty"
    );
    ghoul_assert(
        std::filesystem::is_regular_file(tessellationEvaluationShaderPath),
        "Tessellation evaluation shader file must exist"
    );
    ghoul_assert(
        !tessellationControlShaderPath.empty(),
        "Tessellation control shader must not be empty"
    );
    ghoul_assert(
        std::filesystem::is_regular_file(tessellationControlShaderPath),
        "Tessellation control shader file must exist"
    );

    auto program = std::make_unique<ProgramObject>(name);
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
        LWARNING(std::format("Failed to locate uniform location for '{}'", name));
    }
    return location;
}

bool ProgramObject::setUniform(const std::string& name, GLuint value) const {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLuint v1, GLuint v2) const {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLuint v1, GLuint v2,
                               GLuint v3) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2, v3);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLuint v1, GLuint v2, GLuint v3,
                               GLuint v4) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::uvec2& value) const {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::uvec3& value) const {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::uvec4& value) const {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name,
                               const std::vector<GLuint>& values) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values);
    return true;
}

bool ProgramObject::setUniform(const std::string& name,
                               const std::vector<glm::uvec2>& values) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values);
    return true;
}

bool ProgramObject::setUniform(const std::string& name,
                               const std::vector<glm::uvec3>& values) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values);
    return true;
}

bool ProgramObject::setUniform(const std::string& name,
                               const std::vector<glm::uvec4>& values) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLint value) const {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLint v1, GLint v2) const {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLint v1, GLint v2,
                               GLint v3) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2, v3);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLint v1, GLint v2, GLint v3,
                               GLint v4) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::ivec2& value) const {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::ivec3& value) const {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::ivec4& value) const {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name,
                               const std::vector<GLint>& values) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values);
    return true;
}

bool ProgramObject::setUniform(const std::string& name,
                               const std::vector<glm::ivec2>& values) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values);
    return true;
}

bool ProgramObject::setUniform(const std::string& name,
                               const std::vector<glm::ivec3>& values) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values);
    return true;
}

bool ProgramObject::setUniform(const std::string& name,
                               const std::vector<glm::ivec4>& values) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLfloat value) const {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLfloat v1, GLfloat v2) const {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLfloat v1, GLfloat v2,
                               GLfloat v3) const
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
                               GLfloat v3, GLfloat v4) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::vec2& value) const {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::vec3& value) const {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::vec4& value) const {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name,
                               const std::vector<GLfloat>& values) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values);
    return true;
}

bool ProgramObject::setUniform(const std::string& name,
                               const std::vector<glm::vec2>& values) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values);
    return true;
}

bool ProgramObject::setUniform(const std::string& name,
                               const std::vector<glm::vec3>& values) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values);
    return true;
}

bool ProgramObject::setUniform(const std::string& name,
                               const std::vector<glm::vec4>& values) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLdouble value) const {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLdouble v1, GLdouble v2) const {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, GLdouble v1, GLdouble v2,
                               GLdouble v3) const
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
                               GLdouble v3, GLdouble v4) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, v1, v2, v3, v4);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::dvec2& value) const {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::dvec3& value) const {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::dvec4& value) const {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value);
    return true;
}

bool ProgramObject::setUniform(const std::string& name,
                               const std::vector<GLdouble>& values) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values);
    return true;
}

bool ProgramObject::setUniform(const std::string& name,
                               const std::vector<glm::dvec2>& values) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values);
    return true;
}

bool ProgramObject::setUniform(const std::string& name,
                               const std::vector<glm::dvec3>& values) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values);
    return true;
}

bool ProgramObject::setUniform(const std::string& name,
                               const std::vector<glm::dvec4>& values) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, values);
    return true;
}

bool ProgramObject::setUniform(const std::string& name, const glm::mat2x2& value,
                               Transpose transpose) const
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
                               Transpose transpose) const
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
                               Transpose transpose) const
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
                               Transpose transpose) const
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
                               Transpose transpose) const
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
                               Transpose transpose) const
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
                               Transpose transpose) const
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
                               Transpose transpose) const
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
                               Transpose transpose) const
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
                               Transpose transpose) const
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
                               Transpose transpose) const
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
                               Transpose transpose) const
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
                               Transpose transpose) const
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
                               Transpose transpose) const
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
                               Transpose transpose) const
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
                               Transpose transpose) const
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
                               Transpose transpose) const
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
                               Transpose transpose) const
{
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLint location = uniformLocation(name);
    if (location == -1) {
        return false;
    }
    setUniform(location, value, transpose);
    return true;
}

void ProgramObject::setUniform(GLint location, GLuint value) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform1ui.isResolved()) {
        glProgramUniform1ui(_id, location, value);
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1ui(location, value);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLuint v1, GLuint v2) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2ui.isResolved()) {
        glProgramUniform2ui(_id, location, v1, v2);
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2ui(location, v1, v2);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLuint v1, GLuint v2, GLuint v3) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3ui.isResolved()) {
        glProgramUniform3ui(_id, location, v1, v2, v3);
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3ui(location, v1, v2, v3);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location,
                               GLuint v1, GLuint v2, GLuint v3, GLuint v4) const
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4ui.isResolved()) {
        glProgramUniform4ui(_id, location, v1, v2, v3, v4);
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4ui(location, v1, v2, v3, v4);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::uvec2& value) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2uiv.isResolved()) {
        glProgramUniform2uiv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2uiv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::uvec3& value) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3uiv.isResolved()) {
        glProgramUniform3uiv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3uiv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::uvec4& value) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4uiv.isResolved()) {
        glProgramUniform4uiv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4uiv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const std::vector<GLuint>& values) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform1uiv.isResolved()) {
        glProgramUniform1uiv(
            _id,
            location,
            static_cast<GLsizei>(values.size()),
            values.data()
        );
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1uiv(location, static_cast<GLsizei>(values.size()), values.data());
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location,
                               const std::vector<glm::uvec2>& values) const
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2uiv.isResolved()) {
        glProgramUniform2uiv(
            _id,
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const unsigned int*>(values.data())
        );
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2uiv(
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const unsigned int*>(values.data())
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location,
                               const std::vector<glm::uvec3>& values) const
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3uiv.isResolved()) {
        glProgramUniform3uiv(
            _id,
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const unsigned int*>(values.data())
        );
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3uiv(
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const unsigned int*>(values.data())
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location,
                               const std::vector<glm::uvec4>& values) const
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4uiv.isResolved()) {
        glProgramUniform4uiv(
            _id,
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const unsigned int*>(values.data())
        );
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4uiv(
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const unsigned int*>(values.data())
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLint value) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform1i.isResolved()) {
        glProgramUniform1i(_id, location, value);
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1i(location, value);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLint v1, GLint v2) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2i.isResolved()) {
        glProgramUniform2i(_id, location, v1, v2);
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2i(location, v1, v2);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLint v1, GLint v2, GLint v3) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3i.isResolved()) {
        glProgramUniform3i(_id, location, v1, v2, v3);
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3i(location, v1, v2, v3);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLint v1, GLint v2, GLint v3,
                               GLint v4) const
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4i.isResolved()) {
        glProgramUniform4i(_id, location, v1, v2, v3, v4);
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4i(location, v1, v2, v3, v4);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::ivec2& value) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2iv.isResolved()) {
        glProgramUniform2iv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2iv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::ivec3& value) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3iv.isResolved()) {
        glProgramUniform3iv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3iv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::ivec4& value) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4iv.isResolved()) {
        glProgramUniform4iv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4iv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const std::vector<GLint>& values) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform1iv.isResolved()) {
        glProgramUniform1iv(
            _id,
            location,
            static_cast<GLsizei>(values.size()),
            values.data()
        );
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1iv(location, static_cast<GLsizei>(values.size()), values.data());
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location,
                               const std::vector<glm::ivec2>& values) const
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2iv.isResolved()) {
        glProgramUniform2iv(
            _id,
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const int*>(values.data())
        );
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2iv(
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const int*>(values.data())
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location,
                               const std::vector<glm::ivec3>& values) const
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3iv.isResolved()) {
        glProgramUniform3iv(
            _id,
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const int*>(values.data())
        );
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3iv(
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const int*>(values.data())
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location,
                               const std::vector<glm::ivec4>& values) const
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4iv.isResolved()) {
        glProgramUniform4iv(
            _id,
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const int*>(values.data())
        );
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4iv(
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const int*>(values.data())
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLfloat value) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform1f.isResolved()) {
        glProgramUniform1f(_id, location, value);
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1f(location, value);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLfloat v1, GLfloat v2) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2f.isResolved()) {
        glProgramUniform2f(_id, location, v1, v2);
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2f(location, v1, v2);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLfloat v1, GLfloat v2, GLfloat v3) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3f.isResolved()) {
        glProgramUniform3f(_id, location, v1, v2, v3);
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3f(location, v1, v2, v3);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location,
                               GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4) const
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4f.isResolved()) {
        glProgramUniform4f(_id, location, v1, v2, v3, v4);
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4f(location, v1, v2, v3, v4);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::vec2& value) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2fv.isResolved()) {
        glProgramUniform2fv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2fv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::vec3& value) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3fv.isResolved()) {
        glProgramUniform3fv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3fv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::vec4& value) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4fv.isResolved()) {
        glProgramUniform4fv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4fv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const std::vector<GLfloat>& values) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform1fv.isResolved()) {
        glProgramUniform1fv(
            _id,
            location,
            static_cast<GLsizei>(values.size()),
            values.data()
        );
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1fv(location, static_cast<GLsizei>(values.size()), values.data());
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const std::vector<glm::vec2>& values) const
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2fv.isResolved()) {
        glProgramUniform2fv(
            _id,
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const float*>(values.data())
        );
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2fv(
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const float*>(values.data())
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const std::vector<glm::vec3>& values) const
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3fv.isResolved()) {
        glProgramUniform3fv(
            _id,
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const float*>(values.data())
        );
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3fv(
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const float*>(values.data())
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const std::vector<glm::vec4>& values) const
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4fv.isResolved()) {
        glProgramUniform4fv(
            _id,
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const float*>(values.data())
        );
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4fv(
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const float*>(values.data())
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLdouble value) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform1d.isResolved()) {
        glProgramUniform1d(_id, location, value);
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1d(location, value);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLdouble v1, GLdouble v2) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2d.isResolved()) {
        glProgramUniform2d(_id, location, v1, v2);
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2d(location, v1, v2);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLdouble v1, GLdouble v2,
                               GLdouble v3) const
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3d.isResolved()) {
        glProgramUniform3d(_id, location, v1, v2, v3);
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3d(location, v1, v2, v3);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, GLdouble v1, GLdouble v2, GLdouble v3,
                               GLdouble v4) const
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4d.isResolved()) {
        glProgramUniform4d(_id, location, v1, v2, v3, v4);
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4d(location, v1, v2, v3, v4);
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::dvec2& value) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2dv.isResolved()) {
        glProgramUniform2dv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2dv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::dvec3& value) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3dv.isResolved()) {
        glProgramUniform3dv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3dv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::dvec4& value) const {
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4dv.isResolved()) {
        glProgramUniform4dv(_id, location, 1, value_ptr(value));
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4dv(location, 1, value_ptr(value));
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const std::vector<GLdouble>& values) const
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform1dv.isResolved()) {
        glProgramUniform1dv(
            _id,
            location,
            static_cast<GLsizei>(values.size()),
            values.data()
        );
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform1dv(location, static_cast<GLsizei>(values.size()), values.data());
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location,
                               const std::vector<glm::dvec2>& values) const
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform2dv.isResolved()) {
        glProgramUniform2dv(
            _id,
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const double*>(values.data())
        );
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform2dv(
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const double*>(values.data())
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location,
                               const std::vector<glm::dvec3>& values) const
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform3dv.isResolved()) {
        glProgramUniform3dv(
            _id,
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const double*>(values.data())
        );
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform3dv(
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const double*>(values.data())
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location,
                               const std::vector<glm::dvec4>& values) const
{
    ghoul_assert(location != -1, "Location must not be -1");

    if (glbinding::Binding::ProgramUniform4dv.isResolved()) {
        glProgramUniform4dv(
            _id,
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const double*>(values.data())
        );
    }
    else {
        GLint oldProgram = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
        glUseProgram(_id);
        glUniform4dv(
            location,
            static_cast<GLsizei>(values.size()),
            reinterpret_cast<const double*>(values.data())
        );
        glUseProgram(static_cast<GLuint>(oldProgram));
    }
}

void ProgramObject::setUniform(GLint location, const glm::mat2x2& value,
                               Transpose transpose) const
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
        GLint oldProgram = 0;
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
                               Transpose transpose) const
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
        GLint oldProgram = 0;
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
                               Transpose transpose) const
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
        GLint oldProgram = 0;
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
                               Transpose transpose) const
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
        GLint oldProgram = 0;
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
                               Transpose transpose) const
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
        GLint oldProgram = 0;
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
                               Transpose transpose) const
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
        GLint oldProgram = 0;
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
                               Transpose transpose) const
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
        GLint oldProgram = 0;
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
                               Transpose transpose) const
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
        GLint oldProgram = 0;
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
                               Transpose transpose) const
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
        GLint oldProgram = 0;
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
                               Transpose transpose) const
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
        GLint oldProgram = 0;
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
                               Transpose transpose) const
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
        GLint oldProgram = 0;
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
                               Transpose transpose) const
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
        GLint oldProgram = 0;
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
                               Transpose transpose) const
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
        GLint oldProgram = 0;
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
                               Transpose transpose) const
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
        GLint oldProgram = 0;
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
                               Transpose transpose) const
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
        GLint oldProgram = 0;
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
                               Transpose transpose) const
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
        GLint oldProgram = 0;
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
                               Transpose transpose) const
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
        GLint oldProgram = 0;
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
                               Transpose transpose) const
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
        GLint oldProgram = 0;
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

bool ProgramObject::setSsboBinding(std::string_view name, GLuint binding) const {
    const GLuint index = glGetProgramResourceIndex(
        _id,
        GL_SHADER_STORAGE_BLOCK,
        name.data()
    );
    if (index == GL_INVALID_INDEX) {
        return false;
    }
    setSsboBinding(index, binding);
    return true;
}

void ProgramObject::setSsboBinding(GLuint index, GLuint binding) const {
    glShaderStorageBlockBinding(_id, index, binding);
}

GLuint ProgramObject::attributeLocation(const std::string& name) const {
    const GLint location = glGetAttribLocation(_id, name.data());
    if (!_ignoreAttributeLocationError && location == -1) {
        LWARNING(std::format("Failed to locate attribute location for '{}'", name));
        return GL_INVALID_INDEX;
    }
    return static_cast<GLuint>(location);
}

void ProgramObject::bindAttributeLocation(const std::string& name, GLuint index) const {
    glBindAttribLocation(_id, index, name.data());
}

bool ProgramObject::ignoreAttributeLocationError() const {
    return _ignoreAttributeLocationError;
}

void ProgramObject::setIgnoreAttributeLocationError(IgnoreError ignoreError) {
    _ignoreAttributeLocationError = ignoreError;
}

bool ProgramObject::setAttribute(const std::string& name, bool value) {
    ghoul_assert(!name.empty(), "Name must not be empty");

    const GLuint location = attributeLocation(name);
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
    if (transpose) {
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
    if (transpose) {
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
    if (transpose) {
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
    if (transpose) {
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
    if (transpose) {
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
    if (transpose) {
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
    if (transpose) {
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
    if (transpose) {
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
    if (transpose) {
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
    if (transpose) {
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
    if (transpose) {
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
    if (transpose) {
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
    if (transpose) {
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
    if (transpose) {
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
    if (transpose) {
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
    if (transpose) {
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
    if (transpose) {
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
    if (transpose) {
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
                                               const std::string& name) const
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

std::vector<std::string> ProgramObject::activeSubroutineUniformNames(
                                                ShaderObject::ShaderType shaderType) const
{
    GLint maximumUniformNameLength = 0;
    glGetProgramStageiv(
        _id,
        static_cast<GLenum>(shaderType),
        GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH,
        &maximumUniformNameLength
    );

    int countActiveSubroutineUniforms = 0;
    std::vector<char> buffer(maximumUniformNameLength);
    glGetProgramStageiv(
        _id,
        static_cast<GLenum>(shaderType),
        GL_ACTIVE_SUBROUTINE_UNIFORMS,
        &countActiveSubroutineUniforms
    );
    std::vector<std::string> result(countActiveSubroutineUniforms);
    for (GLuint i = 0; i < static_cast<GLuint>(countActiveSubroutineUniforms); i++) {
        glGetActiveSubroutineUniformName(
            _id,
            static_cast<GLenum>(shaderType),
            i,
            maximumUniformNameLength,
            nullptr,
            buffer.data()
        );
        result[i] = std::string(buffer.data());
    }

    return result;
}

std::vector<std::string> ProgramObject::compatibleSubroutineNames(
                                                      ShaderObject::ShaderType shaderType,
                                                   GLuint subroutineUniformLocation) const
{
    ghoul_assert(
        subroutineUniformLocation != GL_INVALID_INDEX,
        "Location must not be GL_INVALID_INDEX"
    );

    GLint maximumUniformNameLength = 0;
    glGetProgramStageiv(
        _id,
        static_cast<GLenum>(shaderType),
        GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH,
        &maximumUniformNameLength
    );

    GLint numCompatibleSubroutines = 0;
    glGetActiveSubroutineUniformiv(
        _id,
        static_cast<GLenum>(shaderType),
        subroutineUniformLocation,
        GL_NUM_COMPATIBLE_SUBROUTINES,
        &numCompatibleSubroutines
    );
    if (numCompatibleSubroutines == 0) {
        return std::vector<std::string>();
    }

    std::vector<std::string> result(numCompatibleSubroutines);
    std::vector<GLint> indices(numCompatibleSubroutines);
    std::vector<char> buffer(maximumUniformNameLength);
    glGetActiveSubroutineUniformiv(
        _id,
        static_cast<GLenum>(shaderType),
        subroutineUniformLocation,
        GL_COMPATIBLE_SUBROUTINES,
        indices.data()
    );
    for (GLuint i = 0; i < static_cast<GLuint>(numCompatibleSubroutines); i++) {
        glGetActiveSubroutineName(
            _id,
            static_cast<GLenum>(shaderType),
            static_cast<GLuint>(indices[i]),
            maximumUniformNameLength,
            nullptr,
            buffer.data()
        );
        result[i] = std::string(buffer.data());
    }
    return result;
}

std::vector<std::string> ProgramObject::compatibleSubroutineNames(
                                                      ShaderObject::ShaderType shaderType,
                                           const std::string& subroutineUniformName) const
{
    ghoul_assert(!subroutineUniformName.empty(), "Name must not be empty");

    const GLint index = subroutineUniformLocation(shaderType, subroutineUniformName);
    if (index == -1) {
        return std::vector<std::string>();
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
    int countActiveSubroutineUniforms = 0;
    glGetProgramStageiv(
        _id,
        static_cast<GLenum>(shaderType),
        GL_ACTIVE_SUBROUTINE_UNIFORMS,
        &countActiveSubroutineUniforms
    );
    if (static_cast<size_t>(countActiveSubroutineUniforms) != indices.size()) {
        LWARNING(std::format(
            "Number of active subroutine uniforms ({}) is different from passed uniform "
            "subroutine indices ({})",
            countActiveSubroutineUniforms, indices.size()
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
    int countActiveSubroutineUniforms = 0;
    glGetProgramStageiv(
        _id,
        static_cast<GLenum>(shaderType),
        GL_ACTIVE_SUBROUTINE_UNIFORMS,
        &countActiveSubroutineUniforms
    );
    if (static_cast<size_t>(countActiveSubroutineUniforms) != values.size()) {
        LWARNING(std::format(
            "Number of active subroutine uniforms ({}) is different from passed uniform "
            "subroutine indices ({})",
            countActiveSubroutineUniforms,
            values.size()
        ));
        return false;
    }
#endif // GHL_DEBUG

    std::vector<GLuint> uniformIndices = std::vector<GLuint>(values.size());
    const std::vector<std::string>& uniformSubroutines = activeSubroutineUniformNames(
        shaderType
    );
    for (size_t i = 0; i < uniformSubroutines.size(); i++) {
        const std::string& uniformSubroutine = uniformSubroutines[i];
        auto subroutine = values.find(uniformSubroutine);
#ifdef GHL_DEBUG
        if (subroutine == values.end()) {
            LWARNING(std::format(
                "Uniform subroutine name '{}' was not present in map", uniformSubroutine
            ));
            return false;
        }
#endif // GHL_DEBUG
        const std::string& nameSubroutine = subroutine->second;
        const GLuint idxSubroutine = subroutineIndex(shaderType, nameSubroutine);
#ifdef GHL_DEBUG
        if (idxSubroutine == GL_INVALID_INDEX) {
            LWARNING(std::format(
                "Subroutine name '{}' was not found in shader object", nameSubroutine
            ));
            return false;
        }
#endif // GHL_DEBUG
        uniformIndices[i] = idxSubroutine;
    }
    glUniformSubroutinesuiv(
        static_cast<GLenum>(shaderType),
        static_cast<GLsizei>(uniformIndices.size()),
        uniformIndices.data()
    );
    return true;
}

void ProgramObject::bindFragDataLocation(const std::string& name, GLuint colorNumber) {
    ghoul_assert(!name.empty(), "Name must not be empty");
    ghoul_assert(
        colorNumber != GL_INVALID_INDEX,
        "Location must not be GL_INVALID_INDEX"
    );

#ifdef GHL_DEBUG
    GLint maxBuffers = 0;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxBuffers);
    if (colorNumber >= static_cast<GLuint>(maxBuffers)) {
        LWARNING(std::format(
            "ColorNumber '{}' is bigger than the maximum of simultaneous outputs '{}'",
            colorNumber, maxBuffers
        ));
        return;
    }
#endif // GHL_DEBUG

    glBindFragDataLocation(_id, colorNumber, name.c_str());
}

} // namespace ghoul::opengl
