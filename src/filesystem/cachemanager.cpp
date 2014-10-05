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

#include <assert.h>
#include <fstream>

namespace {
	const std::string _loggerCat = "CacheManager";
	const std::string _cacheFile = "cache";
}

namespace ghoul {

CacheManager* CacheManager::_manager = nullptr;

void CacheManager::initialize(std::string cacheDirectory) {
	assert(_manager == nullptr);
	_manager = new CacheManager(cacheDirectory);
	assert(_manager != nullptr);
}

void CacheManager::deinitialize() {
	assert(_manager != nullptr);
	delete _manager;
	_manager = nullptr;
}

CacheManager& CacheManager::ref() {
	assert(_manager);
	return *_manager;
}

CacheManager::CacheManager(std::string cacheDirectory)
	: _cacheDirectory(cacheDirectory)
{
	std::string&& path = FileSys.pathByAppendingComponent(_cacheDirectory, _cacheFile);
	std::ifstream file(path);
	std::string line;
	while (std::getline(file, line)) {
		std::stringstream s(line);
		std::string hash;
		CacheInformation info;
		s >> hash;
		s >> info.filename;
		info.isPersistent = true;
		_cachedFiles.emplace(hash, info);
	}
}

CacheManager::~CacheManager() {
	std::string&& path = FileSys.pathByAppendingComponent(_cacheDirectory, _cacheFile);
	std::ofstream file(path, std::ofstream::out);
	// Delete all the non-persistent files
	// Save the persistent files in a cache file
	for (auto p : _cachedFiles) {
		if (!p.second.isPersistent)
			FileSys.deleteFile(p.second.filename);
		else
			file << p.first << "\t" << p.second.filename << std::endl; 
	}
}


bool CacheManager::cacheFile(std::string file, const std::string& hash,
	bool isPersistent)
{
	auto it = _cachedFiles.find(hash);
	if (it != _cachedFiles.end()) {
		const CacheInformation& info = it->second;
		
		if (info.filename == file && info.isPersistent == isPersistent)
			return true;
		else {
			LERROR("A cached file for hash '" << hash << "' was already registered with "
				"a different file and/or persistency: Stored: {" << info.filename <<
				", " << std::boolalpha << info.isPersistent << "}, passed: {" <<
				file << ", " << isPersistent << "}");
			return false;
		}
	}
	else {
		CacheInformation i = {std::move(file), isPersistent};
		_cachedFiles.emplace(hash, i);
		return true;
	}
}

bool CacheManager::hasCachedFile(const std::string& hash) const
{
	return _cachedFiles.find(hash) != _cachedFiles.end();
}

bool CacheManager::getFile(const std::string& hash, std::string& filename) const
{
	auto it = _cachedFiles.find(hash);
	if (it == _cachedFiles.end())
		return false;
	else {
		filename = it->second.filename;
		return true;
	}
}

void CacheManager::clearCache() {
	// Delete all files
	for (auto p : _cachedFiles) {
		FileSys.deleteFile(p.second.filename);
	}
}

std::string CacheManager::cacheDirectory() const {
	return _cacheDirectory;
}

} // namespace ghoul
