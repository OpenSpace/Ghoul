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
#include <ghoul/filesystem/file.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/crc32.h>

#include <algorithm>
#include <assert.h>
#include <fstream>

namespace {
	const std::string _loggerCat = "CacheManager";
	const std::string _cacheFile = "cache";
	const char _hashDelimiter = '|'; // something that cannot occur in the filesystem
}

namespace ghoul {
namespace filesystem {

CacheManager::CacheManager(std::string directory)
	: _directory(directory)
{
	std::string&& path = FileSys.pathByAppendingComponent(_directory, _cacheFile);
	std::ifstream file(path);
	if (file.good()) {
		std::string line;
		while (std::getline(file, line)) {
			std::stringstream s(line);

			unsigned int hash;
			s >> hash;

			CacheInformation info;
			std::getline(file, info.file);
			std::getline(file, info.information);
			info.isPersistent = true;
			_files.emplace(hash, info);
		}
	}
}

CacheManager::~CacheManager() {
	std::string&& path = FileSys.pathByAppendingComponent(_directory, _cacheFile);
	std::ofstream file(path, std::ofstream::out);
	if (file.good()) {
		// Delete all the non-persistent files
		// Save the persistent files in a cache file
		for (auto p : _files) {
			if (!p.second.isPersistent) {
				if (FileSys.fileExists(p.second.file))
					FileSys.deleteFile(p.second.file);
			}
			else
				file << p.first << std::endl <<
						p.second.file << std::endl <<
						p.second.information << std::endl; 
		}
	}
	else
		LERROR("Could not open file '" << path << "' for writing permanent cache files");

	cleanDirectory(_directory);
}

bool CacheManager::getCachedFile(std::string file, std::string information,
	std::string& cachedFileName, bool isPersistent)
{
	unsigned int hash = generateHash(file, information);

	auto it = _files.find(hash);
	if (it != _files.end()) {
		cachedFileName = it->second.file;
		return true;
	}
	
	File isPer(file);
	std::string filename = isPer.filename();
	std::string destinationBase = FileSys.pathByAppendingComponent(_directory, filename);

	if (!FileSys.directoryExists(destinationBase))
		FileSys.createDirectory(destinationBase);

	std::string destination = FileSys.pathByAppendingComponent(destinationBase,
		std::to_string(hash));
	if (!FileSys.directoryExists(destination))
		FileSys.createDirectory(destination);

	cachedFileName = FileSys.pathByAppendingComponent(destination, filename);

	CacheInformation info = {
		cachedFileName,
		information,
		isPersistent
	};
	_files.emplace(hash, info);
	return true;
}

bool CacheManager::hasCachedFile(const std::string& file,
	const std::string& information) const
{
	unsigned int hash = generateHash(file, information);
	return (_files.find(hash) != _files.end());
}

unsigned int CacheManager::generateHash(std::string file, std::string information) const
{
	std::string hashString = file + _hashDelimiter + information;
	unsigned int hash = hashCRC32(hashString);

	return hash;
}

void CacheManager::cleanDirectory(const Directory& dir) const {
	std::vector<std::string> contents = dir.read();
	for (auto content : contents) {
		if (FileSys.directoryExists(content))
			cleanDirectory(content);
	}

	contents = dir.read();
	if (contents.size() == 0)
		FileSys.deleteDirectory(dir);
}

} // namespace filesystem
} // namespace ghoul
