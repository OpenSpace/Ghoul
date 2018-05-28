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

#include <ghoul/filesystem/cachemanager.h>

#include <ghoul/filesystem/file.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/crc32.h>
#include <ghoul/fmt.h>
#include <algorithm>
#include <fstream>

namespace {
    constexpr const char* _loggerCat = "CacheManager";
    constexpr const char* _cacheFile = "cache";

    // something that cannot occur in the filesystem
    constexpr const char _hashDelimiter = '|';

    unsigned int generateHash(const std::string& file, const std::string& information) {
        std::string hashString = file + _hashDelimiter + information;
        unsigned int hash = ghoul::hashCRC32(hashString);
        return hash;
    }

} // namespace

namespace ghoul::filesystem {

CacheManager::CacheException::CacheException(std::string msg)
    : RuntimeError(std::move(msg), "Cache")
{}

CacheManager::MalformedCacheException::MalformedCacheException(std::string file,
                                                               std::string msg)
    : CacheException(fmt::format("Malformed cache at '{}'", file))
    , cacheFile(std::move(file))
    , message(std::move(msg))
{}

CacheManager::ErrorLoadingCacheException::ErrorLoadingCacheException(std::string msg)
    : CacheException(std::move(msg))
{}

CacheManager::IllegalArgumentException::IllegalArgumentException(std::string argument)
    : CacheException(fmt::format("Argument '{}' contains an illegal character", argument))
    , argumentName(std::move(argument))
{}

CacheManager::CacheManager(std::string directory, int version)
    : _version(version)
{
    ghoul_assert(!directory.empty(), "Directory must not be empty");
    _directory = std::move(directory);

    // In the cache state, we check our cache directory for all values, in a later step
    // we remove all persistent values, so that only the non-persistent values remain
    // Under normal operation, the resulting vector should be of size == 0, but if the
    // last execution of the application crashed, the directory was not cleaned up
    // properly
    std::vector<LoadedCacheInfo> cacheState = cacheInformationFromDirectory(_directory);
    std::string path = FileSys.pathByAppendingComponent(_directory, _cacheFile);

    std::ifstream file(path);
    if (file.good()) {
        std::string line;
        // The first line of the file contains the version number
        std::getline(file, line);
        if (line != std::to_string(_version)) {
            LINFO(fmt::format(
                "Cache version has changed. Current version {}; new version {}",
                line,
                _version
            ));
            for (const LoadedCacheInfo& cache : cacheState) {
                LINFO(fmt::format("Deleting file '{}'", cache.second));
                FileSys.deleteFile(cache.second);
            }
            cleanDirectory(_directory);
            file.close();
            FileSys.deleteFile(path);
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
            } catch (const std::invalid_argument&) {
                throw MalformedCacheException(path);
            }

            CacheInformation info;
            std::getline(file, info.file);
            if (!file.good()) {
                throw MalformedCacheException(path);
            }

            info.isPersistent = true;
            _files[hash] = info;

            // If the current hash + file is contained in the cache state, we have to
            // remove it
            cacheState.erase(
                std::remove_if(
                    cacheState.begin(), cacheState.end(),
                    [hash, &info](const LoadedCacheInfo& i) {
                        return hash == i.first && info.file == i.second;
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
            FileSys.deleteFile(cache.second);
        }
        // First clean the cache directory with all contents
        cleanDirectory(_directory);
        //FileSys.deleteDirectory(_directory, true);
        file.close();
        FileSys.deleteFile(path);
        if (!FileSys.directoryExists(_directory)) {
            // Then recreate the directory for further use
            FileSys.createDirectory(_directory);
        }
    }
}

CacheManager::~CacheManager() {
    std::string path = FileSys.pathByAppendingComponent(_directory, _cacheFile);
    std::ofstream file(path, std::ofstream::out);
    if (file.good()) {
        file << _version << std::endl;
        for (const std::pair<unsigned int, CacheInformation>& p : _files) {
            if (!p.second.isPersistent) {
                // Delete all the non-persistent files
                FileSys.deleteFile(p.second.file);
            }
            else {
                // Save the persistent files in the cache file
                file << p.first << '\n' << p.second.file << '\n';
            }
        }
        file.close();
    }
    else {
        LERROR(fmt::format(
            "Could not open file '{}' for writing parmanent cache files",
            path
        ));
    }
    cleanDirectory(_directory);
}

std::string CacheManager::cachedFilename(const File& file, Persistent isPersistent) {
    std::string lastModifiedTime = file.lastModifiedDate();
    return cachedFilename(file, lastModifiedTime, isPersistent);
}

std::string CacheManager::cachedFilename(const File& file, const std::string& information,
                                         Persistent isPersistent)
{
    return cachedFilename(file.filename(), information, isPersistent);
}

std::string CacheManager::cachedFilename(const std::string& baseName,
                                         const std::string& information,
                                         Persistent isPersistent)
{
    size_t pos = baseName.find_first_of("/\\?%*:|\"<>");
    if (pos != std::string::npos) {
        throw IllegalArgumentException(baseName);
    }

    unsigned int hash = generateHash(baseName, information);

    // If we couldn't find the file, we have to generate a directory with the name of the
    // hash and return the full path containing of the cache path + requested filename +
    // hash value
    std::string destinationBase = FileSys.pathByAppendingComponent(_directory, baseName);

    // If this is the first time anyone requests a cache for the file name, we have to
    // create the base directory under the cache directory
    if (!FileSys.directoryExists(destinationBase)) {
        FileSys.createDirectory(destinationBase);
    }

    std::string destination = FileSys.pathByAppendingComponent(
        destinationBase,
        std::to_string(hash)
    );

    // The new destination should always not exist, since we checked before if we have the
    // value in the map and only get here if it isn't; persistent cache entries are always
    // in the map and non-persistent entries should have been deleted on application close
    if (!FileSys.directoryExists(destination)) {
        FileSys.createDirectory(destination);
    }

    auto it = _files.find(hash);
    if (it != _files.end()) {
        // If we find the hash, it has been created before and we can just return the
        // file name to the caller
        return it->second.file;
    }

    // Generate and output the newly generated cache name
    std::string cachedFileName = FileSys.pathByAppendingComponent(destination, baseName);

    // Store the cache information in the map
    CacheInformation info = {
        cachedFileName,
        isPersistent
    };
    _files[hash] = info;
    return cachedFileName;
}

bool CacheManager::hasCachedFile(const File& file) const {
    std::string lastModifiedTime = file.lastModifiedDate();
    return hasCachedFile(file, lastModifiedTime);
}

bool CacheManager::hasCachedFile(const File& file, const std::string& information) const {
    return hasCachedFile(file.filename(), information);
}

bool CacheManager::hasCachedFile(const std::string& baseName,
                                 const std::string& information) const
{
    size_t pos = baseName.find_first_of("/\\?%*:|\"<>");
    if (pos != std::string::npos) {
        throw IllegalArgumentException(baseName);
    }

    unsigned int hash = generateHash(baseName, information);
    return _files.find(hash) != _files.end();
}

void CacheManager::removeCacheFile(const File& file) {
    std::string lastModifiedTime = file.lastModifiedDate();
    removeCacheFile(file, lastModifiedTime);
}

void CacheManager::removeCacheFile(const File& file, const std::string& information) {
    removeCacheFile(file.filename(), information);
}

void CacheManager::removeCacheFile(const std::string& baseName,
                                   const std::string& information)
{
    size_t pos = baseName.find_first_of("/\\?%*:|\"<>");
    if (pos != std::string::npos) {
        throw IllegalArgumentException(baseName);
    }

    unsigned int hash = generateHash(baseName, information);

    auto it = _files.find(hash);
    if (it != _files.end()) {
        // If we find the hash, it has been created before and we can just return the
        // file name to the caller
        const std::string& cachedFileName = it->second.file;
        FileSys.deleteFile(cachedFileName);
        _files.erase(it);
    }
}

void CacheManager::cleanDirectory(const Directory& dir) const {
    // First search for all subdirectories and call this function recursively on them
    std::vector<std::string> contents = dir.readDirectories();
    for (const std::string& content : contents) {
        if (FileSys.directoryExists(content)) {
            cleanDirectory(content);
        }
    }
    // We get to this point in the recursion if either all subdirectories have been
    // deleted or there exists a file somewhere in the directory tree

    contents = dir.read();
    bool isEmpty = contents.empty();
#ifdef __APPLE__
    // Apple stores the .DS_Store directory in the directory which can be removed
    std::string dsStore = FileSys.pathByAppendingComponent(dir, ".DS_Store");
    isEmpty |= ((contents.size() == 1) && contents[0] == dsStore);
#endif
    // If this directory is empty, we can delete it
    if (isEmpty) {
        FileSys.deleteDirectory(dir, FileSystem::Recursive::Yes);
    }
}

std::vector<CacheManager::LoadedCacheInfo> CacheManager::cacheInformationFromDirectory(
                                                               const Directory& dir) const
{
    std::vector<LoadedCacheInfo> result;
    std::vector<std::string> directories = dir.readDirectories();
    for (const std::string& directory : directories) {
        Directory d(directory);

        // Extract the name of the directory
        // +1 as the last path delimiter is missing from the path
        std::string directoryName = directory.substr(dir.path().size() + 1);

        std::vector<std::string> hashes = d.readDirectories();
        for (const std::string& hash : hashes) {
            // Extract the hash from the directory name
            // +1 as the last path delimiter is missing from the path
            std::string hashName = hash.substr(d.path().size() + 1);

            std::vector<std::string> files = Directory(hash).readFiles();
            // Cache directories should only contain a single file with the
            // same name as the directory
            if (files.size() > 1) {
                throw ErrorLoadingCacheException(fmt::format(
                    "Directory '{}' contained more than one file",
                    hash
                ));
            }
            if (files.size() == 1) {
                // Extract the file name from the full path
                // +1 as the last path delimiter is missing from the path
                std::string filename = files[0].substr(Directory(hash).path().size() + 1);
                if (filename != directoryName) {
                    throw ErrorLoadingCacheException(fmt::format(
                        "File contained in cache directory '{}' contains a file with "
                        "name '{}' instead of expected '{}'",
                        hash,
                        filename,
                        directoryName
                    ));
                }

                result.emplace_back(std::stoul(hashName), files[0]);
            }
        }
    }

    return result;
}

} // namespace ghoul::filesystem
