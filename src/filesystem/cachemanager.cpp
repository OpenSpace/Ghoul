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

#include <ghoul/fmt.h>
#include <ghoul/filesystem/file.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/assert.h>
#include <ghoul/misc/crc32.h>
#include <algorithm>
#include <chrono>
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

    // something that cannot occur in the filesystem
    constexpr const char HashDelimiter = '|';

    using LoadedCacheInfo = std::pair<unsigned int, std::filesystem::path>;

    unsigned int generateHash(std::filesystem::path file, std::string_view information) {
        std::string s = fmt::format("{}{}{}", file.string(), HashDelimiter, information);
        unsigned int hash = ghoul::hashCRC32(s);
        return hash;
    }

    std::string lastModifiedDate(std::filesystem::path path) {
        if (!std::filesystem::is_regular_file(path)) {
            throw ghoul::RuntimeError(fmt::format(
                "Error retrieving last-modified date for {}. File did not exist", path
            ));
        }
#ifdef WIN32
        WIN32_FILE_ATTRIBUTE_DATA infoData;
        BOOL success = GetFileAttributesEx(
            path.string().c_str(),
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
            throw ghoul::RuntimeError(fmt::format(
                "Could not retrieve last-modified date for file {}: {}", path, msg
            ));
        }
        else {
            FILETIME lastWriteTime = infoData.ftLastWriteTime;
            SYSTEMTIME time;
            //LPSYSTEMTIME time = NULL;
            bool success2 = FileTimeToSystemTime(&lastWriteTime, &time);
            if (!success2) {
                const DWORD error = GetLastError();
                LPTSTR errorBuffer = nullptr;
                FormatMessage(
                    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER |
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
                throw ghoul::RuntimeError(fmt::format(
                    "'FileTimeToSystemTime' failed for file {}: {}", path, msg
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
        stat(path.string().c_str(), &attrib);
        struct tm* time = gmtime(&(attrib.st_ctime));
        char buffer[128];
        strftime(buffer, 128, "%Y-%m-%dT%H:%M:%S", time);
        return buffer;
#endif
    }

    void cleanDirectory(const std::filesystem::path& path) {
        if (!std::filesystem::is_directory(path)) {
            return;
        }

        // First search for all subdirectories and call this function recursively on them
        namespace fs = std::filesystem;
        for (const fs::directory_entry& e : fs::directory_iterator(path)) {
            if (e.is_directory()) {
                cleanDirectory(e.path());
            }
        }

        // We get to this point in the recursion if either all subdirectories have been
        // deleted or there exists a file somewhere in the directory tree
        bool isEmpty = true;
        for (const fs::directory_entry& e : fs::directory_iterator(path)) {
            if (e.is_regular_file() || e.is_directory()) {
                isEmpty = false;
                break;
            }
        }

#ifdef __APPLE__
        // Apple stores the .DS_Store directory in the directory which can be removed
        std::string dsStore = FileSys.pathByAppendingComponent(dir, ".DS_Store");
        isEmpty |= ((contents.size() == 1) && contents[0] == dsStore);
#endif // __APPLE__
        // If this directory is empty, we can delete it
        if (isEmpty) {
            std::filesystem::remove(path);
        }
    }


    std::vector<LoadedCacheInfo> cacheInformationFromDirectory(
                                                        const std::filesystem::path& path)
    {
        std::vector<LoadedCacheInfo> result;
        namespace fs = std::filesystem;
        for (const fs::directory_entry& e : fs::directory_iterator(path)) {
            if (!e.is_directory()) {
                continue;
            }

            // Extract the name of the directory
            // +1 as the last path delimiter is missing from the path
            std::string directoryName = e.path().string().substr(path.string().size() + 1);

            for (const fs::directory_entry& f : fs::directory_iterator(e)) {
                const std::string hash = f.path().string();
                // Extract the hash from the directory name
                // +1 as the last path delimiter is missing from the path
                const std::string hashName = hash.substr(e.path().string().size() + 1);

                std::vector<fs::directory_entry> files;
                for (fs::directory_entry g : fs::directory_iterator(hash)) {
                    files.push_back(g);
                }

                // Cache directories should only contain a single file with the
                // same name as the directory
                if (files.size() > 1) {
                    throw ghoul::filesystem::CacheManager::ErrorLoadingCacheException(
                        fmt::format("Directory '{}' contained more than one file", hash)
                    );
                }
                if (files.size() == 1) {
                    // Extract the file name from the full path
                    // +1 as the last path delimiter is missing from the path
                    std::string filename = files[0].path().string().substr(hash.size() + 1);
                    if (filename != directoryName) {
                        throw ghoul::filesystem::CacheManager::ErrorLoadingCacheException(
                            fmt::format(
                                "File contained in cache directory '{}' contains a file "
                                "with name '{}' instead of expected '{}'",
                                hash, filename, directoryName
                            )
                        );
                    }

                    result.emplace_back(std::stoul(hashName), files[0].path());
                }
            }
        }

        return result;
    }
} // namespace

namespace ghoul::filesystem {

CacheManager::CacheException::CacheException(std::filesystem::path msg)
    : RuntimeError(msg.string(), "Cache")
{}

CacheManager::MalformedCacheException::MalformedCacheException(std::filesystem::path file)
    : CacheException(fmt::format("Malformed cache at {}", file))
    , cacheFile(std::move(file))
{}

CacheManager::ErrorLoadingCacheException::ErrorLoadingCacheException(
                                                                std::filesystem::path msg)
    : CacheException(std::move(msg))
{}

CacheManager::IllegalArgumentException::IllegalArgumentException(
                                                           std::filesystem::path argument)
    : CacheException(fmt::format("Argument '{}' contains an illegal character", argument))
    , argumentName(std::move(argument))
{}

CacheManager::CacheManager(std::filesystem::path directory, int version)
    : _directory(std::move(directory))
    , _version(version)
{
    ghoul_assert(!_directory.empty(), "Directory must not be empty");

    // In the cache state, we check our cache directory for all values, in a later step
    // we remove all persistent values, so that only the non-persistent values remain
    // Under normal operation, the resulting vector should be of size == 0, but if the
    // last execution of the application crashed, the directory was not cleaned up
    // properly
    std::vector<LoadedCacheInfo> cacheState = cacheInformationFromDirectory(_directory);
    const std::filesystem::path path = _directory / CacheFile;

    std::ifstream file(path);
    if (file.good()) {
        std::string line;
        // The first line of the file contains the version number
        std::getline(file, line);
        if (line != std::to_string(_version)) {
            LINFO(fmt::format(
                "Cache version has changed. Current version {}; new version {}",
                line, _version
            ));
            for (const LoadedCacheInfo& cache : cacheState) {
                LINFO(fmt::format("Deleting file '{}'", cache.second));
                if (std::filesystem::is_regular_file(cache.second)) {
                    std::filesystem::remove(cache.second);
                }
            }
            cleanDirectory(_directory);
            file.close();
            if (std::filesystem::is_regular_file(path)) {
                std::filesystem::remove(path);
            }
            return;
        }

        // loading the cache file that might exist from a previous run of the application
        // and reading all the persistent files from that file
        while (std::getline(file, line)) {
            // The structure of the cache file is assumed as follows (writting in the
            // destructor call of the application's previous run):
            // for each file:
            //   hash number\n
            //   filepath\n
            unsigned long hash;
            try {
                hash = std::stoul(line);
            }
            catch (const std::invalid_argument&) {
                throw MalformedCacheException(path);
            }

            //CacheInformation info;
            std::string filename;
            std::getline(file, filename);
            std::filesystem::path p = filename;
            //info.file = filename;
            if (!file.good()) {
                throw MalformedCacheException(path);
            }

            //info.isPersistent = true;
            _files[hash] = p;

            // If the current hash + file is contained in the cache state, we have to
            // remove it
            cacheState.erase(
                std::remove_if(
                    cacheState.begin(), cacheState.end(),
                    [hash, &p](const LoadedCacheInfo& i) {
                        return hash == i.first && p == i.second;
                    }
                ),
                cacheState.end()
            );
        }
    }

    // At this point all values that remain in the cache state vector are left from a
    // previous crash of the application
    if (!cacheState.empty()) {
        LINFO("There was a crash in the previous run and it left the cache unclean. "
              "Cleaning it now");
        for (const LoadedCacheInfo& cache : cacheState) {
            LINFO(fmt::format("Deleting file '{}'", cache.second));
            if (std::filesystem::is_regular_file(cache.second)) {
                std::filesystem::remove(cache.second);
            }
        }
        // First clean the cache directory with all contents
        cleanDirectory(_directory);
        file.close();
        if (std::filesystem::is_regular_file(path)) {
            std::filesystem::remove(path);
        }
        if (!std::filesystem::is_directory(_directory)) {
            // Then recreate the directory for further use
            std::filesystem::create_directory(_directory);
        }
    }
}

CacheManager::~CacheManager() {
    const std::filesystem::path path = _directory / CacheFile;
    std::ofstream file(path, std::ofstream::out);
    if (file.good()) {
        file << _version << '\n';
        for (const std::pair<const unsigned long, std::filesystem::path>& p : _files) {
            // Save the persistent files in the cache file
            file << p.first << '\n' << p.second.string() << '\n';
        }
        file.close();
    }
    else {
        LERROR(fmt::format("Could not open {} for writing permanent cache files", path));
    }
    cleanDirectory(_directory);
}

std::string CacheManager::cachedFilename(const std::filesystem::path& file,
                                         std::optional<std::string_view> information)
{
    std::string info;
    if (information.has_value()) {
        info = std::string(*information);
    }
    else {
        info = lastModifiedDate(file);
    }

    const std::filesystem::path baseName = file.filename();
    const size_t pos = baseName.string().find_first_of("/\\?%*:|\"<>");
    if (pos != std::string::npos) {
        throw IllegalArgumentException(baseName);
    }

    const unsigned int hash = generateHash(baseName, info);

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

    auto it = _files.find(hash);
    if (it != _files.end()) {
        // If we find the hash, it has been created before and we can just return the
        // file name to the caller
        return it->second.string();
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
    std::string info;
    if (information.has_value()) {
        info = std::string(*information);
    }
    else {
        info = lastModifiedDate(file);
    }

    const std::filesystem::path baseName = file.filename();
    const size_t pos = baseName.string().find_first_of("/\\?%*:|\"<>");
    if (pos != std::string::npos) {
        throw IllegalArgumentException(baseName);
    }

    const unsigned long hash = generateHash(baseName, info);
    return _files.find(hash) != _files.end();
}

void CacheManager::removeCacheFile(const std::filesystem::path& file,
                                   std::optional<std::string_view> information)
{
    std::string info;
    if (information.has_value()) {
        info = std::string(*information);
    }
    else {
        info = lastModifiedDate(file);
    }

    const std::filesystem::path baseName = file.filename();
    const size_t pos = baseName.string().find_first_of("/\\?%*:|\"<>");
    if (pos != std::string::npos) {
        throw IllegalArgumentException(baseName);
    }

    const unsigned int hash = generateHash(baseName, info);
    auto it = _files.find(hash);
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
