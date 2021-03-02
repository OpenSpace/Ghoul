/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2021                                                               *
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

#include <ghoul/fmt.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/misc/assert.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#include <windows.h>
#else // ^^^^ WIN32 // !WIN32 vvvv
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#endif // WIN32

using std::string;

namespace {
#ifdef WIN32
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // _CRT_SECURE_NO_WARNINGS
#endif // WIN32
} // namespace

namespace ghoul::filesystem {

File::FileException::FileException(std::string msg)
    : RuntimeError(std::move(msg), "File")
{}

File::File(string filename, RawPath isRawPath, FileChangedCallback fileChangedCallback)
    : _fileChangedCallback(std::move(fileChangedCallback))
{
    ghoul_assert(!filename.empty(), "Filename must not be empty");

    _filename = isRawPath ?
        std::move(filename) :
        FileSys.absolutePath(std::move(filename));

    if (_fileChangedCallback) {
        installFileChangeListener();
    }
}

File::File(const File& file)
    : _filename(file._filename)
    , _fileChangedCallback(file._fileChangedCallback)
{
    if (_fileChangedCallback) {
        installFileChangeListener();
    }
}

File::~File() {
    if (_fileChangedCallback) {
        removeFileChangeListener();
    }
}

void File::setCallback(FileChangedCallback callback) {
    if (_fileChangedCallback) {
        removeFileChangeListener();
    }
    _fileChangedCallback = std::move(callback);
    if (_fileChangedCallback) {
        installFileChangeListener();
    }
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
    const string::size_type separator = _filename.rfind(FileSystem::PathSeparator);
    if (separator != string::npos) {
        return _filename.substr(separator + 1);
    }
    else {
        return _filename;
    }
}

string File::baseName() const {
    string fileName = filename();
    const string::size_type dot = fileName.rfind('.');
    if (dot != string::npos) {
        return fileName.substr(0, dot);
    }
    else {
        return fileName;
    }
}

string File::fullBaseName() const {
    const string::size_type dot = _filename.rfind('.');
    if (dot != string::npos) {
        return _filename.substr(0, dot);
    }
    else {
        return _filename;
    }
}

string File::directoryName() const {
    const string::size_type separator = _filename.rfind(FileSystem::PathSeparator);
    if (separator != string::npos) {
        return _filename.substr(0, separator);
    }
    else {
        return _filename;
    }
}

string File::fileExtension() const {
    const string::size_type dot = _filename.rfind('.');
    if (dot != string::npos) {
        return _filename.substr(dot + 1);
    }
    else {
        return _filename;
    }
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
        FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPTSTR>(&errorBuffer), // NOLINT
            0,
            nullptr
        );
        std::string msg(errorBuffer);
        LocalFree(errorBuffer);
        throw FileException(fmt::format(
            "Could not retrieve last-modified date for file '{}': {}", _filename, msg
        ));
    }
    else {
        FILETIME lastWriteTime = infoData.ftLastWriteTime;
        SYSTEMTIME time;
        //LPSYSTEMTIME time = NULL;
        success = FileTimeToSystemTime(&lastWriteTime, &time);
        if (!success) {
            const DWORD error = GetLastError();
            LPTSTR errorBuffer = nullptr;
            FormatMessage(
                FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                error,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                reinterpret_cast<LPTSTR>(&errorBuffer), // NOLINT
                0,
                nullptr
            );
            std::string msg(errorBuffer);
            LocalFree(errorBuffer);
            throw FileException(fmt::format(
                "'FileTimeToSystemTime' failed for file '{}': {}", _filename, msg
            ));
        }
        else {
            return fmt::format(
                "{}-{}-{}T{}:{}:{}.{}", time.wYear, time.wMonth, time.wDay, time.wHour,
                time.wMinute, time.wSecond, time.wMilliseconds
            );
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

} // namespace ghoul::filesystem
