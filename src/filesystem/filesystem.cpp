/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2022                                                               *
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
#include <ghoul/misc/profiling.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#include <direct.h>
#include <Shlwapi.h>
#include <Windows.h>
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

namespace {
    constexpr const char* _loggerCat = "FileSystem";
} // namespace

std::filesystem::path absPath(std::string path) {
    ghoul_assert(!path.empty(), "Path must not be empty");
    std::filesystem::path expanded = FileSys.expandPathTokens(std::move(path));
    std::filesystem::path absolute = std::filesystem::absolute(std::move(expanded));
    return absolute.lexically_normal();
}

std::filesystem::path absPath(std::filesystem::path path) {
    return absPath(path.string());
}

std::filesystem::path absPath(const char* path) {
    return absPath(std::string(path));
}


namespace ghoul::filesystem {

FileSystem* FileSystem::_instance = nullptr;

FileSystem::FileSystem() {
    std::filesystem::path temporaryPath = std::filesystem::temp_directory_path();
    LINFO(fmt::format("Set temporary path ${{TEMPORARY}} to {}", temporaryPath));
    registerPathToken("${TEMPORARY}", temporaryPath);

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

void FileSystem::registerPathToken(std::string token, std::filesystem::path path,
                                   Override override)
{
    ghoul_assert(!token.empty(), "Token must not be empty");
    ghoul_assert(token.find("${") == 0, "Token must start with '${'");
    ghoul_assert(token[token.size() - 1] == '}', "Token must end with '}'");

    if (!override) {
        ghoul_assert(
            _tokenMap.find(token) == _tokenMap.end(),
            "Token must not have been registered before"
        );
    }

    if (override) {
        const auto it = _tokenMap.find(token);
        if (it != _tokenMap.end()) {
            _tokenMap.erase(it);
        }
    }
    _tokenMap[std::move(token)] = std::move(path);
}

std::filesystem::path FileSystem::expandPathTokens(std::string path,
                                      const std::vector<std::string>& ignoredTokens) const
{
    // TokenInformation = <token, beginning position, length>
    struct TokenInformation {
        std::string token;
        size_t beginning;
        size_t length;
    };
    auto nextToken = [ignoredTokens, &path]() {
        size_t currentPosition = 0;
        while (true) {
            const size_t beginning = path.find("${", currentPosition);
            const size_t closing = path.find("}", beginning + 2);
            const size_t closingLocation = closing + 1;

            if (beginning == std::string::npos || closing == std::string::npos) {
                // There is no token left
                return TokenInformation{ "", std::string::npos, 0 };
            }

            const std::string token = path.substr(beginning, closingLocation - beginning);
            const auto it = std::find(ignoredTokens.begin(), ignoredTokens.end(), token);
            if (it != ignoredTokens.end()) {
                // The found token is an ignored one
                currentPosition = closingLocation;
                continue;
            }
            else {
                return TokenInformation{ token, beginning, closingLocation - beginning };
            }
        }
    };

    while (true) {
        TokenInformation tokenInformation = nextToken();
        if (tokenInformation.token.empty()) {
            break;
        }

        const auto it = _tokenMap.find(tokenInformation.token);
        if (it == _tokenMap.end()) {
            throw RuntimeError(
                fmt::format("Token '{}' could not be resolved", tokenInformation.token),
                "FileSystem"
            );
        }

        std::string replacement = it->second.string();
        path.replace(tokenInformation.beginning, tokenInformation.length, replacement);
    }

    return path;
}

std::vector<std::string> FileSystem::tokens() const {
    std::vector<std::string> tokens;
    tokens.reserve(_tokenMap.size());
    for (const std::pair<const std::string, std::filesystem::path>& token : _tokenMap) {
        tokens.push_back(token.first);
    }
    return tokens;
}

bool FileSystem::hasRegisteredToken(const std::string& token) const {
    return _tokenMap.find(token) != _tokenMap.end();
}

bool FileSystem::containsToken(const std::string& path) const {
    ghoul_assert(!path.empty(), "Path must not be empty");

    bool hasOpeningBrace = path.find("${") != std::string::npos;
    bool hasClosingBrace = path.find("}") != std::string::npos;
    return hasOpeningBrace && hasClosingBrace;
}

void FileSystem::createCacheManager(const std::filesystem::path& directory) {
    ghoul_assert(
        std::filesystem::is_directory(directory),
        "Cache directory did not exist"
    );
    ghoul_assert(!_cacheManager, "CacheManager was already created");

    _cacheManager = std::make_unique<CacheManager>(directory.string());
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
}

} // namespace ghoul::filesystem
