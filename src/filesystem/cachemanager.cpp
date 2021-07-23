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

#include <ghoul/filesystem/cachemanager.h>

#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/crc32.h>
#include <fstream>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#include <Windows.h>
#else // ^^^^ WIN32 // !WIN32 vvvv
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#endif // WIN32

namespace {
    constexpr const char* _loggerCat = "CacheManager";
    const std::filesystem::path CacheFile = "cache";
    constexpr const int CacheVersion = 2;

    using LoadedCacheInfo = std::pair<unsigned long, std::filesystem::path>;

    unsigned int generateHash(std::filesystem::path file, std::string_view information) {
        // something that cannot occur in the filesystem
        constexpr const char HashDelimiter = '|';

        std::string s = fmt::format("{}{}{}", file.string(), HashDelimiter, information);
        unsigned int hash = ghoul::hashCRC32(s);
        return hash;
    }

    std::string lastModifiedDate(std::filesystem::path path) {
        if (!std::filesystem::is_regular_file(path)) {
            throw ghoul::RuntimeError(
                fmt::format(
                    "Error retrieving last-modified date for {}. File did not exist", path
                ),
                "Cache"
            );
        }
#ifdef WIN32
        WIN32_FILE_ATTRIBUTE_DATA infoData;
        const BOOL success = GetFileAttributesEx(
            path.string().c_str(),
            GetFileExInfoStandard,
            &infoData
        );
        if (!success) {
            const DWORD error = GetLastError();
            std::vector<char> buffer(1024, '\0');
            FormatMessage(
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                error,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                buffer.data(),
                0,
                nullptr
            );
            std::string msg(buffer.data());
            throw ghoul::RuntimeError(
                fmt::format(
                    "Could not retrieve last-modified date for {}: {}", path, msg
                ),
                "Cache"
            );
        }
        else {
            SYSTEMTIME time;
            const BOOL success2 = FileTimeToSystemTime(&infoData.ftLastWriteTime, &time);
            if (!success2) {
                const DWORD error = GetLastError();
                std::vector<char> buffer(1024, '\0');
                FormatMessage(
                    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    nullptr,
                    error,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    buffer.data(),
                    0,
                    nullptr
                );
                std::string msg(buffer.data());
                throw ghoul::RuntimeError(
                    fmt::format("'FileTimeToSystemTime' failed for {}: {}", path, msg),
                    "Cache"
                );
            }
            else {
                return fmt::format(
                    "{}-{}-{}T{}:{}:{}.{}", time.wYear, time.wMonth, time.wDay,
                    time.wHour, time.wMinute, time.wSecond, time.wMilliseconds
                );
            }
        }
#else // ^^^^ WIN32 // !WIN32 vvvv
        struct stat attrib;
        stat(path.string().c_str(), &attrib);
        struct tm* time = gmtime(&attrib.st_ctime);
        char buffer[128];
        strftime(buffer, 128, "%Y-%m-%dT%H:%M:%S", time);
        return buffer;
#endif // WIN32
    }

    // List all of the <path, hash> pairs that are stored in the cache directory pointed
    // to by path
    std::map<unsigned long, std::filesystem::path> cacheInfoFromDirectory(
                                                        const std::filesystem::path& path)
    {
        std::map<unsigned long, std::filesystem::path> result;
        namespace fs = std::filesystem;
        for (const fs::directory_entry& e : fs::recursive_directory_iterator(path)) {
            if (!e.is_regular_file() || e.path().filename() == CacheFile) {
                continue;
            }

            fs::path thisFilename = e.path().filename();
            fs::path hashName = e.path().parent_path().filename();
            fs::path parentFilename = e.path().parent_path().parent_path().filename();

            if (thisFilename != parentFilename) {
                throw ghoul::RuntimeError(
                    fmt::format(
                        "File contained in cache directory {} contains a file "
                        "with name {} instead of expected {}",
                        path, thisFilename, parentFilename
                    ),
                    "Cache"
                );
            }

            unsigned long hash = std::stoul(hashName.string());
            result[hash] = e.path();
        }

        return result;
    }
} // namespace

namespace ghoul::filesystem {

CacheManager::CacheManager(std::filesystem::path directory)
    : _directory(std::move(directory))
{
    ghoul_assert(std::filesystem::is_directory(_directory), "Directory must exit");

    const std::filesystem::path cacheFile = _directory / CacheFile;
    std::ifstream file(cacheFile);
    if (file.good()) {
        std::string line;
        std::getline(file, line);
        if (line != std::to_string(CacheVersion)) {
            LINFO(fmt::format(
                "Cache version has changed. Current: {}; New: {}", line, CacheVersion
            ));
            file.close();
            std::filesystem::remove_all(_directory);
            std::filesystem::create_directory(_directory);
        }
    }

    // In the cache state, we check our cache directory for all values, in a later step
    // we remove all persistent values, so that only the non-persistent values remain
    // Under normal operation, the resulting vector should be of size == 0, but if the
    // last execution of the application crashed, the directory was not cleaned up
    // properly
    _files = cacheInfoFromDirectory(_directory);
}

CacheManager::~CacheManager() {
    const std::filesystem::path path = _directory / CacheFile;
    std::ofstream file(path, std::ofstream::out);
    if (!file.good()) {
        LERROR(fmt::format("Could not open {} for writing cache version file", path));
    }

    file << CacheVersion;
}

std::filesystem::path CacheManager::cachedFilename(const std::filesystem::path& file,
                                              std::optional<std::string_view> information)
{
    const std::filesystem::path baseName = file.filename();
    const size_t pos = baseName.string().find_first_of("/\\?%*:|\"<>");
    if (pos != std::string::npos) {
        throw ghoul::RuntimeError(
            fmt::format("Argument {} contains an illegal character", file.filename()),
            "Cache"
        );
    }

    std::string lastModified = lastModifiedDate(file);
    const unsigned int hash = generateHash(
        baseName,
        information.has_value() ? *information : lastModified
    );

    // If we couldn't find the file, we have to generate a directory with the name of the
    // hash and return the full path containing of the cache path + filename + hash value
    const std::filesystem::path destinationBase = _directory / baseName;

    // If this is the first time anyone requests a cache for the file name, we have to
    // create the base directory under the cache directory
    if (!std::filesystem::is_directory(destinationBase)) {
        std::filesystem::create_directory(destinationBase);
    }

    const std::string destination = fmt::format("{}/{}", destinationBase.string(), hash);

    // The new destination should always not exist, since we checked before if we have the
    // value in the map and only get here if it isn't; persistent cache entries are always
    // in the map and non-persistent entries should have been deleted on application close
    if (!std::filesystem::is_directory(destination)) {
        std::filesystem::create_directory(destination);
    }

    const auto it = _files.find(hash);
    if (it != _files.cend()) {
        // If we find the hash, it has been created before and we can just return the
        // file name to the caller
        return it->second;
    }

    // Generate and output the newly generated cache name
    const std::string cachedName = fmt::format("{}/{}", destination, baseName.string());

    // Store the cache information in the map
    _files[hash] = cachedName;
    return cachedName;
}

bool CacheManager::hasCachedFile(const std::filesystem::path& file,
                                 std::optional<std::string_view> information) const
{
    const std::filesystem::path baseName = file.filename();
    const size_t pos = baseName.string().find_first_of("/\\?%*:|\"<>");
    if (pos != std::string::npos) {
        throw ghoul::RuntimeError(
            fmt::format("Argument {} contains an illegal character", file.filename()),
            "Cache"
        );
    }

    std::string lastModified = lastModifiedDate(file);
    const unsigned long hash = generateHash(
        baseName,
        information.has_value() ? *information : lastModified
    );
    return _files.find(hash) != _files.end();
}

void CacheManager::removeCacheFile(const std::filesystem::path& file,
                                   std::optional<std::string_view> information)
{
    const std::filesystem::path baseName = file.filename();
    const size_t pos = baseName.string().find_first_of("/\\?%*:|\"<>");
    if (pos != std::string::npos) {
        throw ghoul::RuntimeError(
            fmt::format("Argument {} contains an illegal character", file.filename()),
            "Cache"
        );
    }

    std::string lastModified = lastModifiedDate(file);
    const unsigned int hash = generateHash(
        baseName,
        information.has_value() ? *information : lastModified
    );
    const auto it = _files.find(hash);
    if (it != _files.end()) {
        // If we find the hash, it has been created before and we can just return the
        // file name to the caller
        if (std::filesystem::is_regular_file(it->second)) {
            std::filesystem::remove(it->second);
        }
        _files.erase(it);
    }
}

} // namespace ghoul::filesystem
