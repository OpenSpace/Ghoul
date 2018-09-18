/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2018                                                               *
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

#include <ghoul/fmt.h>
#include <ghoul/filesystem/cachemanager.h>
#include <ghoul/filesystem/file.h>
#include <ghoul/logging/logmanager.h>
#include <algorithm>
#include <regex>

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
#include <string.h>
#include <errno.h>
#endif

#if !defined(WIN32) && !defined(__APPLE__)
#include <sys/inotify.h>
#endif

using std::string;
using std::vector;

namespace {
    constexpr const char* _loggerCat = "FileSystem";
    constexpr const char* TemporaryPathToken = "TEMPORARY";

    constexpr size_t constLength(const char* str) {
        return (*str == '\0') ? 0 : constLength(str + 1) + 1;
    }

    constexpr size_t TokenOpeningBracesSize = constLength(
        ghoul::filesystem::FileSystem::TokenOpeningBraces
    );

    constexpr size_t TokenClosingBracesSize = constLength(
        ghoul::filesystem::FileSystem::TokenClosingBraces
    );
} // namespace

namespace ghoul::filesystem {

FileSystem::FileSystemException::FileSystemException(const string& msg)
    : RuntimeError(msg, "FileSystem")
{}

FileSystem::ResolveTokenException::ResolveTokenException(string t)
    : FileSystemException(fmt::format("Token '{}' could not be resolved", t))
    , token(std::move(t))
{}

FileSystem::FileSystem() {
    std::string temporaryPath;
#ifdef WIN32
    std::vector<char> ptr;
    DWORD result = GetTempPath(0, ptr.data());
    if (result != 0) {
        std::vector<TCHAR> tempPath(result);
        result = GetTempPath(result, tempPath.data());
        if ((result != 0) || (result <= tempPath.size())) {
            temporaryPath = tempPath.data();
        }
    }
#else // WIN32
    temporaryPath = P_tmpdir;
#endif // WIN32

    if (!temporaryPath.empty()) {
        LINFO(fmt::format("Set temporary path ${{TEMPORARY}} to {}", temporaryPath));
        registerPathToken(
            TokenOpeningBraces + std::string(TemporaryPathToken) + TokenClosingBraces,
            temporaryPath
        );
    }
    else {
        throw FileSystemException(
            "Could not find the path of the system's temporary files");
    }

#if !defined(WIN32) && !defined(__APPLE__)
    _inotifyHandle = inotify_init();
    _keepGoing = true;
    _t = std::thread(inotifyWatcher);
#endif
}

FileSystem::~FileSystem() {
#if defined WIN32
    deinitializeInternalWindows();
#elif defined __APPLE__
    deinitializeInternalApple();
#else
    deinitializeInternalLinux();
#endif
}

string FileSystem::absolutePath(string path, const vector<string>& ignoredTokens) const {
    ghoul_assert(!path.empty(), "Path must not be empty");
    expandPathTokens(path, ignoredTokens);

    const int PathBufferSize = 4096;
    std::vector<char> buffer(PathBufferSize);

#ifdef WIN32
    const DWORD success = GetFullPathName(
        path.c_str(),
        PathBufferSize,
        buffer.data(),
        nullptr
    );
    if (success == 0) {
        throw FileSystemException(fmt::format(
            "Error retrieving absolute path '{}'",
            path
        ));
    }
#else
    if (!realpath(path.c_str(), buffer.data())) {
        // Find the longest path that exists
        string fullPath(path);
        string::size_type match;
        string::size_type lastMatch = std::string::npos;
        while ((match = fullPath.rfind(PathSeparator, lastMatch - 1)) != string::npos) {
            string before = fullPath.substr(0, match);
            string after = fullPath.substr(match, string::npos);
            if (realpath(before.c_str(), buffer.data())) {
                string resolvedPath = string(buffer.data()) + after;
                return resolvedPath;
            }
            lastMatch = match;
        }
        // Nothing in the relative path was found, use current directory
        string before = ".";
        string after = fullPath;
        if (realpath(before.c_str(), buffer.data())) {
            string resolvedPath = string(buffer.data()) + PathSeparator + after;
            return resolvedPath;
        }
    }
#endif

    path.assign(buffer.data());
    return path;
}

string FileSystem::relativePath(string path, const Directory& baseDirectory) const {
    ghoul_assert(!path.empty(), "Path must not be empty");

    string pathAbsolute = cleanupPath(absolutePath(path));
    string directoryAbsolute = cleanupPath(absolutePath(baseDirectory));

    // Return identity path if absolutes are equal
    if (pathAbsolute == directoryAbsolute) {
        return ".";
    }

    // Check for different drives on Windows
    if (pathAbsolute[0] != directoryAbsolute[0]) {
        return path;
    }

    // Find the common part in the 'path' and 'baseDirectory'
    size_t commonBasePosition = commonBasePathPosition(pathAbsolute, directoryAbsolute);
    string directoryRemainder = directoryAbsolute.substr(commonBasePosition);
    string relativePath = pathAbsolute.substr(commonBasePosition);
    if (relativePath[0] == PathSeparator) {
        relativePath = relativePath.substr(1);
    }

    // Construct the relative path by iteratively subtracting the additional folders from
    // 'directoryRemainder'
    size_t position = directoryRemainder.find(PathSeparator);
    while (position != string::npos) {
        if (relativePath.empty()) {
            relativePath = "..";
        }
        else {
            relativePath = ".." + (PathSeparator + relativePath); // NOLINT
        }
        position = directoryRemainder.find(PathSeparator, position + 1);
    }
    return relativePath;
}

string FileSystem::pathByAppendingComponent(string path, string component) const {
    return std::move(path) + PathSeparator + std::move(component);
}

std::string FileSystem::convertPathSeparator(std::string path, char separator) const {
    std::replace(path.begin(), path.end(), PathSeparator, separator);
    return path;
}

Directory FileSystem::currentDirectory() const {
#ifdef WIN32
    // Get the size of the directory
    DWORD size = GetCurrentDirectory(0, nullptr);
    std::vector<char> buffer(size);
    DWORD success = GetCurrentDirectory(size, buffer.data());
    if (success == 0) {
        // Log error
        DWORD error = GetLastError();
        LPTSTR errorBuffer = nullptr;
        DWORD nValues = FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&errorBuffer, // NOLINT
            0,
            nullptr
        );
        if ((nValues > 0) && (errorBuffer != nullptr)) {
            string msg(errorBuffer);
            LocalFree(errorBuffer);
            throw FileSystemException(fmt::format(
                "Error retrieving current directory: {}",
                msg
            ));
        }
        throw FileSystemException("Error retrieving current directory");
    }
    string currentDir(buffer.data());
#else
    std::vector<char> buffer(MAXPATHLEN);
    char* result = getcwd(buffer.data(), MAXPATHLEN);
    if (result == nullptr) {
        throw FileSystemException(fmt::format(
            "Error retrieving current directory: {}", strerror(errno)
        ));
    }
    string currentDir(buffer.data());
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
        DWORD nValues = FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&errorBuffer, // NOLINT
            0,
            nullptr
        );
        if ((nValues > 0) && (errorBuffer != nullptr)) {
            string msg(errorBuffer);
            LocalFree(errorBuffer);
            throw FileSystemException(fmt::format(
                "Error setting current directory: {}", msg
            ));
        }
    }
#else
    const int success = chdir(directory.path().c_str());
    if (success != 0) {
        throw FileSystemException(fmt::format(
            "Error setting current directory: {}", strerror(errno)
        ));
    }
#endif
}

bool FileSystem::fileExists(const File& path) const {
#ifdef WIN32
    BOOL exists = PathFileExists(path.path().c_str());
    if (exists == FALSE) {
        // The path did not exist, so the file cannot exist
        return false;
    }
    else {
        const DWORD attributes = GetFileAttributes(path.path().c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES) {
            const DWORD error = GetLastError();
            if ((error != ERROR_FILE_NOT_FOUND) && (error != ERROR_PATH_NOT_FOUND)) {
                LPTSTR errorBuffer = nullptr;
                DWORD nValues = FormatMessage(
                    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                    nullptr,
                    error,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR)&errorBuffer, // NOLINT
                    0,
                    nullptr
                );
                if ((nValues > 0) && (errorBuffer != nullptr)) {
                    string msg(errorBuffer);
                    LocalFree(errorBuffer);
                    throw FileSystemException(fmt::format(
                        "Error retrieving file attributes: {}", msg
                    ));
                }
            }
            throw FileSystemException("Error retrieving file attributes");
        }
        else {
            return (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
        }
    }
#else
    struct stat buffer;
    const int statResult = stat(path.path().c_str(), &buffer);
    if (statResult != 0) {
        if (errno == ENOENT) {
            // The error is that the file didn't exist
            return false;
        }
        else {
            throw FileSystemException(fmt::format(
                "Error retrieving file attributes: {}", strerror(errno)
            ));
        }
    }
    const int isFile = S_ISREG(buffer.st_mode);
    return (isFile != 0);
#endif
}

bool FileSystem::directoryExists(const Directory& path) const {
#ifdef WIN32
    BOOL isDirectory = PathIsDirectory(path.path().c_str());
    return isDirectory == static_cast<BOOL>(FILE_ATTRIBUTE_DIRECTORY);
#else
    const string& dirPath = path.path();
    struct stat buffer;
    const int statResult = stat(dirPath.c_str(), &buffer);
    if (statResult != 0) {
        if (errno == ENOENT) {
            // The error is that the file didn't exist
            return false;
        }
        else {
            throw FileSystemException(fmt::format(
                "Error retrieving file attributes: {}", strerror(errno)
            ));
        }
    }
    const int isDir = S_ISDIR(buffer.st_mode);
    return (isDir != 0);
#endif
}

bool FileSystem::deleteFile(const File& path) const {
    bool isFile = fileExists(path);
    if (isFile) {
        int removeResult = remove(path.path().c_str());
        return (removeResult == 0);
    }
    else {
        return false;
    }
}

void FileSystem::createDirectory(const Directory& path, Recursive recursive) const {
    if (recursive) {
        std::vector<Directory> directories;
        Directory dir = path;
        while (!FileSys.directoryExists(dir)) {
            //LERROR("Adding path to v: " << d.path());
            directories.push_back(dir);
            dir = dir.parentDirectory();
        }

        std::for_each(
            directories.rbegin(),
            directories.rend(),
            [this](const Directory& d) {
                if (!FileSys.directoryExists(d)) {
                    createDirectory(d, Recursive::No);
                }
            });

        return;
    }
    else {
#ifdef WIN32
        BOOL success = CreateDirectory(path.path().c_str(), nullptr);
        if (!success) {
            DWORD error = GetLastError();
            if (ERROR == ERROR_ALREADY_EXISTS) {
                return;
            }
            else {
                LPTSTR errorBuffer = nullptr;
                DWORD nValues = FormatMessage(
                    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                    nullptr,
                    error,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR)&errorBuffer, // NOLINT
                    0,
                    nullptr
                );
                if ((nValues > 0) && (errorBuffer != nullptr)) {
                    string errorMsg(errorBuffer);
                    LocalFree(errorBuffer);
                    throw FileSystemException(fmt::format(
                        "Error creating directory '{}': {}", path.path(), errorMsg
                    ));
                }
                throw FileSystemException(fmt::format(
                    "Error creating directory '{}'", path.path()
                ));
            }
        }
#else
        int success = mkdir(path.path().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (success != 0 && errno != EEXIST) {
            throw FileSystemException(fmt::format(
                "Error creating diretory '{}': {}", path.path(), strerror(errno)
            ));
        }
#endif
    }
}

void FileSystem::deleteDirectory(const Directory& path, Recursive recursive) const {
    ghoul_assert(directoryExists(path), "Path must be an existing directory");
    if ((!recursive) && (!emptyDirectory(path))) {
        throw FileSystemException("Directory must be empty");
    }

#ifdef WIN32
    const string& dirPath = path;

    WIN32_FIND_DATA FindFileData;

    const std::string dirWildcard = dirPath + PathSeparator + "*";

    //List files
    HANDLE hFind = FindFirstFile(dirWildcard.c_str(), &FindFileData);
    do {
        const std::string p = FindFileData.cFileName;
        if (p == "." || p == "..") {
            continue;
        }

        const std::string fullPath = dirPath + PathSeparator + p;
        if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            try {
                deleteDirectory(fullPath, recursive);
            }
            catch (...) {
                FindClose(hFind);
                throw;
            }
        }
        else {
            const int rmFileResult = remove(fullPath.c_str());
            if (rmFileResult != 0) {
                FindClose(hFind);
                throw FileSystemException(fmt::format(
                    "Error removing directory '{}'", path.path()
                ));
            }
        }
    } while (FindNextFile(hFind, &FindFileData));
    FindClose(hFind);

    const int rmDirResult = _rmdir(dirPath.c_str());
    if (rmDirResult == -1) {
        throw FileSystemException(fmt::format(
            "Error removing directory '{}'", path.path()
        ));
    }
#else
    const string& dirPath = path;

    if (recursive) {
        DIR* directory = opendir(dirPath.c_str());

        if (!directory) {
            throw FileSystemException(fmt::format(
                "Error removing directory '{}': {}", path.path(), strerror(errno)
            ));
        }

        while (true) {
            struct dirent* p = readdir(directory);
            if (p == nullptr) {
                break;
            }

            string name = string(p->d_name);
            if (name == "." || name == "..") {
                continue;
            }

            struct stat statbuf;
string fullName = dirPath + "/" + name;
int statResult = stat(fullName.c_str(), &statbuf);
if (statResult == 0) {
    if (S_ISDIR(statbuf.st_mode)) {
        deleteDirectory(fullName);
    }
    else {
        int removeSuccess = remove(fullName.c_str());
        if (removeSuccess != 0) {
            throw FileSystemException(fmt::format(
                "Error deleting file '{}' in directory '{}': {}",
                fullName, path.path(), strerror(errno)
            ));
        }
    }
}
else {
    throw FileSystemException(fmt::format(
        "Error getting information about file '{}' in directory '{}': {}",
        fullName, path.path(), strerror(errno)
    ));
}
        }
        closedir(directory);
    }
    int rmdirSuccess = rmdir(dirPath.c_str());
    if (rmdirSuccess == -1) {
        throw FileSystemException(fmt::format(
            "Error deleting directory '{}': {}", path.path(), strerror(errno)
        ));
    }
#endif
}

bool FileSystem::emptyDirectory(const Directory& path) const {
    const string& dirPath = path;
#ifdef WIN32
    return PathIsDirectoryEmpty(dirPath.c_str()) == TRUE;
#else
    int n = 0;
    DIR* dir = opendir(dirPath.c_str());
    if (dir == NULL) {
        //Not a directory or doesn't exist
        return false;
    }
    while (readdir(dir) != nullptr) {
        // The '.' and '..' are always present
        if (++n > 2) {
            break;
        }
    }
    closedir(dir);
    if (n <= 2) {
        // Directory Empty
        return true;
    }
    else {
        return false;
    }
#endif
}

void FileSystem::registerPathToken(string token, string path, Override override) {
    ghoul_assert(!token.empty(), "Token must not be empty");

    ghoul_assert(
        (token.substr(0, TokenOpeningBracesSize) == TokenOpeningBraces) &&
        (token.substr(token.size() - TokenClosingBracesSize) == TokenClosingBraces),
        "Token must be enclosed by TokenBraces"
    );

    if (!override) {
        ghoul_assert(_tokenMap.find(token) == _tokenMap.end(),
            "Token must not have been registered before"
        );
    }

    if (override) {
        auto it = _tokenMap.find(token);
        if (it != _tokenMap.end()) {
            _tokenMap.erase(it);
        }
    }
    _tokenMap[std::move(token)] = std::move(path);
}

bool FileSystem::expandPathTokens(string& path, const vector<string>& ignoredTokens) const
{
    // TokenInformation = <token, beginning position, length>
    struct TokenInformation {
        string token;
        string::size_type beginning;
        string::size_type length;
    };
    auto nextToken = [ignoredTokens, &path]() -> TokenInformation {
        string::size_type currentPosition = 0;
        while (true) {
            string::size_type beginning = path.find(TokenOpeningBraces, currentPosition);
            string::size_type closing = path.find(
                TokenClosingBraces, beginning + std::strlen(TokenOpeningBraces)
            );
            string::size_type closingLocation = closing + TokenClosingBracesSize;

            if (beginning == string::npos || closing == string::npos) {
                // There is no token left
                return { "", string::npos, 0 };
            }

            std::string currentToken = path.substr(
                beginning,
                closingLocation - beginning
            );

            auto it = std::find(ignoredTokens.begin(), ignoredTokens.end(), currentToken);
            if (it != ignoredTokens.end()) {
                // The found token is an ignored one
                currentPosition = closingLocation;
                continue;
            }
            else {
                return { currentToken, beginning, closingLocation - beginning };
            }
        }
    };

    while (true) {
        TokenInformation tokenInformation = nextToken();
        if (tokenInformation.token.empty()) {
            break;
        }

        const std::string& replacement = resolveToken(tokenInformation.token);
        path.replace(
            tokenInformation.beginning,
            tokenInformation.length,
            replacement
        );
    }

    //while (containsToken(path)) {
    //    string::size_type beginning = path.find(TokenOpeningBraces, currentPosition);
    //    string::size_type closing = path.find(
    //        TokenClosingBraces, beginning + TokenOpeningBraces.size()
    //    );
    //    string::size_type closingLocation = closing + TokenClosingBraces.size();
    //    std::string currentToken = path.substr(beginning, closingLocation - beginning);

    //    currentPosition = closingLocation;
    //    auto it = std::find(ignoredTokens.begin(), ignoredTokens.end(), currentToken);
    //    if (it != ignoredTokens.end()) {
    //        // The token we have found has to be ignored
    //        continue;
    //    }
    //    const std::string& replacement = resolveToken(currentToken);
    //    if (replacement == currentToken) {
    //        // replacement == currentToken will be true if the respective token could
    //        // not be found;  resolveToken will print an error in that case
    //        return false;
    //    }
    //    path.replace(
    //        beginning,
    //        closing + TokenClosingBraces.size() - beginning,
    //        replacement
    //    );
    //}
    return true;
}

std::vector<string> FileSystem::tokens() const {
    std::vector<string> tokens;
    for (const auto& token : _tokenMap) {
        tokens.push_back(token.first);
    }
    return tokens;
}

bool FileSystem::hasRegisteredToken(const string& token) const {
    return _tokenMap.find(token) != _tokenMap.end();
}

void FileSystem::createCacheManager(const Directory& cacheDirectory, int version) {
    ghoul_assert(directoryExists(cacheDirectory), "Cache directory did not exist");
    ghoul_assert(!_cacheManager, "CacheManager was already created");

    _cacheManager = std::make_unique<CacheManager>(cacheDirectory, version);
    ghoul_assert(_cacheManager, "CacheManager creation failed");
}

void FileSystem::destroyCacheManager() {
    ghoul_assert(_cacheManager, "CacheManager was not created");
    _cacheManager = nullptr;
}

CacheManager* FileSystem::cacheManager() {
    return _cacheManager.get();
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

string FileSystem::cleanupPath(string path) const {
    ghoul_assert(!path.empty(), "Path must not be empty");

#ifdef WIN32
    // In Windows, replace all '/' by '\\' for conformity
    std::replace(path.begin(), path.end(), '/', '\\');
    std::string drivePart = path.substr(0, 3);
    std::regex driveRegex = std::regex(R"(([[:lower:]][\:][\\]))");
    bool hasCorrectSize = path.size() >= 3;
    if (hasCorrectSize && std::regex_match(drivePart, driveRegex)) {
        std::transform(
            path.begin(),
            path.begin() + 1,
            path.begin(),
            [](char v) { return static_cast<char>(toupper(v)); }
        );
    }
#else
    // Remove all double separators (will automatically be done on Windows)
#endif
    size_t position = 0;
    while (position != string::npos) {
        char dualSeparator[] = { PathSeparator, PathSeparator };
        position = path.find(dualSeparator, 0, 2);
        if (position != string::npos) {
            path = path.substr(0, position) + path.substr(position + 1);
        }
    }

    // Remove trailing separator
    if (path[path.size() - 1] == PathSeparator) {
        path = path.substr(0, path.size() - 1);
    }

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
        int result = p1.compare(0, nextPosition, p2, 0 , nextPosition);
        if (result == 0) {
            currentPosition = nextPosition;
            nextPosition = p1.find(PathSeparator, nextPosition + 1);
        }
        else {
            break;
        }
    }
    int result = p1.compare(0, p1.length(), p2, 0 , p1.length());
    if (result == 0) {
        currentPosition = p1.length();
    }
    return currentPosition;
}

bool FileSystem::containsToken(const string& path) const {
    ghoul_assert(!path.empty(), "Path must not be empty");

    bool hasOpeningBrace = path.find(TokenOpeningBraces) != string::npos;
    bool hasClosingBrace = path.find(TokenClosingBraces) != string::npos;
    return hasOpeningBrace && hasClosingBrace;
}

bool FileSystem::hasToken(const string& path, const string& token) const {
    ghoul_assert(!path.empty(), "Path must not be empty");
    ghoul_assert(!token.empty(), "Token must not be empty");

    if (!containsToken(path)) {
        return false;
    }
    else {
        string::size_type beginning = path.find(TokenOpeningBraces);
        string::size_type closing = path.find(TokenClosingBraces);
        while ((beginning != string::npos) && (closing != string::npos)) {
            string::size_type closingLocation = closing + TokenClosingBracesSize;
            std::string currentToken = path.substr(beginning, closingLocation);
            if (currentToken == token) {
                return true;
            }
            else {
                beginning = path.find(TokenOpeningBraces, closing);
                closing = path.find(TokenClosingBraces, beginning);
            }
        }
        return false;
    }
}

string FileSystem::resolveToken(const string& token) const {
    ghoul_assert(!token.empty(), "Token must not be empty");

    auto it = _tokenMap.find(token);
    if (it == _tokenMap.end()) {
        throw ResolveTokenException(token);
    }
    else {
        return it->second;
    }
}

} // namespace ghoul::filesystem
