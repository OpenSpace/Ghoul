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

#include <ghoul/filesystem/filesystem.h>

#include <ghoul/fmt.h>
#include <ghoul/filesystem/cachemanager.h>
#include <ghoul/filesystem/file.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/assert.h>
#include <ghoul/misc/profiling.h>
#include <algorithm>
#include <filesystem>
#include <regex>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#include <direct.h>
#include <Shlwapi.h>
#include <windows.h>
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

FileSystem* FileSystem::_instance = nullptr;

FileSystem::FileSystemException::FileSystemException(string msg)
    : RuntimeError(std::move(msg), "FileSystem")
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

void FileSystem::initialize() {
    ghoul_assert(!isInitialized(), "FileSystem is already initialized");
    _instance = new FileSystem;
}

void FileSystem::deinitialize() {
    ghoul_assert(isInitialized(), "FileSystem is not initialized");
    delete _instance;
    _instance = nullptr;
}

bool FileSystem::isInitialized() {
    return _instance != nullptr;
}

FileSystem& FileSystem::ref() {
    ghoul_assert(isInitialized(), "FileSystem is not initialized");
    return *_instance;
}

string FileSystem::absolutePath(string path) const {
    ghoul_assert(!path.empty(), "Path must not be empty");
    expandPathTokens(path);
    std::filesystem::path p = std::filesystem::absolute(path);
    return p.string();
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
    ghoul_assert(
        std::filesystem::is_directory(cacheDirectory.path()),
        "Cache directory did not exist"
    );
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
    ZoneScoped

#ifdef WIN32
    // Sleeping for 0 milliseconds will trigger any pending asynchronous procedure calls
    SleepEx(0, TRUE);
#endif
#if defined(__APPLE__)
    triggerFilesystemEventsInternalApple();
#endif
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
