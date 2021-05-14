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

#ifndef __GHOUL___FILESYSTEM___H__
#define __GHOUL___FILESYSTEM___H__

#include <ghoul/filesystem/file.h>
#include <ghoul/misc/boolean.h>
#include <ghoul/misc/exception.h>
#include <filesystem>
#include <map>

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
#endif // WIN32

class CacheManager;
class File;

/**
 * The methods in this class are used to convert relative paths into absolute paths
 * (#absolutePath), however the main functionality of the FileSystem is to deal with path
 * tokens. These are tokens of the form <code>${...}</code> which are like variables,
 * pointing to a specific location. These tokens can only be bound once, as some of the
 * tokens might already have been resolved and changing the tokens later might lead to
 * inconsistencies. For the same reason, it is not possible to unregister tokens. Every
 * FileSystem contains one token <code>${TEMPORARY}</code> that points to the location of
 * the system's temporary files.
 */
class FileSystem {
public:
    BooleanType(Override);

    /// Exception that gets thrown if the FileSystem encounters a nonrecoverable error
    struct FileSystemException : RuntimeError {
        explicit FileSystemException(std::string msg);
    };

    /// Exception that gets thrown if a file system token could not be resolved
    struct ResolveTokenException : FileSystemException {
        explicit ResolveTokenException(std::string t);

        const std::string token;
    };

    FileSystem(const FileSystem& rhs) = delete;
    FileSystem(const FileSystem&&) = delete;
    FileSystem& operator=(const FileSystem& rhs) = delete;
    FileSystem& operator=(FileSystem&& rhs) = delete;

    static void initialize();
    static void deinitialize();
    static bool isInitialized();
    static FileSystem& ref();

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
    void registerPathToken(std::string token, std::filesystem::path path,
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
     * \return The path with the tokens replaced
     */
    [[nodiscard]] std::filesystem::path expandPathTokens(std::string path,
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
    void createCacheManager(const std::filesystem::path& directory, int version = -1);

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
    int addFileListener(std::filesystem::path path, File::FileChangedCallback callback);

    /**
     * Removes the file object from tracking lists. The file on the filesystem may still
     * be tracked and other File objects may still have callbacks registered.
     *
     * \pre \p file must not be a <code>nullptr</code>
     * \pre \p file must have been added before (addFileListener)
     */
    void removeFileListener(int callbackIdentifier);

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
     * Returns true, if the \p path contains the \p token.
     *
     * \param path The path that is checked for the existence of the \p token
     * \param token The token that is checked for existence in the \p path
     * \return <code>true</code> if the \p token exists in the \p path
     * \pre \p path must not be empty
     * \pre \p token must not be empty
     */
    bool hasToken(const std::string& path, const std::string& token) const;

    /// This map stores all the tokens that are used in the FileSystem.
    std::map<std::string, std::filesystem::path> _tokenMap;

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

    struct FileChangeInfo {
        static int NextIdentifier;
        int identifier;
        std::filesystem::path path;
        File::FileChangedCallback callback;
    };
    /// The list of all tracked files
    std::vector<FileChangeInfo> _trackedFiles;

    /// The list of tracked directories
    std::map<std::string, DirectoryHandle*> _directories;

#elif defined(__APPLE__)
    /// OS X specific deinitialize function
    void deinitializeInternalApple();

    /// OS X callback handler
    static void callbackHandler(const std::string& path);

    /// Friend callback handler calling the static callback handler
    friend void callbackHandler(const std::string& path);

    struct FileChangeInfo {
        static int NextIdentifier;
        int identifier;
        std::filesystem::path path;
        File::FileChangedCallback callback;
    };
    /// The list of all tracked files
    std::vector<FileChangeInfo> _trackedFiles;

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

    struct FileChangeInfo {
        static int NextIdentifier;
        int identifier;
        int inotifyHandle;
        std::filesystem::path path;
        File::FileChangedCallback callback;
    };
    /// The list of all tracked files
    std::vector<FileChangeInfo> _trackedFiles;
#endif

    static FileSystem* _instance;
};

#define FileSys (ghoul::filesystem::FileSystem::ref())

} // namespace ghoul::filesystem

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
std::string absPath(std::string path);

#endif // __GHOUL___FILESYSTEM___H__
