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

std::string glslVersionString() {
	int versionMajor;
	int versionMinor;
	int profileMask;
	glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
	glGetIntegerv(GL_MINOR_VERSION, &versionMinor);
	glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profileMask);
	std::stringstream ss;
	ss << "#version " << versionMajor << versionMinor << "0" << // version is set
		(profileMask & GL_CONTEXT_CORE_PROFILE_BIT ? " core" :
		(profileMask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT ? " compatibility" : ""));
	return ss.str();
}
}

namespace ghoul {
namespace opengl {

std::vector<std::string> ShaderObject::_includePaths = std::vector<std::string>();

ShaderObject::ShaderObject(ShaderType shaderType)
    : _id(0)
    , _type(shaderType)
    , _fileName("")
    , _shaderName("")
	, _loggerCat("ShaderObject")
	, _onChangeCallback(nullptr)
{
    _id = glCreateShader(_type);
    if (_id == 0)
        LERROR("glCreateShader returned 0");
}

ShaderObject::ShaderObject(ShaderType shaderType, std::string filename)
    : _id(0)
    , _type(shaderType)
    , _fileName(std::move(filename))
    , _shaderName("")
	, _loggerCat("ShaderObject")
	, _onChangeCallback(nullptr)
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
						   std::string name)
    : _id(0)
    , _type(shaderType)
    , _fileName(std::move(filename))
    , _shaderName(std::move(name))
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
	for (ghoul::filesystem::File* f : _trackedFiles)
		delete f;
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

void ShaderObject::setShaderObjectCallback(ShaderObjectCallback changeCallback) {
	_onChangeCallback = changeCallback;

	for (ghoul::filesystem::File* fileObject : _trackedFiles) {
		fileObject->setCallback(_onChangeCallback);
	}
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
	for (ghoul::filesystem::File* f : _trackedFiles)
		delete f;
	_trackedFiles.erase(_trackedFiles.begin(), _trackedFiles.end());

	// No need to allocate a new contents string for every shader
	// This makes ShaderObjects not thread safe
	static std::string contents;
	contents = "";

	bool success = readFile(_fileName, contents);
	
	// If in debug mode, output the source to file
#ifndef NDEBUG
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
#endif
	
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

		std::string filedefs = "";
		size_t filehash = 0;
		for (ghoul::filesystem::File* file : _trackedFiles) {
			const std::string& path = file->path();
			filedefs += std::to_string(filehash) + ": " + path + "\n";
			++filehash;
		}

        GLchar* log = new GLchar[logLength];
        glGetShaderInfoLog(_id, logLength, NULL, log);
		LERROR(typeAsString() << " compile error:\n" << log << std::endl << filedefs);

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
	for (const auto f : _trackedFiles) {
		if (f->path() == filename) {
			LWARNING("Already including '" << filename << "'");
			return false;
		}
	}

	// Construct a file object, track the current file 
	// even if we cannot open the file
	using namespace ghoul::filesystem;
	File* fileObject = new File(filename, false);
	int fileHash = -1;
	if (track) {
		fileObject->setCallback(_onChangeCallback);
		fileHash = static_cast<int>(_trackedFiles.size());
		_trackedFiles.push_back( fileObject);
	}

	// Check that file can be opened
	std::ifstream f(filename, std::ifstream::in);
	if (!f.good()) {
		LWARNING("Could not open '" << filename << "': " << strerror(errno));
		f.close();
		return false;
	}

	// Ready to start parsing
	// ugly slightly more efficient version, 3-4x faster
	unsigned int lineNumber = 0;
	static std::string line;
	static const std::string ws = " \n\r\t";
	static const std::string includeString = "#include";
	static const std::string notrackString = ":notrack";
	static const std::string versionString = "#version __CONTEXT__";
    
#ifdef __APPLE__
    static const std::string includeSeparator = "";
#else
    static const std::string includeSeparator = ";";
#endif
    


	auto addLineDef = [&lineNumber, &fileHash](std::string* content) {
		*content += "#line " + std::to_string(lineNumber) + " " + std::to_string(fileHash) + "\n";
	};
	while (std::getline(f, line)) {
		++lineNumber;
		size_t start_pos = line.find_first_not_of(ws);
		if (start_pos == std::string::npos)
			start_pos = 0;
		size_t end_pos = line.find_last_not_of(ws);
		if (end_pos == std::string::npos)
			end_pos = line.length();
		else
			end_pos += 1;

		const size_t length = end_pos - start_pos;

		// If begins with #include
		if (length > 11 && line.substr(start_pos, includeString.length()) == includeString) {
			bool success = false;

			size_t p1 = line.find_first_not_of(ws, start_pos + includeString.length());
			size_t p2 = std::string::npos;
			if (p1 != std::string::npos) {
				if (line.at(p1) == '\"') {
					p2 = line.find_first_of("\"", p1 + 1);
					if (p2 != std::string::npos) {
						std::string includeFilename = fileObject->directoryName() + FileSystem::PathSeparator + line.substr(p1 + 1, p2 - p1 - 1);

						size_t remaining = end_pos - p2;
						bool keepTrack = true;
						if (remaining > notrackString.length()) {
							if (line.find_first_of(notrackString, p2) != std::string::npos)
								keepTrack = false;
						}

						bool includeFileWasFound = FileSys.fileExists(includeFilename);

						// Resolve the include paths if this default includeFilename does
						// not exist
						if (!includeFileWasFound) {
							for (const std::string& includePath : _includePaths) {
								includeFilename = includePath + FileSystem::PathSeparator + line.substr(p1 + 1, p2 - p1 - 1);

								if (FileSys.fileExists(includeFilename)) {
									includeFileWasFound = true;
									break;
								}
							}
						}

						if (includeFileWasFound) {
							// The ; are in the code for forcing compiler errors to occur
							// in the correct file. Otherwise, they would leak from the
							// included file into the source file (and vice versa)
							content += includeSeparator + "// Begin including '" + includeFilename + "'\n";
							if (!readFile(includeFilename, content, track && keepTrack))
								content += "// Warning, unsuccessful loading of '" + includeFilename + "'\n";
							content += includeSeparator + "// End including '" + includeFilename + "'\n";
						}
						else {
							LERROR("Could not resolve file path for include file '" <<
								line.substr(p1 + 1, p2 - p1 - 1) << "'");

							content += includeSeparator + "// Error including file '" + line.substr(p1 + 1, p2 - p1 - 1) + "'\n";
						}
						addLineDef(&content);
						success = true;
					}
				}
				else if (line.at(p1) == '<') {
					p2 = line.find_first_of(">", p1 + 1);
					if (p2 != std::string::npos) {
						std::string includeFilename = absPath(line.substr(p1 + 1, p2 - p1 - 1));

						size_t remaining = end_pos - p2;
						bool keepTrack = true;
						if (remaining > notrackString.length()) {
							if (line.find_first_of(notrackString, p2) != std::string::npos)
								keepTrack = false;
						}

						content += includeSeparator + "// Begin including '" + includeFilename + "'\n";
						if (!readFile(includeFilename, content, track && keepTrack))
							content += "// Warning, unsuccessful loading of '" + includeFilename + "'\n";
						content += includeSeparator + "// End including '" + includeFilename + "'\n";
						lineNumber += 1;
						addLineDef(&content);
						success = true;
					}
				}
			}
			if (!success) {
				content += includeSeparator + "// Error in #include pattern!\n";
			}
		}
		else if (line == versionString) {
			static std::string versionString = glslVersionString();
			content += versionString + "\n";
		}
		else{
			content += line + "\n";
		}
	}

	if (!track) {
		delete fileObject;
	}

	f.close();
	return true;
}

bool ShaderObject::addIncludePath(std::string folderPath) {
	folderPath = absPath(folderPath);

	// We only want unique values in this list
	if (std::find(_includePaths.begin(), _includePaths.end(), folderPath)
			!= _includePaths.end())
	{
		LERRORC("ShaderObject", 
			"Include path '" << folderPath << "' was already registered");
		return false;
	}

	// We only want valid folders in this folder
	if (!FileSys.directoryExists(folderPath)) {
		LERRORC("ShaderObject",
			"Include path '" << folderPath << "' is not a valid folder");
		return false;
	}

	// If we managed to get to this place, we have a valid directory
	_includePaths.push_back(std::move(folderPath));
	return true;
}

} // namespace opengl
} // namespace ghoul

#ifdef _MSC_VER
#pragma warning(pop)
#endif
