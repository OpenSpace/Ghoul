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

#include <ghoul/filesystem/file.h>

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/logging/logmanager.h>

using std::function;
using std::string;



namespace ghoul {
namespace filesystem {

namespace {
const string _loggerCat = "File";
#ifdef WIN32
const char pathSeparator = '\\';
const unsigned int changeBufferSize = 16384u;

#define _CRT_SECURE_NO_WARNINGS
#elif __APPLE__
const char pathSeparator = '/';
#else
const char pathSeparator = '/';
#endif
}

File::File(std::string filename, bool isRawPath,
           FileChangedCallback fileChangedCallback)
    : _fileChangedCallback(std::move(fileChangedCallback))
{
    if (isRawPath)
        _filename = std::move(filename);
    else
        _filename = std::move(FileSys.absolutePath(std::move(filename)));

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
    string::size_type separator = _filename.rfind(pathSeparator);
    if (separator != string::npos)
        return _filename.substr(separator + 1);
    else
        return _filename;
}

string File::baseName() const {
    string&& fileName = filename();
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
    string::size_type separator = _filename.rfind(pathSeparator);
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
