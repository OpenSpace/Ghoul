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

#include <ghoul/filesystem/cachemanager.h>

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/crc32.h>

#include <algorithm>
#include <assert.h>
#include <fstream>
#include <string>

namespace {
	const std::string _loggerCat = "CacheManager";
	const std::string _cacheFile = "cache";
	const char _hashDelimiter = '|'; // something that cannot occur in the filesystem
}

namespace ghoul {
namespace filesystem {

CacheManager::CacheManager(std::string directory)
    : _directory(std::move(directory))
{
    // In the cache state, we check our cache directory for all values, in a later step
    // we remove all persistent values, so that only the non-persistent values remain
    // Under normal operation, the resulting vector should be of size == 0, but if the
    // last execution of the application crashed, the directory was not cleaned up
    // properly
    std::vector<LoadedCacheInfo> cacheState = cacheInformationFromDirectory(_directory);
	std::string&& path = FileSys.pathByAppendingComponent(_directory, _cacheFile);

	std::ifstream file(path);
	if (file.good()) {
		std::string line;
        // loading the cache file that might exist from a previous run of the application
        // and reading all the persistent files from that file
		while (std::getline(file, line)) {
            // The structure of the cache file is assumed as follows (writting in the
            // destructor call of the application's previous run):
            // for each file:
            //   hash number\n
            //   filepath\n
			std::stringstream s(line);

			unsigned int hash;
			s >> hash;

			CacheInformation info;
			std::getline(file, info.file);
			info.isPersistent = true;
			_files.emplace(hash, info);

            // If the current hash + file is contained in the cache state, we have to
            // remove it
			cacheState.erase(std::remove_if(cacheState.begin(), cacheState.end(), 
				[hash, &info](const LoadedCacheInfo& i) {
					return hash == i.first && info.file == i.second;
				}), cacheState.end());
		}
	}
    
    // At this point all values that remain in the cache state vector are left from a
    // previous crash of the application
    if (!cacheState.empty()) {
        LINFO("There was a crash in the previous run and it left the cache unclean. "
              "Cleaning it now");
        for (auto cache : cacheState) {
            LINFO("Deleting file '" << cache.second << "'");
            FileSys.deleteFile(cache.second);
        }
        cleanDirectory(_directory);
    }
}

CacheManager::~CacheManager() {
	std::string&& path = FileSys.pathByAppendingComponent(_directory, _cacheFile);
	std::ofstream file(path, std::ofstream::out);
	if (file.good()) {
		for (auto p : _files) {
			if (!p.second.isPersistent) {
                // Delete all the non-persistent files
				if (FileSys.fileExists(p.second.file))
					FileSys.deleteFile(p.second.file);
			}
			else
                // Save the persistent files in the cache file
				file << p.first << std::endl <<
						p.second.file << std::endl;
		}
		file.close();
	}
	else
		LERROR("Could not open file '" << path << "' for writing permanent cache files");
	cleanDirectory(_directory);
}

bool CacheManager::getCachedFile(const File& file, std::string& cachedFileName,
	bool isPersistent)
{
	std::string lastModifiedTime = file.lastModifiedDate();
	return getCachedFile(file, lastModifiedTime, cachedFileName, isPersistent);
}


bool CacheManager::getCachedFile(const File& file, const std::string& information,
                                 std::string &cachedFileName, bool isPersistent)
{
    return getCachedFile(file.filename(), information, cachedFileName, isPersistent);
}
    
bool CacheManager::getCachedFile(const std::string& baseName,
                                 const std::string& information,
                                 std::string& cachedFileName, bool isPersistent)
{
    size_t pos = baseName.find_first_of("/\\?%*:|\"<>");
    if (pos != std::string::npos) {
        LERROR("Base name '" << baseName << "' consists of illegal character");
        return false;
    }
    
	unsigned int hash = generateHash(baseName, information);

    // If we couldn't find the file, we have to generate a directory with the name of the
    // hash and return the full path containing of the cache path + requested filename +
    // hash value
	std::string destinationBase = FileSys.pathByAppendingComponent(_directory, baseName);

    // If this is the first time anyone requests a cache for the file name, we have to
    // create the base directory under the cache directory
	if (!FileSys.directoryExists(destinationBase))
		FileSys.createDirectory(destinationBase);

	std::string destination = FileSys.pathByAppendingComponent(destinationBase,
		std::to_string(hash));
    
    // The new destination should always not exist, since we checked before if we have the
    // value in the map and only get here if it isn't; persistent cache entries are always
    // in the map and non-persistent entries should have been deleted on application close
	if (!FileSys.directoryExists(destination))
		FileSys.createDirectory(destination);

	auto it = _files.find(hash);
	if (it != _files.end()) {
		// If we find the hash, it has been created before and we can just return the
		// file name to the caller
		cachedFileName = it->second.file;
		return true;
	}

    // Generate and output the newly generated cache name
	cachedFileName = FileSys.pathByAppendingComponent(destination, baseName);

    // Store the cache information in the map
	CacheInformation info = {
		cachedFileName,
		isPersistent
	};
	_files.emplace(hash, info);
	return true;
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
    size_t pos = baseName.find_first_of("/\\?%*:|\"<>.");
    if (pos != std::string::npos) {
        LERROR("Base name '" << baseName << "' consists of illegal character");
        return false;
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
    size_t pos = baseName.find_first_of("/\\?%*:|\"<>.");
    if (pos != std::string::npos) {
        LERROR("Base name '" << baseName << "' consists of illegal character");
        return;
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

unsigned int CacheManager::generateHash(std::string file, std::string information) const
{
	std::string hashString = file + _hashDelimiter + information;
	unsigned int hash = hashCRC32(hashString);

	return hash;
}

void CacheManager::cleanDirectory(const Directory& dir) const {
    LDEBUG("Cleaning directory '" << dir << "'");
    // First search for all subdirectories and call this function recursively on them
	std::vector<std::string> contents = dir.readDirectories();
	for (auto content : contents) {
        if (FileSys.directoryExists(content)) {
			cleanDirectory(content);
        }
	}
    // We get to this point in the recursion if either all subdirectories have been
    // deleted or there exists a file somewhere in the directory tree

	contents = dir.read();
    bool isEmpty = contents.size() == 0;
#ifdef __APPLE__
    // Apple stores the .DS_Store directory in the directory which can be removed
    std::string dsStore = FileSys.pathByAppendingComponent(dir, ".DS_Store");
    isEmpty |= ((contents.size() == 1) && contents[0] == dsStore);
#endif
    // If this directory is empty, we can delete it
    if (isEmpty) {
        LDEBUG("Deleting directory '" << dir << "'");
		FileSys.deleteDirectory(dir);
    }
}
    
std::vector<CacheManager::LoadedCacheInfo> CacheManager::cacheInformationFromDirectory(
    const Directory& dir) const
{
    std::vector<LoadedCacheInfo> result;
    std::vector<std::string> directories = dir.readDirectories(false);
    for (auto directory : directories) {
        Directory d(directory);

        // Extract the name of the directory
        // +1 as the last path delimiter is missing from the path
        std::string directoryName = directory.substr(dir.path().size() + 1);
        
        std::vector<std::string> hashes = d.readDirectories();
        for (auto hash : hashes) {
            // Extract the hash from the directory name
            // +1 as the last path delimiter is missing from the path
            std::string hashName = hash.substr(d.path().size() + 1);
            
            std::vector<std::string> files = Directory(hash).readFiles();
            // Cache directories should only contain a single file with the
            // same name as the directory
            if (files.size() > 1)
                LERROR("Directory '" << hash << "' contained more than one file");
            if (files.size() == 1) {
                // Extract the file name from the full path
                // +1 as the last path delimiter is missing from the path
                std::string filename = files[0].substr(Directory(hash).path().size() + 1);
				if (filename != directoryName)
					LERROR("File contained in cache directory '" <<
					hash << "' contains a file with name '" << filename <<
					"instead of the expected '" << directoryName << "'");
				else
					// Adding the absPath to normalize all / and \ for Windows 
					result.emplace_back(std::stoul(hashName), absPath(files[0]));
            }
        }
    }
    
    return std::move(result);
}

} // namespace filesystem
} // namespace ghoul
