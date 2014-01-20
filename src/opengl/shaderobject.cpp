/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012 Alexander Bock                                                     *
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

#include <ghoul/logging/log.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996)
#endif

namespace ghoul {
namespace opengl {

ShaderObject::ShaderObject(ShaderType shaderType)
    : _id(0)
    , _type(shaderType)
    , _fileName("")
    , _shaderName("")
    , _loggerCat("ShaderObject")
{
    _id = glCreateShader(_type);
    if (_id == 0)
        LERROR("glCreateShader returned 0");
}

ShaderObject::ShaderObject(ShaderType shaderType, const std::string& filename) 
    : _id(0)
    , _type(shaderType)
    , _fileName(filename)
    , _shaderName("")
    , _loggerCat("ShaderObject")
{
#ifdef DEBUG
    if (filename == "")
        LWARNING("Filename is empty");
#endif

    _id = glCreateShader(_type);
    if (_id == 0)
        LERROR("glCreateShader returned 0");
    setShaderFilename(filename);
}

ShaderObject::ShaderObject(ShaderType shaderType, const std::string& filename,
                           const std::string& name)
    : _id(0)
    , _type(shaderType)
    , _fileName(filename)
    , _shaderName(name)
    , _loggerCat("ShaderObject('" + name + "')")
{
    _id = glCreateShader(_type);
    if (_id == 0)
        LERROR("glCreateShader returned 0");
#ifdef GL_VERSION_4_3
    if (glObjectLabel)
        glObjectLabel(GL_SHADER, _id, GLsizei(_shaderName.length() + 1),
                      _shaderName.c_str());
#endif
    setShaderFilename(_fileName);
}

ShaderObject::ShaderObject(const ShaderObject& cpy)
    : _id(0)
    , _type(cpy._type)
    , _fileName(cpy._fileName)
    , _shaderName(cpy._shaderName)
    , _loggerCat(cpy._loggerCat)
{
    _id = glCreateShader(_type);
    if (_id == 0)
        LERROR("glCreateShader returned 0");
#ifdef GL_VERSION_4_3
    if (glObjectLabel)
        glObjectLabel(GL_SHADER, _id, GLsizei(_shaderName.length() + 1),
                      _shaderName.c_str());
#endif
    setShaderFilename(_fileName);
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
        _fileName = rhs._fileName;
        _shaderName = rhs._shaderName;
        _loggerCat = rhs._loggerCat;

        glDeleteShader(_id);
        _id = glCreateShader(_type);
        if (_id == 0)
            LERROR("glCreateShader returned 0");
#ifdef GL_VERSION_4_3
        if (glObjectLabel)
            glObjectLabel(GL_SHADER, _id, GLsizei(_shaderName.length() + 1)
                          , _shaderName.c_str());
#endif
        setShaderFilename(_fileName);
    }
    return *this;
}

void ShaderObject::setName(const std::string& name) {
    _shaderName = name;
    _loggerCat = "ShaderObject['" + name + "']";
#ifdef GL_VERSION_4_3
    if (glObjectLabel)
        glObjectLabel(GL_SHADER, _id, GLsizei(_shaderName.length() + 1),
_shaderName.c_str());
#endif
}

const std::string& ShaderObject::name() const {
    return _shaderName;
}

bool ShaderObject::hasName() const {
    return !_shaderName.empty();
}

bool ShaderObject::setShaderFilename(const std::string& filename) {
    if (filename == "") {
        deleteShader();
        return true;
    }
    
    std::ifstream shaderFile(filename.c_str());

    // Can the file be opened?
    if (!shaderFile.is_open()) {
        LERROR("Could not open " + typeAsString() + " file: " + filename);
        return false;
    }
            
    // Make sure the file is not empty
    shaderFile.seekg(0, std::ios_base::end);
    std::streamoff fileLength = shaderFile.tellg();
    shaderFile.seekg(0, std::ios_base::beg);

    if (fileLength == 0) {
        LERROR("Could not load " + typeAsString()
        + " file '" + filename + "': File is empty");
        return false;
    }

    //Read shader source line by line
    std::vector<const char*> shaderSource;
    while (shaderFile.good()) {
        std::string currentLine;
        std::getline(shaderFile, currentLine);

        char* cstr = new char[currentLine.size() + 2]; // +1 for \0 and +1 for \n
	    std::strcpy(cstr, (currentLine + "\n").c_str());
        shaderSource.push_back(cstr);
    }
    shaderFile.close();

    glShaderSource(_id, GLsizei(shaderSource.size()), &shaderSource[0], NULL);

    for (size_t i = 0; i < shaderSource.size(); ++i)
        delete[] shaderSource[i];

    LINFO("Loaded " + typeAsString() + ": '" + filename + "'");
    return true;
}

void ShaderObject::deleteShader() {
    glDeleteShader(_id);
    _id = 0;
}

bool ShaderObject::rebuildFromFile() {
    return setShaderFilename(_fileName);
}

bool ShaderObject::compile() {
    glCompileShader(_id);

    GLint compilationStatus;
    glGetShaderiv(_id, GL_COMPILE_STATUS, &compilationStatus);
    if (compilationStatus == GL_FALSE) {
        GLint logLength;
        glGetShaderiv(_id, GL_INFO_LOG_LENGTH, &logLength);

        if (logLength == 0) {
            LERROR(typeAsString() + " compile error: Unknown error");
            return false;
        }

        GLchar* log = new GLchar[logLength];
        glGetShaderInfoLog(_id, logLength, NULL, log);
        LERROR(typeAsString() + " compile error:\n" + log);
        delete[] log;

        return false;
    }
    return compilationStatus == GL_TRUE;
}

std::string ShaderObject::typeAsString() const {
    return ShaderObject::stringForShaderType(_type);
}

std::string ShaderObject::stringForShaderType(ShaderType type) {
    switch (type) {
    case ShaderTypeVertex:
        return "Vertex shader";
    case ShaderTypeTesselationControl:
        return "Tesselation Control shader";
    case ShaderTypeTesselationEvaluation:
        return "Tesselation Evaluation shader";
    case ShaderTypeGeometry:
        return "Geometry shader";
    case ShaderTypeFragment:
        return "Fragment shader";
#ifdef GL_VERSION_4_3
    case ShaderTypeCompute:
        return "Compute shader";
#endif
    default:
        assert(false);
        return "";
    }
}


} // namespace opengl
} // namespace ghoul

#ifdef _MSC_VER
#pragma warning(pop)
#endif
