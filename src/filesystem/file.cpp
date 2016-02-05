/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2016                                                               *
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

#include <ghoul/filesystem/file.h>

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/logging/logmanager.h>

#include <format.h>

#ifdef WIN32
#include <windows.h>
#else
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#endif

using std::function;
using std::string;

namespace {
    const string _loggerCat = "File";
#ifdef WIN32
    const unsigned int changeBufferSize = 16384u;
    
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif
}

namespace ghoul {
namespace filesystem {

File::FileException::FileException(const std::string& msg)
    : RuntimeError(msg, "File")
{}

File::File(std::string filename, bool isRawPath, FileChangedCallback fileChangedCallback)
    : _fileChangedCallback(std::move(fileChangedCallback))
{
    ghoul_assert(!filename.empty(), "Filename must not be empty");
    
    if (isRawPath)
        _filename = std::move(filename);
    else
        _filename = std::move(FileSys.absolutePath(std::move(filename)));

    if (_fileChangedCallback)
        installFileChangeListener();
}

File::File(const File& cpy)
    : _filename(cpy._filename)
    , _fileChangedCallback(cpy._fileChangedCallback)
{
    if (_fileChangedCallback)
        installFileChangeListener();
}

File::~File() {
	if (_fileChangedCallback)
		removeFileChangeListener();
}

void File::setCallback(FileChangedCallback callback) {
	if (_fileChangedCallback)
        removeFileChangeListener();
    _fileChangedCallback = std::move(callback);
    if (_fileChangedCallback)
        installFileChangeListener();
}

const File::FileChangedCallback& File::callback() const {
    return _fileChangedCallback;
}

File::operator const std::string&() const {
    return _filename;
}

const std::string& File::path() const {
    return _filename;
}

std::string File::filename() const {
    string::size_type separator = _filename.rfind(FileSystem::PathSeparator);
    if (separator != string::npos)
        return _filename.substr(separator + 1);
    else
        return _filename;
}

string File::baseName() const {
    string fileName = filename();
    string::size_type dot = fileName.rfind(".");
    if (dot != string::npos)
        return fileName.substr(0, dot);
    else
        return fileName;
}

string File::fullBaseName() const {
    string::size_type dot = _filename.rfind(".");
    if (dot != string::npos)
        return _filename.substr(0, dot);
    else
        return _filename;
}

string File::directoryName() const {
    string::size_type separator = _filename.rfind(FileSystem::PathSeparator);
    if (separator != string::npos)
        return _filename.substr(0, separator);
    else
        return _filename;
}

string File::fileExtension() const {
    string::size_type dot = _filename.rfind(".");
    if (dot != string::npos)
        return _filename.substr(dot + 1);
    else
        return _filename;
}
    
std::string File::lastModifiedDate() const {
	if (!FileSys.fileExists(_filename)) {
        throw FileException(fmt::format(
            "Error retrieving last-modified date for file '{}'. File did not exist",
            _filename
        ));
	}
#ifdef WIN32
	WIN32_FILE_ATTRIBUTE_DATA infoData;
	BOOL success = GetFileAttributesEx(
		_filename.c_str(),
		GetFileExInfoStandard,
		&infoData
		);
	if (!success) {
		const DWORD error = GetLastError();
		LPTSTR errorBuffer = nullptr;
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&errorBuffer,
			0,
			NULL);
		if (errorBuffer != nullptr) {
			std::string error(errorBuffer);
            LocalFree(errorBuffer);
            throw FileException(fmt::format(
                "Could not retrieve last-modified date for file '{}': {}",
                _filename,
                error
            ));
		}
        throw FileException(fmt::format(
            "Could not retrieve last-modified date for file '{}'",
            _filename
        ));
	}
	else {
		FILETIME lastWriteTime = infoData.ftLastWriteTime;
		SYSTEMTIME time;
		//LPSYSTEMTIME time = NULL;
		BOOL success = FileTimeToSystemTime(&lastWriteTime, &time);
		if (!success) {
			const DWORD error = GetLastError();
			LPTSTR errorBuffer = nullptr;
			DWORD nValues = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				error,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&errorBuffer,
				0,
				NULL);
			if ((nValues > 0) && (errorBuffer != nullptr)) {
				std::string error(errorBuffer);
                LocalFree(errorBuffer);
                throw FileException(fmt::format(
                    "'FileTimeToSystemTime' failed for file '{}': {}",
                    _filename,
                    error
                ));
			}
            throw FileException(fmt::format(
                "'FileTimeToSystemTime' failed for file '{}'",
                _filename
            ));
		}
		else {
			return std::to_string(time.wYear) + "-" + std::to_string(time.wMonth) + "-" +
				std::to_string(time.wDay) + "T" + std::to_string(time.wHour) + ":" +
				std::to_string(time.wMinute) + ":" + std::to_string(time.wSecond) + "." +
				std::to_string(time.wMilliseconds);
		}
	}
#else
	struct stat attrib;
	stat(_filename.c_str(), &attrib);
	struct tm* time = gmtime(&(attrib.st_ctime));
	char buffer[128];
	strftime(buffer, 128, "%Y-%m-%dT%H:%M:%S", time);
	return buffer;
#endif
}


void File::installFileChangeListener() {
	FileSys.addFileListener(this);
}

void File::removeFileChangeListener() {
	FileSys.removeFileListener(this);
}

std::ostream& operator<<(std::ostream& os, const File& f) {
    return os << f.path();
}

} // namespace filesystem
} // namespace ghoul
