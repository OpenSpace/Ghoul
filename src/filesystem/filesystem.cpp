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

#include <ghoul/filesystem/filesystem.h>

#include <ghoul/filesystem/cachemanager.h>
#include <ghoul/logging/logmanager.h>

#include <algorithm>
#include <cassert>
#include <regex>
#include <cstdio>

#ifdef WIN32
#include <direct.h>
#include <windows.h>
#include <Shlwapi.h>
#else
#include <dirent.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#endif

using std::string;

namespace {
    const string _loggerCat = "FileSystem";
}

namespace ghoul {
namespace filesystem {

FileSystem* FileSystem::_fileSystem = nullptr;
const std::string FileSystem::TokenOpeningBraces = "${";
const std::string FileSystem::TokenClosingBraces = "}";
#ifdef WIN32
const char FileSystem::PathSeparator = '\\';
#else
const char FileSystem::PathSeparator = '/';
#endif

FileSystem::FileSystem()
    : _cacheManager(nullptr)
{}

FileSystem::~FileSystem() {
    if(_cacheManager)
        delete _cacheManager;
}

void FileSystem::initialize() {
    assert(_fileSystem == nullptr);
    if (_fileSystem == nullptr)
        _fileSystem = new FileSystem;

#if !defined(WIN32) && !defined(__APPLE__)
	_fileSystem->initializeInternalLinux();
#endif
}

void FileSystem::deinitialize() {
    assert(_fileSystem != nullptr);
#ifdef WIN32
	_fileSystem->deinitializeInternalWindows();
#elif __APPLE__
	_fileSystem->deinitializeInternalApple();
#else
	_fileSystem->deinitializeInternalLinux();
#endif
    delete _fileSystem;
}

FileSystem& FileSystem::ref() {
    assert(_fileSystem != nullptr);
    return *_fileSystem;
}

string FileSystem::absolutePath(string path) const {
    expandPathTokens(path);

    static const int PATH_BUFFER_SIZE = 4096;
    char* buffer = nullptr;
#ifdef WIN32
    buffer = new char[PATH_BUFFER_SIZE];
    const DWORD success = GetFullPathName(path.c_str(), PATH_BUFFER_SIZE, buffer, 0);
    if (success == 0) {
        delete[] buffer;
        buffer = nullptr;
    }
#else
    char errorBuffer[PATH_BUFFER_SIZE];
    buffer = realpath(path.c_str(), errorBuffer);
    if (buffer == NULL) {
        LERROR("Error resolving the real path. Problem part: '" << errorBuffer << "'");
        return path;
    }
#endif

    if (buffer) {
        path = string(buffer);
#ifdef WIN32
        delete[] buffer;
#endif
        return path;
    }

    return path;
}

string FileSystem::relativePath(string path,
                                const Directory& baseDirectory) const
{
    if (path.empty()) {
        LERROR("'path' must contain a path");
        return path;
    }
    string&& pathAbsolute = cleanupPath(absolutePath(path));
    string&& directoryAbsolute = cleanupPath(absolutePath(baseDirectory));

    // Return identity path if absolutes are equal
    if (pathAbsolute == directoryAbsolute)
        return ".";

    // Check for different drives on Windows
    if (pathAbsolute[0] != directoryAbsolute[0])
        return path;

    // Find the common part in the 'path' and 'baseDirectory'
    size_t commonBasePosition = commonBasePathPosition(pathAbsolute, directoryAbsolute);
    string&& directoryRemainder = directoryAbsolute.substr(commonBasePosition);
    string relativePath = pathAbsolute.substr(commonBasePosition);
    if (relativePath[0] == PathSeparator)
        relativePath = relativePath.substr(1);

    // Construct the relative path by iteratively subtracting the additional folders from
    // 'directoryRemainder'
    size_t position = directoryRemainder.find(PathSeparator);
    while (position != string::npos) {
        if (relativePath.empty())
            relativePath = "..";
        else
            relativePath = ".." + (PathSeparator + relativePath);
        position = directoryRemainder.find(PathSeparator, position + 1);
    }
    return relativePath;
}
    
std::string FileSystem::pathByAppendingComponent(std::string path,
												 std::string component) const
{
	return std::move(path) + PathSeparator + std::move(component);
}

Directory FileSystem::currentDirectory() const {
#ifdef WIN32
    // Get the size of the directory
    DWORD size = GetCurrentDirectory(0, NULL);
    char* buffer = new char[size];
    DWORD success = GetCurrentDirectory(size, buffer);
    if (success == 0) {
        // Log error
        DWORD error = GetLastError();
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
            string error(errorBuffer);
            LocalFree(errorBuffer);
            LERROR("Error retrieving current directory: " << error);
        }
        return Directory();
    }
    string&& currentDir = std::move(string(buffer));
    delete[] buffer;
#else
    char* buffer = new char[MAXPATHLEN];
    char* result = getcwd(buffer, MAXPATHLEN);
    if (result == nullptr) {
        LERROR("Error retrieving current directory: " << errno);
        return Directory();
    }
    string currentDir = string(buffer);
    delete[] buffer;
#endif
    return Directory(currentDir);
}
    
void FileSystem::setCurrentDirectory(const Directory& directory) const {
#ifdef WIN32
    const BOOL success = SetCurrentDirectory(directory.path().c_str());
    if (success == 0) {
        // Log error
        DWORD error = GetLastError();
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
            string error(errorBuffer);
            LocalFree(errorBuffer);
            LERROR("Error setting current directory: " << error);
        }
    }
#else
    const int success = chdir(directory.path().c_str());
    if (success != 0)
        LERROR("Error setting current directory: " << errno);
#endif
}

bool FileSystem::fileExists(const File& path) const {
	return fileExists(path.path(),true);
}

bool FileSystem::fileExists(std::string path, bool isRawPath) const {
	if (!isRawPath)
		path = absPath(path);
#ifdef WIN32
	const DWORD attributes = GetFileAttributes(path.c_str());
	if (attributes == INVALID_FILE_ATTRIBUTES) {
		const DWORD error = GetLastError();
		if ((error != ERROR_FILE_NOT_FOUND) && (error != ERROR_PATH_NOT_FOUND)) {
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
				string error(errorBuffer);
				LocalFree(errorBuffer);
				LERROR("Error retrieving file attributes: " << error);
			}
		}
		return false;
	}
	else
		return (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
	struct stat buffer;
	const int statResult = stat(path.c_str(), &buffer);
	if (statResult != 0)
		return false;
	const int isFile = S_ISREG(buffer.st_mode);
	return (isFile != 0);
#endif
}

bool FileSystem::directoryExists(const Directory& path) const {
#ifdef WIN32
    const DWORD attributes = GetFileAttributes(path.path().c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        const DWORD error = GetLastError();
        if ((error != ERROR_FILE_NOT_FOUND) && (error != ERROR_PATH_NOT_FOUND)) {
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
                string error(errorBuffer);
                LocalFree(errorBuffer);
                LERROR("Error retrieving file attributes: " << error);
            }
        }
        return false;
    }
    else
        return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
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
    
bool FileSystem::createDirectory(const Directory& path, bool recursive) const {
	if (recursive) {
		std::vector<Directory> directories;
		Directory d = path;
        while (!FileSys.directoryExists(d)) {
            //LERROR("Adding path to v: " << d.path());
			directories.push_back(d);
			d = d.parentDirectory();
		}

		bool success = true;
		std::for_each(
			directories.rbegin(),
			directories.rend(),
			[&success, this](const Directory& d)
			   {
				if (!success)
					return;
				else
					success = createDirectory(d, false);
				});

		return success;
	}
	else {
#ifdef WIN32
		BOOL success = CreateDirectory(path.path().c_str(), NULL);
		if (success)
			return true;
		else {
			DWORD error = GetLastError();
			if (ERROR == ERROR_ALREADY_EXISTS)
				return true;
			else {
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
					string error(errorBuffer);
					LocalFree(errorBuffer);
					LERROR("Error creating directory '" << path << "': " << error);
				}
				else 
					LERROR("Error creating directory '" << path << "'");

				return false;
			}
		}
#else
		int success = mkdir(path.path().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		if (success == 0)
			return true;
		else {
			if (errno == EEXIST)
				return true;
			else {
				LERROR("Error creating directory '" << path << "': " << errno);
				return false;
			}
		}
#endif
	}
}

bool FileSystem::deleteDirectory(const Directory& path, bool recursive) const {
    const bool isDir = directoryExists(path);
    if (!isDir)
        return false;

	if (!recursive && !emptyDirectory(path))
		return false;

#ifdef WIN32
	const string& dirPath = path;
	bool success = true;
	
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	DWORD Attributes; 

	const std::string dirWildcard = dirPath + PathSeparator + "*";

	//List files
	hFind = FindFirstFile(dirWildcard.c_str(), &FindFileData);
	do{
		const std::string p = FindFileData.cFileName;
		if (p == "." || p == "..")
			continue;

		const std::string fullPath = dirPath + PathSeparator + p;
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			bool rmDirResult = deleteDirectory(fullPath, recursive);
			if (!rmDirResult)
				success = false;
		}
		else
		{
			const int rmFileResult = remove(fullPath.c_str());
			if (rmFileResult != 0)
				success = false;
		}
	}while(FindNextFile(hFind, &FindFileData));
	FindClose(hFind);

	if (!success)
		return false;

	const int rmDirResult = _rmdir(dirPath.c_str());
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

bool FileSystem::emptyDirectory(const Directory& path) const {
    const string& dirPath = path;
#ifdef WIN32
	return PathIsDirectoryEmpty(dirPath.c_str());
#else
	int n = 0;
	struct dirent *d;
	DIR *dir = opendir(dirPath.c_str());
	if (dir == NULL) //Not a directory or doesn't exist
		return false;
	while ((d = readdir(dir)) != NULL) {
		if (++n > 2)
			break;
	}
	closedir(dir);
	if (n <= 2) //Directory Empty
		return true;
	else
		return false;
#endif
}

void FileSystem::registerPathToken(string token, string path, bool override) {
#ifdef GHL_DEBUG
    if (token.empty()) {
        LERROR("Token cannot not be empty");
        return;
    }
    
    std::string&& beginning = token.substr(0, TokenOpeningBraces.size());
    std::string&& ending = token.substr(token.size() - TokenClosingBraces.size());
    if ((beginning != TokenOpeningBraces) || (ending != TokenClosingBraces)) {
        LERROR("Token has to start with '" + TokenOpeningBraces +
                    "' and end with '" + TokenClosingBraces + "'");
        return;
    }
    
    
	if (!override) {
		if (_fileSystem->_tokenMap.find(token) != _fileSystem->_tokenMap.end()) {
			LERROR("Token already bound to path '" +
						_fileSystem->_tokenMap[token] + "'");
			return;
		}
	}
#endif
	if (override) {
		auto it = _fileSystem->_tokenMap.find(token);
		_fileSystem->_tokenMap.erase(it);
	}
	_fileSystem->_tokenMap.emplace(token, path);
}
    
string FileSystem::cleanupPath(string path) const {
#ifdef WIN32
    // In Windows, replace all '/' by '\\' for conformity
    std::replace(path.begin(), path.end(), '/', '\\');
    std::string&& drivePart = path.substr(0, 3);
    std::regex&& driveRegex = std::regex("([[:lower:]][\\:][\\\\])");
    const bool hasCorrectSize = path.size() >= 3;
    if (hasCorrectSize && std::regex_match(drivePart, driveRegex))
        std::transform(path.begin(), path.begin() + 1,
                        path.begin(), toupper);
#else
    // Remove all double separators (will automatically be done on Windows)
#endif
    size_t position = 0;
    while (position != string::npos) {
        char dualSeparator[2];
        dualSeparator[0] = PathSeparator;
        dualSeparator[1] = PathSeparator;
        position = path.find(dualSeparator);
        if (position != string::npos)
            path = std::move(path.substr(0, position) + path.substr(position + 1));
    }
    
    // Remove trailing separator
    if (path[path.size() - 1] == PathSeparator)
        path = std::move(path.substr(0, path.size() - 1));
    
    return path;
}
    
size_t FileSystem::commonBasePathPosition(const string& p1, const string& p2) const {
    // 'currentPosition' stores the position until which the two paths are the same,
    // 'nextPosition' is a look-ahead. If the look-ahead is equal as well,
    // 'currentPosition' is replaced by this. At the end of the loop 'currentPosition'
    // contains the last position until which both paths are the same
    size_t currentPosition = 0;
    size_t nextPosition = p1.find(PathSeparator);
    while (nextPosition != string::npos) {
        const int result = p1.compare(0, nextPosition, p2, 0 , nextPosition);
        if (result == 0) {
            currentPosition = nextPosition;
            nextPosition = p1.find(PathSeparator, nextPosition + 1);
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
    const bool hasOpeningBrace = path.find(TokenOpeningBraces) != string::npos;
    const bool hasClosingBrace = path.find(TokenClosingBraces) != string::npos;
    return hasOpeningBrace && hasClosingBrace;
}

bool FileSystem::expandPathTokens(std::string& path) const {
    while (hasTokens(path)) {
        string::size_type beginning = path.find(TokenOpeningBraces);
        string::size_type closing = path.find(TokenClosingBraces);
        string::size_type closingLocation = closing + TokenClosingBraces.size();
        const std::string currentToken = path.substr(beginning, closingLocation);
        const std::string& replacement = resolveToken(currentToken);
        if (replacement == currentToken) {
            // replacement == currentToken will be true if the respective token could not
            // be found;  resolveToken will print an error in that case
            return false;
        }
        path.replace(beginning, closing + TokenClosingBraces.size() - beginning, replacement);
    }
    return true;
}

std::vector<std::string> FileSystem::tokens() const {
	std::vector<std::string> tokens;
	for (auto token : _tokenMap) {
		tokens.push_back(token.first);
	}
	return tokens;
}
    
bool FileSystem::createCacheManager(const Directory& cacheDirectory) {
    if (!_fileSystem->directoryExists(cacheDirectory)) {
        LERROR("Requested cache directory '" << cacheDirectory << "' did not exist");
        return false;
    }
    
    if (_fileSystem->_cacheManager != nullptr) {
        LERROR("CacheManager was already created");
        return false;
    }
    
    _fileSystem->_cacheManager = new CacheManager(cacheDirectory);
    assert(_fileSystem->_cacheManager);
    return true;
}

void FileSystem::destroyCacheManager() {
	assert(_fileSystem->_cacheManager);

	delete _fileSystem->_cacheManager;
	_fileSystem->_cacheManager = nullptr;
}

CacheManager* FileSystem::cacheManager() {
	assert(_cacheManager);
	return _cacheManager;
}

void FileSystem::triggerFilesystemEvents() {
#ifdef WIN32
	// Sleeping for 0 milliseconds will trigger any pending asynchronous procedure calls 
	SleepEx(0, TRUE);
#endif
#if defined(__APPLE__)
    triggerFilesystemEventsInternalApple();
#endif
}

bool FileSystem::hasToken(const std::string& path, const std::string& token) const {
    if (!hasTokens(path))
        return false;
    else {
        string::size_type beginning = path.find(TokenOpeningBraces);
        string::size_type closing = path.find(TokenClosingBraces);
        while ((beginning != string::npos) && (closing != string::npos)) {
            string::size_type closingLocation = closing + TokenClosingBraces.size();
            const std::string currentToken = path.substr(beginning, closingLocation);
            if (currentToken == token)
                return true;
            else {
                beginning = path.find(TokenOpeningBraces, closing);
                closing = path.find(TokenClosingBraces, beginning);
            }
        }
        return false;
    }
}
    
std::string FileSystem::resolveToken(const std::string& token) const {
    const std::map<std::string, std::string>::const_iterator it = _tokenMap.find(token);
    if (it == _tokenMap.end()) {
        LERROR("Token '" + token + "' could not be resolved");
        return token;
    }
    else
        return it->second;
}

} // namespace filesystem
} // namespace ghoul
