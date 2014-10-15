/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2014                                                               *
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

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstring>
#include <fstream>
#include <functional>
#include <regex>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996)
#endif

namespace {
const std::string _loggerCat = "ShaderObject";
}

namespace ghoul {
namespace opengl {

ShaderObject::ShaderObject(ShaderType shaderType, ShaderObjectCallback changeCallback)
    : _id(0)
    , _type(shaderType)
    , _fileName("")
    , _shaderName("")
	, _loggerCat("ShaderObject")
	, _onChangeCallback(changeCallback)
{
    _id = glCreateShader(_type);
    if (_id == 0)
        LERROR("glCreateShader returned 0");
}

ShaderObject::ShaderObject(ShaderType shaderType, std::string filename, ShaderObjectCallback changeCallback)
    : _id(0)
    , _type(shaderType)
    , _fileName(std::move(filename))
    , _shaderName("")
	, _loggerCat("ShaderObject")
	, _onChangeCallback(changeCallback)
{
#ifdef DEBUG
    if (filename == "")
        LWARNING("Filename is empty");
#endif

    _id = glCreateShader(_type);
    if (_id == 0)
        LERROR("glCreateShader returned 0");
    setShaderFilename(_fileName);
}

ShaderObject::ShaderObject(ShaderType shaderType, std::string filename,
						   std::string name, ShaderObjectCallback changeCallback)
    : _id(0)
    , _type(shaderType)
    , _fileName(std::move(filename))
    , _shaderName(std::move(name))
	, _loggerCat("ShaderObject('" + _shaderName + "')")
	, _onChangeCallback(changeCallback)
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
	, _onChangeCallback(cpy._onChangeCallback)
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

ShaderObject::ShaderObject(ShaderObject&& rhs) {
	if (this != &rhs) {
		glDeleteShader(_id);
		_id = rhs._id;
		rhs._id = 0;

		_type = rhs._type;
		_fileName = std::move(rhs._fileName);
		_shaderName = std::move(rhs._shaderName);
		_loggerCat = std::move(rhs._loggerCat);
		_onChangeCallback = rhs._onChangeCallback;
		_trackedFiles = std::move(rhs._trackedFiles);

	}
}

ShaderObject::~ShaderObject() {
    glDeleteShader(_id);
	_id = 0;
	for (auto f : _trackedFiles)
		delete f.second;
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
		_onChangeCallback = rhs._onChangeCallback;

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
		_loggerCat = std::move(rhs._loggerCat);
		_onChangeCallback = rhs._onChangeCallback;
		_trackedFiles = std::move(rhs._trackedFiles);
	
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

	// Clear tracked files, this might have changed since last time
	for (auto f : _trackedFiles)
		delete f.second;
	_trackedFiles.erase(_trackedFiles.begin(), _trackedFiles.end());

	std::string contents;
	contents.reserve(8192);

	bool success = readFile(_fileName, contents);
	// TODO: Some other solution for outputting the source as human readable for debugging
	std::ofstream os(_fileName + ".OpenSpaceGenerated.glsl");
	os << contents;
	os.close();

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

bool ShaderObject::readFile(const std::string& filename, std::string& content, bool track) {
	// check that the file exists
	if (!FileSys.fileExists(filename)) {
		LWARNING("Could not find '" << filename << "'");
		return false;
	}

	// Check that the file is currently not in _trackedFiles
	auto s = _trackedFiles.find(filename);
	if (s != _trackedFiles.end()) {
		LWARNING("Already including '" << filename << "'");
		return false;
	}

	// Check that file can be opened
	std::ifstream f(filename, std::ifstream::in);
	if (!f.is_open()) {
		LWARNING("Could not open '" << filename << "': " << strerror(errno));
		return false;
	}

	// Construct a file object
	using namespace ghoul::filesystem;
	
	// CODE FOR DEBUG PURPOSE
	//filesystem::File::FileChangedCallback c = nullptr;
	//c = [ this](const filesystem::File&){
	//	LDEBUG("Update fired from " << name());
	//	if (_onChangeCallback)
	//		_onChangeCallback;
	//};
	
	File* fileObject = new File(filename, false);
	if (track) {
		fileObject->setCallback(_onChangeCallback);
		_trackedFiles[filename] = fileObject;
	}

	// Ready to start parsing
	std::string line;
	std::smatch m;
	std::regex e1(R"(^\s*#include \"(.+)\"\s*(:notrack)?\s*)");	// Regex to match relative paths
	std::regex e2(R"(^\s*#include <(.+)>\s*(:notrack)?\s*)");		// Regex to match ghoul absPath
	while (std::getline(f, line)) {
		if (std::regex_search(line, m, e1)) {
			std::string includeFilename = fileObject->directoryName() + FileSystem::PathSeparator + std::string(m[1]);
			content += "// Begin including '" + includeFilename + "'\n";
			if (!readFile(includeFilename, content, track && m[2] == ""))
				content += "// Warning, unsuccessful loading of '" + includeFilename + "'\n";
			content += "// End including '" + includeFilename + "'\n";
		}
		else if (std::regex_search(line, m, e2)) {
			std::string includeFilename = absPath(m[1]);
			content += "// Including '" + includeFilename + "'\n";
			if (!readFile(includeFilename, content, track && m[2] == ""))
				content += "// Warning, unsuccessful loading of '" + includeFilename + "'\n";
			content += "// End including '" + includeFilename + "'\n";
		}
		else {
			content += line + "\n";
		}
	}
	if (!track) {
		delete fileObject;
	}

	f.close();
	return true;
}

} // namespace opengl
} // namespace ghoul

#ifdef _MSC_VER
#pragma warning(pop)
#endif
