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

#include <map>
#include <string>

namespace ghoul {

	// hash unique string
class CacheManager {
public:
	static void initialize(std::string cacheDirectory);
	static void deinitialize();
	static CacheManager& ref();

	std::string cacheDirectory() const;

	bool cacheFile(std::string file, const std::string& hash,
		bool isPersistent = false);
	bool hasCachedFile(const std::string& hash) const;
	bool getFile(const std::string& hash, std::string& filename) const; 
	//bool invalidateCache(const std::string& identifier);
	void clearCache();

protected:
	struct CacheInformation {
		std::string filename;
		bool isPersistent;
	};

	CacheManager(std::string cacheDirectory);
	~CacheManager();
	CacheManager(const CacheManager& c) = delete;
	CacheManager(CacheManager&& m) = delete;
	CacheManager& operator=(const CacheManager& rhs) = delete;

	static CacheManager* _manager; ///< Singleton member
	std::string _cacheDirectory;
	std::map<std::string, CacheInformation> _cachedFiles;
};

} // namespace ghoul

#endif // __CACHEMANAGER_H__
