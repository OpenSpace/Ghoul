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

#ifndef __CACHEMANAGER_H__
#define __CACHEMANAGER_H__

#include <ghoul/filesystem/directory.h>

#include <map>
#include <string>

namespace ghoul {
namespace filesystem {

class CacheManager {
public:
	CacheManager(std::string directory);
	~CacheManager();

	bool getCachedFile(std::string file, std::string information,
		std::string& cachedFileName, bool isPersistent = false);
	bool hasCachedFile(const std::string& file, const std::string& information) const;

protected:
	struct CacheInformation {
		std::string file;
		std::string information;
		bool isPersistent;
	};

	unsigned int generateHash(std::string file, std::string information) const;
	void cleanDirectory(const Directory& dir) const;

	CacheManager(const CacheManager& c) = delete;
	CacheManager(CacheManager&& m) = delete;
	CacheManager& operator=(const CacheManager& rhs) = delete;

	Directory _directory;
	std::map<unsigned int, CacheInformation> _files;
};

} // namespace filesystem
} // namespace ghoul

#endif // __CACHEMANAGER_H__
