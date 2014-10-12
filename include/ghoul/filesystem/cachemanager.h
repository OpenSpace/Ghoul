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

#include <ghoul/filesystem/file.h>

#include <map>
#include <string>

namespace ghoul {
namespace filesystem {

/**
 * The CacheManager allows users to request a storage location for an, optionally
 * persistent, file path to store a cached result. This class only generates and manages
 * the file paths and does not do any caching itself. The use case for this is are
 * two-fold:<br>
 * First, expensive operation that has a result, which gets written to a file and the
 * developer wants to retain the results without recomputing it at every application
 * start. For this, a cache file is requested with the getCachedFile method and the
 * <code>isPersistent</code> flag set to <code>true</code>. Using the same
 * <code>file</code> and <code>information</code> values, the same path will be retrieved
 * in subsequent application runs.<br>
 * The second use-case is a temporary file, also with the getCachedFile method, but the
 * <code>isPersistent</code> flag set to <code>false</code>. Non-persistent files will
 * automatically be deleted when the program ends.<br>
 * The persistent files are stored in a <code>cache</code> file so that they can be
 * retained between application runs.
 */
class CacheManager {
public:
    /**
     * The constructor will automatically register all persistent cache entries from
     * previous application runs and clean the directory of non-persistent entries that
     * might have been left intact if the previous run crashed. After the constructor
     * returns, the CacheManager will leave a cleaned cache directory and the persistent
     * files are correctly registered and available.
     */
	CacheManager(std::string directory);
    
    /**
     * The destructor will save all information on persistent files in a
     * <code>cache</code> file in the cache directory that was passed in the constructor
     * so that they can be retrieved when the application is started up again. All
     * non-persistent files are automatically deleted in the destructor.
     */
	~CacheManager();

    /**
     * Returns the path to a storage location for the cached file. Depending on the
     * persistence (<code>isPersistent</code>), the directory and files will automatically
     * be cleaned on application end or be made available automatically on the next
     * application run. The method will use the date of last modification as a unique
	 * identifier for the file. Subsequent calls (in the same run or different) with the
	 * same <code>file</code> will consistently produce the same file path until the
	 * last-modified date changes. If the cached file was created before, the
	 * <code>isPersistent</code> parameter is silently ignored.
     * \param file The file name of the file for which the cached entry is to be retrieved
     * \param cachedFileName The output file name pointing to the cached file that can be
     * used by the caller to store the results
     * \param isPersistent This parameter will only be used if the cached file is used for
     * the first time and determines if the CacheManager should automatically delete the
     * file when the application closes (<code>false</code>) or if the file should be kept
     * and automatically be re-added to the CacheManager on the next application run
     * (<code>true</code>). If the cached file has been created before, this parameter is
     * silently ignored.
     * \return <code>true</code> if a file name was returned successfully,
     * <code>false</code> otherwise
     */
	bool getCachedFile(const File& file, std::string& cachedFileName,
		bool isPersistent = false);

    /**
     * Returns the path to a storage location for the cached file. Depending on the
     * persistence (<code>isPersistent</code>), the directory and files will automatically
     * be cleaned on application end or be made available automatically on the next
     * application run. Subsequent calls (in the same run or different) with the same
     * <code>file</code> and <code>information</code> will consistently produce the same
     * file path. The combination of <code>file</code> and <code>information</code> is the
     * unique key for the returned cached file. If the cached file was created before, the
     * <code>isPersistent</code> parameter is silently ignored.
     * \param file The file name of the file for which the cached entry is to be retrived
     * \param information Additional information that is used to uniquely identify the
     * cached file. This can be, for example, resolutions or parameters. The combination
     * of the <code>file</code> + <code>information</code> uniquely identifies a cached
     * file
     * \param cachedFileName The output file name pointing to the cached file that can be
     * used by the caller to store the results
     * \param isPersistent This parameter will only be used if the cached file is used for
     * the first time and determines if the CacheManager should automatically delete the
     * file when the application closes (<code>false</code>) or if the file should be kept
     * and automatically be re-added to the CacheManager on the next application run
     * (<code>true</code>). If the cached file has been created before, this parameter is
     * silently ignored.
     * \return <code>true</code> if a file name was returned successfully,
     * <code>false</code> otherwise
     */
    bool getCachedFile(const File& file, const std::string& information,
                       std::string& cachedFileName, bool isPersistent = false);

    /**
     * Returns the path to a storage location for the cached file. Depending on the
     * persistence (<code>isPersistent</code>), the directory and files will automatically
     * be cleaned on application end or be made available automatically on the next
     * application run. Subsequent calls (in the same run or different) with the same
     * <code>baseName</code> and <code>information</code> will consistently produce the
     * same file path. The combination of <code>baseName</code> and
     * <code>information</code> is the unique key for the returned cached file. If the
     * cached file was created before, the <code>isPersistent</code> parameter is silently
     * ignored.<br>
     * As the <code>baseName</code> will be used as a name for a directory in the file
     * system, the usual restrictions apply. The baseName is automatically converted into
     * lower case, so that the <code>baseName</code> of <code>base</code>,
     * <code>bAsE</code>, and <code>BASE</code> all refer to the same file. Furthermore,
     * the <code>baseName</code> cannot contain any of the following characters:
     * <code>/</code>, <code>\\</code>, <code>?</code>, <code>%</code>, <code>*</code>,
     * <code>:</code>, <code>|</code>, <code>"</code>, <code>\<</code>, <code>\></code>,
     * or <code>.</code>
     * \param baseName The base name for which the cached entry is to be retrived. A list
     * of restrictions apply to this base name (see above)
     * \param information Additional information that is used to uniquely identify the
     * cached file. This can be, for example, resolutions or parameters. The combination
     * of the <code>baseName</code> + <code>information</code> uniquely identifies a
     * cached file
     * \param cachedFileName The output file name pointing to the cached file that can be
     * used by the caller to store the results
     * \param isPersistent This parameter will only be used if the cached file is used for
     * the first time and determines if the CacheManager should automatically delete the
     * file when the application closes (<code>false</code>) or if the file should be kept
     * and automatically be re-added to the CacheManager on the next application run
     * (<code>true</code>). If the cached file has been created before, this parameter is
     * silently ignored.
     * \return <code>true</code> if a file name was returned successfully,
     * <code>false</code> otherwise
     */
    bool getCachedFile(const std::string& baseName, const std::string& information,
		std::string& cachedFileName, bool isPersistent = false);

	/**
     * This method checks if a cached <code>file</code> has been registered before in this
     * application run (persistent and non-persistent files) or in a previous run
     * (persistent cache files only). Note that this only checks if a file has been
	 * requested before, not if the cached file has actually been used. The method will
	 * use the date of last modification as a unique identifier for the file.
     * \param file The file for which the cached file should be searched
     * \return <code>true</code> if a cached file was requested before; <code>false</code>
     * otherwise
     */
	bool hasCachedFile(const File& file) const;

    /**
     * This method checks if a cached <code>file</code> has been registered before in this
     * application run (persistent and non-persistent files) or in a previous run
     * (persistent cache files only) with the provided <code>information</code>. Note that
     * this only checks if a file has been requested before, not if the cached file has
     * actually been used.
     * \param file The file for which the cached file should be searched
     * \param information The identifying information for the file
     * \return <code>true</code> if a cached file was requested before; <code>false</code>
     * otherwise
     */
    bool hasCachedFile(const File& file, const std::string& information) const;

    /**
     * This method checks if a cached file has been registered before in this
     * application run (persistent and non-persistent files) or in a previous run
     * (persistent cache files only) with the provided <code>baseName</code> and 
     * <code>information</code>. Note that this only checks if a file has been requested
     * before, not if the cached file has actually been used.
     * \param baseName The base name for which the cache file should be searched
     * \param information The identifying information for the file
     * \return <code>true</code> if a cached file was requested before; <code>false</code>
     * otherwise
     */
    bool hasCachedFile(const std::string& baseName, const std::string& information) const;

	/**
     * Removes the cached file and deleted the entry from the CacheManager. If the
     * <code>file</code> has not previously been used to request a cache entry, no error
     * will be signaled. The method will use the date of last modification as a unique
	 * identifier for the file.
     * \param file The file for which the cache file should be deleted
     */
	void removeCacheFile(const File& file);

    /**
     * Removes the cached file and deleted the entry from the CacheManager. If the
     * <code>file</code> has not previously been used to request a cache entry, no error
     * will be signaled.
     * \param file The file for which the cache file should be deleted
     * \param information The detailed information for the cached file which should be
     * deleted
     */
    void removeCacheFile(const File& file, const std::string& information);

    /**
     * Removes the cached file and deleted the entry from the CacheManager. If the
     * <code>baseName</code> has not previously been used to request a cache entry, no
     * error will be signaled. The same restrictions for the <code>baseName</code> as in
     * getCachedFile apply to this function.
     * \param baseName The base name for which the cache file should be deleted
     * \param information The detailed information identifying the cached file that
     * should be deleted
     */
    void removeCacheFile(const std::string& baseName,
                         const std::string& information);
    
protected:
    /// This struct stores the cache information for a specific hash value.
	struct CacheInformation {
        std::string file; ///< The path to the cached file
        bool isPersistent; ///< if the cached entry should be automatically deleted
	};
    
    typedef std::pair<unsigned int, std::string> LoadedCacheInfo;

	unsigned int generateHash(std::string file, std::string information) const;
	void cleanDirectory(const Directory& dir) const;
    std::vector<LoadedCacheInfo> cacheInformationFromDirectory(
                                                            const Directory& dir) const;

	CacheManager(const CacheManager& c) = delete;
	CacheManager(CacheManager&& m) = delete;
	CacheManager& operator=(const CacheManager& rhs) = delete;

	Directory _directory;
	std::map<unsigned int, CacheInformation> _files;
};

} // namespace filesystem
} // namespace ghoul

#endif // __CACHEMANAGER_H__
