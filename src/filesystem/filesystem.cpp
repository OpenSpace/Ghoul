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

#include <ghoul/filesystem/filesystem.h>

#include <ghoul/logging/logmanager.h>

#include <algorithm>
#include <cassert>
#include <regex>
#include <stdio.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <dirent.h>
#include <unistd.h>
#include <sys/param.h>
#endif

using std::string;
#define GHL_DEBUG
namespace {
    const string _loggerCat = "FileSystem";

    const string _tokenOpeningBraces = "${";
    const string _tokenClosingBraces = "}";

#ifdef _WIN32
    const char pathSeparator = '\\';
#else
    const char pathSeparator = '/';
#endif
}

namespace ghoul {
namespace filesystem {

FileSystem* FileSystem::_fileSystem = nullptr;

FileSystem::FileSystem() {}

void FileSystem::initialize() {
    assert(_fileSystem == nullptr);
    if (_fileSystem == nullptr)
        _fileSystem = new FileSystem;
}

void FileSystem::deinitialize() {
    assert(_fileSystem != nullptr);
    delete _fileSystem;
}

FileSystem& FileSystem::ref() {
    assert(_fileSystem != nullptr);
    return *_fileSystem;
}

string FileSystem::absolutePath(const string& path) const {
    string result = path;
    expandPathTokens(result);

    char* buffer = nullptr;
#ifdef _WIN32
    static const int PATH_BUFFER_SIZE = 4096;
    buffer = new char[PATH_BUFFER_SIZE];
    const DWORD success = GetFullPathName(result.c_str(), PATH_BUFFER_SIZE, buffer, 0);
    if (success == 0) {
        delete[] buffer;
        buffer = nullptr;
    }
#else
    buffer = realpath(result.c_str(), 0);
#endif

    if (buffer) {
        result = string(buffer);
        delete[] buffer;
        return result;
    }

    return result;
}

string FileSystem::relativePath(const string& path,
                                const Directory& baseDirectory) const
{
    if (path.empty()) {
        LERROR_SAFE("'path' must contain a path");
        return path;
    }
    const string& pathAbsolute = cleanupPath(absolutePath(path));
    const string& directoryAbsolute = cleanupPath(absolutePath(baseDirectory));

    // Return identity path if absolutes are equal
    if (pathAbsolute == directoryAbsolute)
        return ".";

    // Check for different drives on Windows
    if (pathAbsolute[0] != directoryAbsolute[0])
        return path;

    // Find the common part in the 'path' and 'baseDirectory'
    size_t commonBasePosition = commonBasePathPosition(pathAbsolute, directoryAbsolute);
    const string& directoryRemainder = directoryAbsolute.substr(commonBasePosition);
    string relativePath = pathAbsolute.substr(commonBasePosition);
    if (relativePath[0] == pathSeparator)
        relativePath = relativePath.substr(1);

    // Construct the relative path by iteratively subtracting the additional folders from
    // 'directoryRemainder'
    size_t position = directoryRemainder.find(pathSeparator);
    while (position != string::npos) {
        if (relativePath.empty())
            relativePath = "..";
        else
            relativePath = ".." + (pathSeparator + relativePath);
        position = directoryRemainder.find(pathSeparator, position + 1);
    }
    return relativePath;
}
    
Directory FileSystem::currentDirectory() const {
    string currentDir;
#ifdef _WIN32
    // Get the size of the directory
    DWORD size = GetCurrentDirectory(0, NULL);
    char* buffer = new char[size];
    DWORD success = GetCurrentDirectory(size, buffer);
    if (success == 0) {
        // Log error
        DWORD error = GetLastError();
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
            string error(errorBuffer);
            LocalFree(errorBuffer);
            LERROR_SAFE("Error retrieving current directory: " << error);
        }
        return Directory();
    }
    currentDir = string(buffer);
    delete[] buffer;
#else
    char* buffer = new char[MAXPATHLEN];
    char* result = getcwd(buffer, MAXPATHLEN);
    if (result == nullptr) {
        LERROR_SAFE("Error retrieving current directory: " << errno);
        return Directory();
    }
    currentDir = string(buffer);
    delete[] buffer;
#endif
    return Directory(currentDir);
}
    
void FileSystem::setCurrentDirectory(const Directory& directory) const {
#ifdef _WIN32
    const BOOL success = SetCurrentDirectory(directory.path().c_str());
    if (success == 0) {
        // Log error
        DWORD error = GetLastError();
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
            string error(errorBuffer);
            LocalFree(errorBuffer);
            LERROR_SAFE("Error setting current directory: " << error);
        }
    }
#else
    const int success = chdir(directory.path().c_str());
    if (success != 0)
        LERROR_SAFE("Error setting current directory: " << errno);
#endif
}

bool FileSystem::fileExists(const File& path) const {
#ifdef _WIN32
    
#else
    const string& filePath = path.path();
    struct stat buffer;
    const int statResult = stat(filePath.c_str(), &buffer);
    if (statResult != 0)
        return false;
    const int isFile = S_ISREG(buffer.st_mode);
    return (isFile != 0);
#endif
}

bool FileSystem::directoryExists(const Directory& path) const {
#ifdef _WIN32
    
#else
    const string& dirPath = path.path();
    struct stat buffer;
    const int statResult = stat(dirPath.c_str(), &buffer);
    if (statResult != 0)
        return false;
    const int isDir = S_ISDIR(buffer.st_mode);
    return (isDir != 0);
#endif
}
    
bool FileSystem::deleteFile(const File& path) const {
    const bool isFile = fileExists(path);
    if (isFile) {
        const int removeResult = remove(path.path().c_str());
        return removeResult == 0;
    }
    else
        return false;
}
    
bool FileSystem::deleteDirectory(const Directory& path) const {
    const bool isDir = directoryExists(path);
    if (!isDir)
        return false;
#ifdef _WIN32
    const int rmDirResult = _rmdir(path.path());
    return rmDirResult != -1;
#else
    const string& dirPath = path;
    DIR* directory = opendir(dirPath.c_str());
    bool success = false;
    
    if (directory) {
        struct dirent* p;
        
        success = true;
        
        while (success) {
            p = readdir(directory);
            if (p == nullptr)
                break;
            
            const string name = string(p->d_name);
            if (name == "." || name == "..")
                continue;

            struct stat statbuf;
            const string fullName = dirPath + "/" + name;
            const int statResult = stat(fullName.c_str(), &statbuf);
            if (statResult == 0) {
                if (S_ISDIR(statbuf.st_mode))
                    success &= deleteDirectory(fullName);
                else {
                    const int removeSuccess = remove(fullName.c_str());
                    success &= (removeSuccess == 0);
                }
            }
        }
        closedir(directory);
    }
    
    if (success) {
        const int rmdirSuccess = rmdir(dirPath.c_str());
        success = (rmdirSuccess == 0);
    }
    
    return success;
#endif
}

void FileSystem::registerPathToken(const string& token, const string& path) {
#ifdef GHL_DEBUG
    if (token.empty()) {
        LERROR_SAFE("Token cannot not be empty");
        return;
    }
    
    const std::string beginning = token.substr(0, _tokenOpeningBraces.size());
    const std::string ending = token.substr(token.size() - _tokenClosingBraces.size());
    if ((beginning != _tokenOpeningBraces) || (ending != _tokenClosingBraces)) {
        LERROR_SAFE("Token has to start with '" + _tokenOpeningBraces +
                    "' and end with '" + _tokenClosingBraces + "'");
        return;
    }
    
    
    if (_fileSystem->_tokenMap.find(token) != _fileSystem->_tokenMap.end()) {
        LERROR_SAFE("Token already bound to path '" +
                    _fileSystem->_tokenMap[token] + "'");
        return;
    }
#endif
    _fileSystem->_tokenMap[token] = path;
}
    
string FileSystem::cleanupPath(const string& path) const {
    string localPath = path;
#ifdef _WIN32
    // In Windows, replace all '/' by '\\' for conformity
    std::replace(localPath.begin(), localPath.end(), '/', '\\');
    const std::string& drivePart = localPath.substr(0,3);
    const std::regex& driveRegex = std::regex("([[:lower:]][\\:][\\\\])");
    const bool hasCorrectSize = localPath.size() >= 3;
    if (hasCorrectSize && std::regex_match(drivePart, driveRegex))
        std::transform(localPath.begin(), localPath.begin() + 1,
                       localPath.begin(), toupper);
#endif
    
    // Remove all double separators
    size_t position = 0;
    while (position != string::npos) {
        position = localPath.find(pathSeparator + pathSeparator);
        if (position != string::npos)
            localPath = localPath.substr(0, position) + localPath.substr(position + 1);
    }
    
    // Remove trailing separator
    if (localPath[localPath.size() - 1] == pathSeparator)
        localPath = localPath.substr(0, localPath.size() - 1);
    
    return localPath;
}
    
size_t FileSystem::commonBasePathPosition(const string& p1, const string& p2) const {
    // 'currentPosition' stores the position until which the two paths are the same,
    // 'nextPosition' is a look-ahead. If the look-ahead is equal as well,
    // 'currentPosition' is replaced by this. At the end of the loop 'currentPosition'
    // contains the last position until which both paths are the same
    size_t currentPosition = 0;
    size_t nextPosition = p1.find(pathSeparator);
    while (nextPosition != string::npos) {
        const int result = p1.compare(0, nextPosition, p2, 0 , nextPosition);
        if (result == 0) {
            currentPosition = nextPosition;
            nextPosition = p1.find(pathSeparator, nextPosition + 1);
        }
        else
            break;
    }
    const int result = p1.compare(0, p1.length(), p2, 0 , p1.length());
    if (result == 0)
        currentPosition = p1.length();
    return currentPosition;
}
    
bool FileSystem::hasTokens(const string& path) const {
    const bool hasOpeningBrace = path.find(_tokenOpeningBraces) != string::npos;
    const bool hasClosingBrace = path.find(_tokenClosingBraces) != string::npos;
    return hasOpeningBrace && hasClosingBrace;
}

bool FileSystem::expandPathTokens(std::string& path) const {
    while (hasTokens(path)) {
        string::size_type beginning = path.find(_tokenOpeningBraces);
        string::size_type closing = path.find(_tokenClosingBraces);
        string::size_type closingLocation = closing + _tokenClosingBraces.size();
        const std::string currentToken = path.substr(beginning, closingLocation);
        const std::string& replacement = resolveToken(currentToken);
        if (replacement == currentToken) {
            // replacement == currentToken will be true if the respective token could not
            // be found;  resolveToken will print an error in that case
            return false;
        }
        path.replace(beginning, closing + _tokenClosingBraces.size() - beginning, replacement);
    }
    return true;
}

bool FileSystem::hasToken(const std::string& path, const std::string& token) const {
    if (!hasTokens(path))
        return false;
    else {
        string::size_type beginning = path.find(_tokenOpeningBraces);
        string::size_type closing = path.find(_tokenClosingBraces);
        while ((beginning != string::npos) && (closing != string::npos)) {
            string::size_type closingLocation = closing + _tokenClosingBraces.size();
            const std::string currentToken = path.substr(beginning, closingLocation);
            if (currentToken == token)
                return true;
            else {
                beginning = path.find(_tokenOpeningBraces, closing);
                closing = path.find(_tokenClosingBraces, beginning);
            }
        }
        return false;
    }
}
    
const std::string FileSystem::resolveToken(const std::string& token) const {
    const std::map<std::string, std::string>::const_iterator it = _tokenMap.find(token);
    if (it == _tokenMap.end()) {
        LERROR_SAFE("Token '" + token + "' could not be resolved");
        return token;
    }
    else
        return it->second;
}

} // namespace filesystem
} // namespace ghoul
