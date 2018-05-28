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

#ifndef __GHOUL___FILESYSTEM___H__
#define __GHOUL___FILESYSTEM___H__

#include <ghoul/designpattern/singleton.h>
#include <ghoul/filesystem/directory.h>
#include <ghoul/misc/boolean.h>
#include <ghoul/misc/exception.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#if !defined(WIN32) && !defined(__APPLE__)
#include <thread>
#endif

namespace ghoul::filesystem {

#ifdef WIN32
struct DirectoryHandle;
void readStarter(DirectoryHandle* directoryHandle);
void callbackHandler(DirectoryHandle* directoryHandle, const std::string& filePath);
#elif defined(__APPLE__)
struct DirectoryHandle;
void callbackHandler(const std::string& path);
#endif

class CacheManager;
class File;

/**
 * The methods in this class are used to access platform-independent features of the
 * underlying filesystem. It is possible to convert relative paths into absolute paths
 * (#absolutePath) and vice versa (#relativePath). The current working directory can be
 * accessed and changed (#currentDirectory), #setCurrentDirectory, #setCurrentDirectory).
 * All methods that require a Directory or File parameter can use an
 * <code>std::string</code> or <code>char*</code> directly as this will be converted into
 * a lightweight Directory or File object. The main functionality of the FileSystem is to
 * deal with path tokens. These are tokens of the form <code>${...}</code> which are like
 * variables, pointing to a specific location. These tokens are resolved in various
 * methods (for example #absolutePath) and thus be used to dynamically set, for example,
 * asset paths with only one point of specification. These tokens can only be bound once,
 * as some of the tokens might already have been resolved and changing the tokens later
 * might lead to inconsistencies. For the same reason, it is not possible to unregister
 * tokens. Every FileSystem contains one token <code>${TEMPORARY}</code> that points to
 * the location of the system's temporary files.
 */
class FileSystem : public Singleton<FileSystem> {
public:
    BooleanType(RawPath);
    BooleanType(Recursive);
    BooleanType(Override);

    friend class Singleton<FileSystem>;

    /// Exception that gets thrown if the FileSystem encounters a nonrecoverable error
    struct FileSystemException : RuntimeError {
        explicit FileSystemException(const std::string& msg);
    };

    /// Exception that gets thrown if a file system token could not be resolved
    struct ResolveTokenException : FileSystemException {
        explicit ResolveTokenException(std::string t);

        std::string token;
    };

    /**
     * The token used to separate individual path elements (<code>\\</code> or
     * <code>/</code>)
     */
#ifdef WIN32
    static constexpr const char PathSeparator = '\\';
#else
    static constexpr const char PathSeparator = '/';
#endif // WIN32

    /// Opening braces that are used for path tokens
    static constexpr const char* TokenOpeningBraces = "${";

    /// Closing braces that are used for path tokens
    static constexpr const char* TokenClosingBraces = "}";

    FileSystem(const FileSystem& rhs) = delete;
    FileSystem& operator=(const FileSystem& rhs) = delete;

    /**
     * Returns the absolute path to the passed \p path, resolving any tokens (if present)
     * in the process. The current working directory (#currentDirectory) is used as a base
     * path for this. All tokens contained in the \p ignoredTokens are ignored from the
     * token resolving
     *
     * \param path The path that should be converted into an absolute path
     * \param ignoredTokens All tokens contained in this list are ignored during the
     *        resolving of path tokens
     * \return The absolute path to the passed \p path
     *
     * \pre \p path must not be empty
     */
    std::string absolutePath(std::string path,
        const std::vector<std::string>& ignoredTokens = std::vector<std::string>()) const;

    /**
     * Returns the path of the passed \p path relative to the passed \p baseDirectory. All
     * path tokens present in the \p path are automatically resolved. The \p baseDirectory
     * can be resolved if the Directory object has been created that way. If, on Windows,
     * the \p path and the \p baseDirectory are on different drives, no relative path can
     * be constructed and the unchanged path is returned.
     *
     * \param path The path that should be converted into a relative path
     * \param baseDirectory The base directory the path is relative to
     * \return The relative path leading from the \p baseDirectory to the \p path
     *
     * \pre \p path must not be empty
     */
    std::string relativePath(std::string path,
        const Directory& baseDirectory = Directory()) const;

    /**
     * Appends the \p component to the \p path by using the appropriate path separator and
     * returns the complete path. This method does not check if the full path refers to a
     * valid file/directory in the system.
     *
     * \param path The base path to which the \p component will be added
     * \param component The path component that should be added to the \p path
     * \return The full path composed of the \p path and the \p component
     */
    std::string pathByAppendingComponent(std::string path, std::string component) const;

    /**
     * Converts the \p path's path separators from the native path separator to the
     * provided \p separator.
     *
     * \param path The path that should be converted to use the new \p separator
     * \param separator The new path separator for the returned path
     * \return The \p path with all native path separators replaced to the \p separator
     */
    std::string convertPathSeparator(std::string path, char separator) const;

    /**
     * Returns the current working directory of this process.
     *
     * \return The current working directory of this process
     *
     * \throw FileSystemException If the current directory could not be determined
     */
    Directory currentDirectory() const;

    /**
     * Changes the current working directory of this process. Please note that this will
     * have an effect on all relative paths which are used henceforth.
     *
     * \param directory The path that will be used as the new working directory
     *
     * \throw FileSystemException If the current directory could not be set
     */
    void setCurrentDirectory(const Directory& directory) const;

    /**
     * Checks if the file at the \p path exists or not. This method will also return
     * <code>false</code> if \p path points to a directory. This method will not expand
     * any tokens that are passed to it.
     *
     * \param path The path that should be tested for existence
     * \return <code>true</code> if \p path points to an existing file, <code>false</code>
     *         otherwise
     *
     * \throw FileSystemException If there is an error retrieving the file attribtues
     *        for the \p path
     */
    bool fileExists(const File& path) const;

    /**
     * Checks if the directory at the \p path exists or not. This method will return
     * <code>false</code> if \p path points to a file.
     *
     * \param path The path that should be tested for existence
     * \return <code>true</code> if \p path points to an existing directory,
     *         <code>false</code> otherwise
     *
     * \throw FileSystemException If there is an error retrieving the directory attributes
     *        for the \p path
     */
    bool directoryExists(const Directory& path) const;

    /**
     * Deletes the file pointed to by \p path. The method will return <code>true</code> if
     * the file was deleted successfully, <code>false</code> otherwise.
     *
     * \param path The file that should be deleted
     * \return <code>true</code> if the file was deleted successfully, <code>false</code>
     *         otherwise
     */
    bool deleteFile(const File& path) const;

    /**
     * Creates the directory pointed to by \p path. The method will return
     * <code>true</code> if the directory was created successfully. If \p recursive is
     * <code>true</code> all directories leading to the \p path will be created a well.
     *
     * \param path The directory that is to be created
     * \param recursive If <code>true</code> all directories leading to the
     *        \p path will be created; if <code>false</code> and the leading directories
     *        do not exist, the method will fail and return <code>false</code>
     *
     * \throw FileSystemException If there was an error creating the directory
     */
    void createDirectory(const Directory& path,
        Recursive recursive = Recursive::No) const;

    /**
     * Deletes the directory pointed to by \p path. The method will return
     * <code>true</code> if the directory was deleted successfully, <code>false</code>
     * otherwise. If \p recursive is <code>true</code> the content will be deleted as
     * well.
     *
     * \param path The directory that should be deleted
     * \param recursive True if content should be removed as well, default is false
     *
     * \throw FileSystemException If there was an error deleting the directory
     * \throw FileSystemException If \p path is not an empty directory and \p recursive
     *        is <code>false</code>
     * \pre \p path must be an existing directory
     */
    void deleteDirectory(const Directory& path,
        Recursive recursive = Recursive::No) const;

    /**
     * Checks if the directory with \p path is empty. The method will return
     * <code>true</code> if the directory is empty, <code>false</code> otherwise.
     *
     * \param path The directory that should be checked
     * \return <code>true</code> if the directory is empty, <code>false</code>
     *         otherwise
     */
    bool emptyDirectory(const Directory& path) const;

    /**
     * Registers the path token \p token with this FileSystem. Henceforth, every call to,
     * for example, #absolutePath(), the constructors of File, or Directory, will replace
     * the \p token with \p path. The tokens cannot be removed or replaced afterwards, as
     * this might lead to inconsistencies since some files might have replaced the tokens
     * while others have not.
     *
     * \param token The token in the form <code>${...}</code>
     * \param path The path the token should point to
     * \param override If <code>true</code> an existing token will be silently overriden
     *
     * \pre \p token must not be empty
     * \pre \p token must start with FileSystem::TokenOpeningBrace and end with
     *      FileSystem::TokenClosingBraces
     * \pre \p token must not have been registered before if \p override is false
     */
    void registerPathToken(std::string token, std::string path,
        Override override = Override::No);

    /**
     * Replaces the path tokens present in the \p path if any exist. If all tokens could
     * be replaced, the method returns <code>true</code>; if <code>false</code> is
     * returned, one or more tokens could not be replaced. In this case, only part of the
     * path is modified.
     *
     * \param path The path whose tokens should be replaced
     * \param ignoredTokens All tokens contained in this list are ignored during the
     *        resolving of path tokens
     * \return <code>true</code> if all tokens were replaced successfully,
     *         <code>false</code> otherwise
     */
    bool expandPathTokens(std::string& path,
        const std::vector<std::string>& ignoredTokens = std::vector<std::string>()) const;

    /**
     * Returns a vector of all registered path tokens.
     *
     * \return A vector of all registered path tokens
     */
    std::vector<std::string> tokens() const;

    /**
     * Checks whether the \p token has been registered to a path before.
     * \param token The token to be checked.
     *
     * \return <code>true</code> if the \p token has been registered to a path before,
     *         <code>false</code> otherwise
     */
    bool hasRegisteredToken(const std::string& token) const;

    /**
     * Returns <code>true</code> if the \p path contains any tokens.
     *
     * \param path The path that is checked for tokens
     * \return <code>true</code> if the \p path contains any tokens
     *
     * \pre \p path must not be empty
     */
    bool containsToken(const std::string& path) const;

    /**
     * Creates a CacheManager for this FileSystem. If a CacheManager already exists, this
     * method will fail and log an error. The passed \p cacheDirectory has to be a valid
     * and existing Directory.
     *
     * \param cacheDirectory The directory in which all cached files will be stored. Has
     *        to be an existing directory with proper read/write access.
     * \param version The version of the this cache. If the passed version is different
     *        from the cache on disk, the cache is completely discarded
     * \return <code>true</code> if the CacheManager was created successfully;
     *         <code>false</code> otherwise. Causes for failure are, among others, a
     *         non-existing directory, missing read/write rights, or if the CacheManager
     *         was created previously without destroying it in between
     *         (destroyCacheManager)
     *
     * \pre \p cacheDirectory must point to an existing directory
     * \pre \p The CacheManager must not have been created before without destroying it
     */
    void createCacheManager(const Directory& cacheDirectory, int version = -1);

    /**
     * Destroys the previously created CacheManager. The destruction of the CacheManager
     * will trigger a cleanup of the cache directory via the CacheManager destructor.
     * After this method returns, a new CacheManager can be reinitialized with a new
     * cache directory
     *
     * \pre CacheManager must have been created before
     */
    void destroyCacheManager();

    /**
     * Returns the CacheManager associated with this FileSystem
     *
     * \return The CacheManager or <code>nullptr</code> if it has not been initialized
     *
     * \pre CacheManager must have been created before
     */
    CacheManager* cacheManager();

    /**
     * Listen to \p file for changes. When \p file is changed the File callback will be
     * called.
     *
     * \param file The file object to be tracked
     *
     * \pre \p file must not be a <code>nullptr</code>
     * \pre \p file must not have been added before
     */
    void addFileListener(File* file);

    /**
     * Removes the file object from tracking lists. The file on the filesystem may still
     * be tracked and other File objects may still have callbacks registered.
     *
     * \pre \p file must not be a <code>nullptr</code>
     * \pre \p file must have been added before (addFileListener)
     */
    void removeFileListener(File* file);

    /**
     * Triggers callbacks on filesystem. May not be needed depending on environment.
     */
    void triggerFilesystemEvents();

private:
    /**
     * Constructs a FileSystem object.
     *
     * \throw FileSystemException if the temporary folder cannot be found
     */
    FileSystem();

    /**
     * Deinitializes the FileSystem and removes all registered path tokens. As some tokens
     * might already have been resolved in some paths, deleting the tokens might lead to
     * inconsistencies.
     */
    ~FileSystem();

    /**
     * This method cleans up a passed \p path by removing any double path separators and
     * replacing all separators into the ones used by the operating system. It also
     * removes any trailing separators at the end of the path.
     *
     * \param path The path that should be cleaned up
     * \return The cleaned path with correct separators
     *
     * \pre \p path must not be empty
     */
    std::string cleanupPath(std::string path) const;

    /**
     * This method returns the position until both paths \p p1 and \p p2 are equal. After
     * the returned position, the paths are diverging.
     *
     * \param p1 The one path that is used for the comparison
     * \param p2 The other path that is used for the comparison
     * \return The position until which the paths \p p1 and \p p2 are equal
     */
    size_t commonBasePathPosition(const std::string& p1, const std::string& p2) const;

    /**
     * Returns true, if the \p path contains the \p token.
     *
     * \param path The path that is checked for the existence of the \p token
     * \param token The token that is checked for existence in the \p path
     * \return <code>true</code> if the \p token exists in the \p path
     * \pre \p path must not be empty
     * \pre \p token must not be empty
     */
    bool hasToken(const std::string& path, const std::string& token) const;

    /**
     * Returns the path that was registered for the \p token. If the token has not been
     * registered with any replacement path, the token itself is returned.
     *
     * \param token The token whose replacement is looked up
     * \return The replacement string for the token
     *
     * \throw FileSystemException If the token could not be resolved
     * \pre \p token must not be empty
     */
    std::string resolveToken(const std::string& token) const;

    /// This map stores all the tokens that are used in the FileSystem.
    std::map<std::string, std::string> _tokenMap;

    /// The cache manager object, only allocated if createCacheManager is called
    std::unique_ptr<CacheManager> _cacheManager;

#ifdef WIN32
    /// Windows specific deinitialize function
    void deinitializeInternalWindows();

    /// Starts watching a directory
    void beginRead(DirectoryHandle* directoryHandle);

    /// Handles the callback for a directory for the local file path
    static void callbackHandler(DirectoryHandle* directoryHandle,
        const std::string& filePath);

    /// External function that calls beginRead
    friend void readStarter(DirectoryHandle* directoryHandle);

    /// External function that calls callbackHandler
    friend void callbackHandler(DirectoryHandle* directoryHandle,
        const std::string& filePath);

    /// The list of all tracked files
    std::multimap<std::string, File*> _trackedFiles;

    /// The list of tracked directories
    std::map<std::string, DirectoryHandle*> _directories;

#elif __APPLE__
    /// OS X specific deinitialize function
    void deinitializeInternalApple();

    /// OS X specific triger filesystem
    void triggerFilesystemEventsInternalApple();

    /// OS X callback handler
    static void callbackHandler(const std::string& path);

    /// Friend callback handler calling the static callback handler
    friend void callbackHandler(const std::string& path);

    /// The list of all tracked files
    std::multimap<std::string, File*> _trackedFiles;

    /// The list of tracked directories
    std::map<std::string, DirectoryHandle*> _directories;

#else // Linux
    /// Linux specific initialize function
    void initializeInternalLinux();

    /// Linux specific deinitialize function
    void deinitializeInternalLinux();

    /// Function that run by the watcher thread
    static void inotifyWatcher();

    int _inotifyHandle;
    bool _keepGoing;
    std::thread _t;

    /// The list of tracked files
    std::multimap<int, File*> _trackedFiles;
#endif
};

#define FileSys (ghoul::filesystem::FileSystem::ref())
#define absPath(__path__) ghoul::filesystem::FileSystem::ref().absolutePath(__path__)

} // namespace ghoul::filesystem

#endif // __GHOUL___FILESYSTEM___H__
