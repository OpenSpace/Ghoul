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

#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include <map>
#include <string>
#include <vector>
#include <thread>

#include <ghoul/filesystem/directory.h>
#include <ghoul/filesystem/file.h>

namespace ghoul {
namespace filesystem {

/**
 * The methods in this class are used to access platform-independent features of the
 * underlying filesystem. It is possible to convert relative paths into absolute paths
 * (#absolutePath()) and vice versa (#relativePath()). The current working directory
 * can be accessed and changed (#currentDirectory(), #setCurrentDirectory(), 
 * #setCurrentDirectory()). All methods that require a Directory or File parameter can use
 * an <code>std::string</code> or <code>char*</code> directly as this will be converted
 * into a lightweight Directory or File object. The main functionality of the FileSystem 
 * is to deal with path tokens. These are tokens of the form <code>${...}</code> which are
 * like variables, pointing to a specific location. These tokens are resolved in various
 * methods (for example #absolutePath()) and thus be used to dynamically set, for example,
 * asset paths with only one point of specification. These tokens can only be bound once,
 * as some of the tokens might already have been resolved and changing the tokens later
 * might lead to inconsistencies. For the same reason, it is not possible to unregister
 * tokens.
 */
class FileSystem {
public:
    static const char PathSeparator;
    static const std::string TokenOpeningBraces;
    static const std::string TokenClosingBraces;

    /**
     * Initializes the singleton object. This method triggers an assert if the FileSystem
     * already has been initialized before without being deinitialized in between
     * (#initialize()).
     */
    static void initialize();
    
    /**
     * Deinitializes the FileSystem and removes all registered path tokens. As some tokens
     * might already have been resolved in some paths, deleting the tokens might lead to
     * inconsistencies. This function will trigger an assert if the FileSystem has not
     * been initialized (#initialize()) before.
     */
    static void deinitialize();
    
    /**
     * Returns a reference to the static FileSystem. This method will trigger an assert if
     * the FileSystem has not been initialized before.
     * \return A reference to the static FileSystem.
     */
    static FileSystem& ref();
    
    /**
     * Returns the absolute path to the passed <code>path</code>, resolving any tokens (if
     * present) in the process. The current working directory (#currentDirectory()) is
     * used as a base path for this.
     * \param path The path that should be converted into an absolute path
     * \return The absolute path to the passed <code>path</code>
     */
    std::string absolutePath(std::string path) const;
    
    /**
     * Returns the relative path of the passed <code>path</code> relative to the passed
     * <code>baseDirectory</code>. All path tokens present in the <code>path</code> are
     * automatically resolved. The <code>baseDirectory</code> can be resolved if the
     * Directory object has been created that way. If, on Windows, the <code>path</code>
     * and the <code>baseDirectory</code> are on different drives, no relative path can be
     * constructed and the unchanged path is returned.
     * \param path The path that should be converted into a relative path
     * \param baseDirectory The base directory the path is relative to
     * \return The relative path leading from the <code>baseDirectory</code> to the
     * <code>path</code>
     */
    std::string relativePath(std::string path,
                             const Directory& baseDirectory = Directory()) const;

    /**
     * Returns the current working directory of this process.
     * \return The current working directory of this process
     */
    Directory currentDirectory() const;
    
    /**
     * Changes the current working directory of this process. Please note that this will
     * have an effect on all relative paths which are used henceforth.
     * \param directory The path that will be used as the new working directory
     */
    void setCurrentDirectory(const Directory& directory) const;

    /**
     * Checks if the file at the <code>path</code> exists or not. This method will also
     * return <code>false</code> if <code>path</code> points to a directory. This method
     * will not expand any tokens that are passed to it.
     * \param path The path that should be tested for existence
     * \return <code>true</code> if <code>path</code> points to an existing file,
     * <code>false</code> otherwise
     */
    bool fileExists(const File& path) const;
    
    /**
     * Checks if the directory at the <code>path</code> exists or not. This method will
     * return <code>false</code> if <code>path</code> points to a file.
     * \param path The path that should be tested for existence
     * \return <code>true</code> if <code>path</code> points to an existing directory,
     * <code>false</code> otherwise
     */
    bool directoryExists(const Directory& path) const;
    
    /**
     * Deletes the file pointed to by <code>path</code>. The method will return <code>true
     * </code> if the file was deleted successfully, <code>false</code> otherwise.
     * \param path The file that should be deleted
     * \return <code>true</code> if the file was deleted successfully, <code>false</code>
     * otherwise
     */
    bool deleteFile(const File& path) const;

    /**
     * Deletes the directory pointed to by <code>path</code>. The method will return
     * <code>true</code> if the file was deleted successfully, <code>false</code>
     * otherwise.
     * \param path The file that should be deleted
     * \return <code>true</code> if the file was deleted successfully, <code>false</code>
     * otherwise
     */
    bool deleteDirectory(const Directory& path) const;
    
    /**
     * Registers the path token <code>token</code> with this FileSystem. Henceforth, every
     * call to, for example, #absolutePath(), the constructors of File, or Directory,
     * will replace the <code>token</code> with <code>path</code>. The tokens cannot be
     * removed or replaced afterwards, as this might lead to inconsistencies since some
     * files might have replaced the tokens while others have not.
     * \param token The token in the form <code>${...}</code>
     * \param path The path the token should point to
     */
    void registerPathToken(std::string token, std::string path);

    /**
     * Replaces the path tokens present in the <code>path</code> if any exist. If all 
     * tokens could be replaced, the method returns <code>true</code>; if
     * <code>false</code> is returned, one or more tokens could not be replaced. In this
     * case, only part of the path is modified.
     * \param path The path whose tokens should be replaced
     * \return <code>true</code> if all tokens were replaced successfully,
     * <code>false</code> otherwise
     */
    bool expandPathTokens(std::string& path) const;

#if !defined(WIN32) && !defined(__APPLE__)
    int inotifyHandle();
    void inotifyAddListener(File* fileobject);
    void inotifyRemoveListener(File* fileobject);
#endif

private:
    /**
     * This method cleans up a passed path by removing any double path separators and
     * replacing all separators into the ones used by the operating system. It also
     * removes any trailing separators at the end of the path.
     * \param path The path that should be cleaned up
     * \return The cleaned path with correct separators
     */
    std::string cleanupPath(std::string path) const;
    
    /**
     * This method returns the position until both paths <code>p1</code> and
     * <code>p2</code> are equal. After the returned position, the paths are diverging.
     * \param p1 The one path that is used for the comparison
     * \param p2 The other path that is used for the comparison
     * \return The position until which the paths <code>p1</code> and <code>p2</code> are
     * equal
     */
    size_t commonBasePathPosition(const std::string& p1, const std::string& p2) const;

    /**
     * Returns <code>true</code> if the <code>path</code> contains any tokens.
     * \param path The path that is checked for tokens
     * \return <code>true</code> if the <code>path</code> contains any tokens
     */
    bool hasTokens(const std::string& path) const;
    
    /**
     * Returns true, if the <code>path</code> contains the token <code>token</code>.
     * \param path The path that is checked for the existence of the <code>token</code>
     * \param token The token that is checked for existence in the <code>path</code>
     * \return <code>true</code> if the <code>token</code> exists in the <code>path</code>
     */
    bool hasToken(const std::string& path, const std::string& token) const;
    
    /**
     * Returns the path that was registered for the <code>token</code>. If the token has
     * not been registered with any replacement path, the token itself is returned.
     * \param token The token whose replacement is looked up
     * \return The replacement string for the token
     */
    std::string resolveToken(const std::string& token) const;

    /**
     * Empty constructor. Private as there should not be any local FileSystems around,
     * but the static one.
     */
    FileSystem();

    FileSystem(const FileSystem& rhs) = delete;
    FileSystem& operator=(const FileSystem& rhs) = delete;
    
    /// This map stores all the tokens that are used in the FileSystem.
    std::map<std::string, std::string> _tokenMap;

#if !defined(WIN32) && !defined(__APPLE__)
    int _inotifyHandle;
    bool _keepGoing;
    std::thread _t;
    std::map<int, File*> _inotifyFiles;
    static void inotifyWatcher();
#endif

    /// This member variable stores the static FileSystem. Has to be initialized and
    /// deinitialized using the #initialize and #deinitialize methods.
    static FileSystem* _fileSystem;
};

#define FileSys (ghoul::filesystem::FileSystem::ref())
#define absPath(__path__) ghoul::filesystem::FileSystem::ref().absolutePath(__path__)

} // namespace filesystem
} // namespace ghoul

#endif // __FILESYSTEM_H__
