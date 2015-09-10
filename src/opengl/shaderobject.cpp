/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2015                                                               *
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
#include <ghoul/filesystem/filesystem>
#include <ghoul/filesystem/cachemanager.h>
#include <ghoul/filesystem/file.h>
#include <ghoul/misc/crc32.h>
#include <ghoul/systemcapabilities/systemcapabilities.h>

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstring>
#include <fstream>
#include <functional>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996)
#endif

namespace {
const std::string _loggerCat = "ShaderObject";
}

namespace ghoul {
namespace opengl {

ShaderObject::ShaderObject(ShaderType shaderType, Dictionary dictionary)
    : _id(0)
    , _type(shaderType)
    , _fileName("")
    , _shaderName("")
    , _dictionary(dictionary)
    , _loggerCat("ShaderObject")
    , _onChangeCallback(nullptr)
{
    _id = glCreateShader(_type);
    _preprocessor.setDictionary(dictionary);
    if (_id == 0)
        LERROR("glCreateShader returned 0");
}

ShaderObject::ShaderObject(ShaderType shaderType, std::string filename, Dictionary dictionary)
    : _id(0)
    , _type(shaderType)
    , _fileName(std::move(filename))
    , _shaderName("")
    , _dictionary(dictionary)
    , _loggerCat("ShaderObject")
    , _onChangeCallback(nullptr)
{
#ifdef DEBUG
    if (filename == "")
        LWARNING("Filename is empty");
#endif

    _id = glCreateShader(_type);
    _preprocessor.setDictionary(dictionary);
    if (_id == 0)
        LERROR("glCreateShader returned 0");
    setShaderFilename(_fileName);
}

ShaderObject::ShaderObject(ShaderType shaderType, std::string filename,
                           std::string name, Dictionary dictionary)
    : _id(0)
    , _type(shaderType)
    , _fileName(std::move(filename))
    , _shaderName(std::move(name))
    , _dictionary(dictionary)
    , _loggerCat("ShaderObject('" + _shaderName + "')")
    , _onChangeCallback(nullptr)
{
    _id = glCreateShader(_type);
    if (_id == 0)
        LERROR("glCreateShader returned 0");
#ifdef GL_VERSION_4_3
    if (glObjectLabel)
        glObjectLabel(GL_SHADER, _id, GLsizei(_shaderName.length() + 1),
                      _shaderName.c_str());
#endif
    _preprocessor.setDictionary(dictionary);
    setShaderFilename(_fileName);
}

ShaderObject::ShaderObject(const ShaderObject& cpy)
    : _id(0)
    , _type(cpy._type)
    , _fileName(cpy._fileName)
    , _shaderName(cpy._shaderName)
    , _dictionary(cpy._dictionary)
    , _loggerCat(cpy._loggerCat)
    , _onChangeCallback(cpy._onChangeCallback)
    , _preprocessor(cpy._preprocessor)
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
    setShaderObjectCallback(_onChangeCallback);
}

ShaderObject::ShaderObject(ShaderObject&& rhs) {
	if (this != &rhs) {
		glDeleteShader(_id);
		_id = rhs._id;
		rhs._id = 0;

		_type = rhs._type;
		_fileName = std::move(rhs._fileName);
		_shaderName = std::move(rhs._shaderName);
		_dictionary = std::move(rhs._dictionary);
		_loggerCat = std::move(rhs._loggerCat);
		_onChangeCallback = rhs._onChangeCallback;
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
        _fileName = rhs._fileName;
        _shaderName = rhs._shaderName;
        _dictionary = rhs._dictionary;
        _loggerCat = rhs._loggerCat;
        _onChangeCallback = rhs._onChangeCallback;
        _preprocessor = rhs._preprocessor;
        setShaderObjectCallback(rhs._onChangeCallback);

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

ShaderObject& ShaderObject::operator=(ShaderObject&& rhs) {
	if (this != &rhs) {
		glDeleteShader(_id);
		_id = rhs._id;
		rhs._id = 0;

		_type = rhs._type;
		_fileName = std::move(rhs._fileName);
		_shaderName = std::move(rhs._shaderName);
		_dictionary = std::move(rhs._dictionary);
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
	if (glObjectLabel)
		glObjectLabel(GL_SHADER, _id, GLsizei(_shaderName.length() + 1),
		_shaderName.c_str());
#endif
}

const std::string& ShaderObject::name() const {
    return _shaderName;
}

void ShaderObject::setDictionary(Dictionary dictionary) {
    _dictionary = dictionary;
    _preprocessor.setDictionary(dictionary);
    rebuildFromFile();
}

void ShaderObject::setShaderObjectCallback(ShaderObjectCallback changeCallback) {
    _onChangeCallback = changeCallback;
    _preprocessor.setCallback([this](const filesystem::File& file) {
	if (_onChangeCallback) {
	    _onChangeCallback(file);
	}
    });
}

bool ShaderObject::hasName() const {
    return !_shaderName.empty();
}

std::string ShaderObject::filename() {
	return _fileName;
}

bool ShaderObject::setShaderFilename(std::string filename) {
	_fileName = std::move(filename);
    if (_fileName == "") {
        deleteShader();
        return true;
    }

    if (!FileSys.fileExists(_fileName)) {
        LERROR("Filename '" << absPath(_fileName) << "' did not exist");
        deleteShader();
        return false;
    }

    // No need to allocate a new contents string for every shader
    // This makes ShaderObjects not thread safe
    static std::string contents;
    contents = "";

    bool success;
    _preprocessor.setShaderPath(_fileName);
    success = _preprocessor.process(contents);



	// If in debug mode, output the source to file
//#ifndef NDEBUG
	std::string generatedFilename;
	ghoul::filesystem::File ghlFile(_fileName);
	if (!FileSys.cacheManager() || 
		!FileSys.cacheManager()->getCachedFile(
			// we use the .baseName() version because otherwise we get a new file 
			// every time we reload the shader
			ghlFile.baseName(), 
			"",
			generatedFilename,
			true)
		)
	{
		// Either the cachemanager wasn't initialized or the
		// filename could not be fetched
		generatedFilename += ".GhoulGenerated.glsl";
	}
	
	std::ofstream os(generatedFilename);
	os << contents;
	os.close();
//#endif
	
	if (!success)
		return false;

    const char* contentPtr =  contents.c_str();
    glShaderSource(_id, 1, &contentPtr, NULL);

    LINFO("Loaded " + typeAsString() + ": '" + _fileName + "'");
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
        LERROR(typeAsString() << " compile error:\n" << log << std::endl << _preprocessor.getFileIdentifiersString());

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

/**
 * Add include path. Deprecated.
 * Use ShaderPreprocessor::addIncludePath(std::string folderPath) instead.
 */
bool ShaderObject::addIncludePath(std::string folderPath) {
    return ShaderPreprocessor::addIncludePath(folderPath);
}



} // namespace opengl
} // namespace ghoul

#ifdef _MSC_VER
#pragma warning(pop)
#endif
