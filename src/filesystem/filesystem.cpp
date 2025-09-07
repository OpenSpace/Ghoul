/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2025                                                               *
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
#include <ghoul/misc/defer.h>
#include <ghoul/misc/profiling.h>

#ifdef WIN32
#include <Windows.h>
#include <ShObjIdl.h>
#include <ShlGuid.h>
#else // ^^^^ WIN32 // !WIN32 vvvv
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif // WIN32

#if !defined(WIN32) && !defined(__APPLE__)
#include <sys/inotify.h>
#endif // !defined(WIN32) && !defined(__APPLE__)

namespace {
    constexpr std::string_view _loggerCat = "FileSystem";
} // namespace

std::filesystem::path absPath(std::string path) {
    ghoul_assert(!path.empty(), "Path must not be empty");
    const std::filesystem::path expanded = FileSys.expandPathTokens(std::move(path));
    const std::filesystem::path absolute = std::filesystem::absolute(expanded);
    return absolute.lexically_normal();
}

std::filesystem::path absPath(const std::filesystem::path& path) {
    return absPath(path.string());
}

std::filesystem::path absPath(const char* path) {
    return absPath(std::string(path));
}

namespace ghoul::filesystem {

FileSystem* FileSystem::_instance = nullptr;

FileSystem::FileSystem() {
    std::filesystem::path temporaryPath = std::filesystem::temp_directory_path();
    LINFO(std::format("Set temporary path ${{TEMPORARY}} to '{}'", temporaryPath));
    registerPathToken("${TEMPORARY}", temporaryPath);

#if !defined(WIN32) && !defined(__APPLE__)
    // This code can't be moved to the initializer list as the inotifyWatcher function
    // passed into the thread will be called immediately. This will try to access the
    // FileSys global and lead to an assertion as the global object has not yet finished
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
            const size_t closing = path.find('}', beginning + 2);
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
                std::format("Token '{}' could not be resolved", tokenInformation.token),
                "FileSystem"
            );
        }

        const std::string replacement = it->second.string();
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

    const bool hasOpeningBrace = path.find("${") != std::string::npos;
    const bool hasClosingBrace = path.find('}') != std::string::npos;
    return hasOpeningBrace && hasClosingBrace;
}

void FileSystem::createCacheManager(const std::filesystem::path& directory) {
    ghoul_assert(
        std::filesystem::is_directory(directory),
        "Cache directory did not exist"
    );
    ghoul_assert(!_cacheManager, "CacheManager was already created");

    _cacheManager = std::make_unique<CacheManager>(directory);
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
    ZoneScoped;

#ifdef WIN32
    // Sleeping for 0 milliseconds will trigger any pending asynchronous procedure calls
    SleepEx(0, TRUE);
#endif
}


#ifdef WIN32
std::filesystem::path FileSystem::resolveShellLink(std::filesystem::path path) {
    IShellLink* psl = nullptr;
    HRESULT hres = CoCreateInstance(
        CLSID_ShellLink,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IShellLink,
        reinterpret_cast<LPVOID*>(&psl)
    );
    if (FAILED(hres)) {
        throw ghoul::RuntimeError(std::format(
            "Failed initializing ShellLink when resolving path '{}' with error: {}",
            path, hres
        ));
    }
    defer { psl->Release(); };

    IPersistFile* ppf = nullptr;
    hres = psl->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&ppf));
    if (FAILED(hres)) {
        throw ghoul::RuntimeError(std::format(
            "Failed querying interface when resolving path '{}' with error: {}",
            path, hres
        ));
    }
    defer{ ppf->Release(); };

    WCHAR wsz[MAX_PATH];
    std::string p = path.string();
    int res = MultiByteToWideChar(CP_ACP, 0, p.c_str(), -1, wsz, MAX_PATH);
    if (res == 0) {
        DWORD error = GetLastError();
        throw ghoul::RuntimeError(std::format(
            "Failed converting path '{}' with error: {}", path, error
        ));
    }

    hres = ppf->Load(wsz, STGM_READ);
    if (FAILED(hres)) {
        throw ghoul::RuntimeError(std::format(
            "Failed loading ShellLink file at path '{}' with error: {}", path, hres
        ));
    }

    hres = psl->Resolve(nullptr, 0);
    if (FAILED(hres)) {
        throw ghoul::RuntimeError(std::format(
            "Failed to resolve ShellLink at path '{}' with error: {}", path, hres
        ));
    }

    CHAR szGotPath[MAX_PATH];
    WIN32_FIND_DATA wfd;
    hres = psl->GetPath(szGotPath, MAX_PATH, &wfd, SLGP_SHORTPATH);
    if (FAILED(hres)) {
        throw ghoul::RuntimeError(std::format(
            "Failed to get path of ShellLink at path '{}' with error: {}", path, hres
        ));
    }

    std::string result = szGotPath;
    return result;
}
#endif // WIN32

std::vector<std::filesystem::path> walkDirectory(const std::filesystem::path& path,
                                                 Recursive recursive, Sorted sorted,
                                 std::function<bool(const std::filesystem::path&)> filter)
{
    ghoul_assert(std::filesystem::exists(path), "Path does not exist");
    ghoul_assert(std::filesystem::is_directory(path), "Path is not a directory");

    namespace fs = std::filesystem;
    std::vector<std::filesystem::path> result;
    try {
        if (fs::is_directory(path)) {
            if (recursive) {
                for (fs::directory_entry e : fs::recursive_directory_iterator(path)) {
                    if (filter(e)) {
                        result.push_back(e.path());
                    }
                }
            }
            else {
                for (fs::directory_entry e : fs::directory_iterator(path)) {
                    if (filter(e)) {
                        result.push_back(e.path());
                    }
                }
            }
        }
        if (sorted) {
            std::sort(result.begin(), result.end());
        }
    }
    catch (const fs::filesystem_error& e) {
        LERROR(std::format(
            "Failed accessing directory {}, with error: {}", path, e.what()
        ));
        return {};
    }

    return result;
}

bool isSubdirectory(std::filesystem::path p, std::filesystem::path root) {
    while (p != p.parent_path()) {
        if (p == root) {
            return true;
        }
        p = p.parent_path();
    }
    return false;
}

} // namespace ghoul::filesystem
