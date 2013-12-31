/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012 Alexander Bock                                                     *
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

#ifndef __FILE_H__
#define __FILE_H__

#include <functional>
#include <string>

#ifdef WIN32
#include <vector>
#include <windows.h>
#elif __APPLE__
#include <CoreServices/CoreServices.h>
#include <sys/stat.h>
#endif

namespace ghoul {
namespace filesystem {

/**
 * This class is an abstract handle for a generic file on the file system. The main
 * functionality is to be able to extract parts of the path like the \see basename , the
 * \see directoryName , or the \see fileExtension . The second functionality of this class
 * is a platform-independent way of being notified of changes of the file. The constructor
 * or the \see setCallback methods expect an std::function object (possibly initialized
 * using a lambda-expression) that will be called whenever the file changes on the
 * harddisk. The callback function has this object passed as a parameter. If many changes
 * of the file happen in quick succession, each change will trigger a separate call of the
 * callback. The file system is not polled, but the changes are pushed to the application,
 * so the changes are registered efficiently.
 */
class File {
public:
    /// The type of the std::function that is used as the prototype for the callback
    typedef std::function<void (const File&)> FileChangedCallback;
    
    /**
     * This method constructs a new File object using a given <code>filename</code> as a
     * C string. <code>isRawPath</code> controls if the path is used without changes, or
     * if tokens (\see FileSystem) should be converted first. <code>fileChangedCallback
     * </code> will be called whenever the pointed file changes on the harddisk.
     * \param filename The path to the file this File object should point to
     * \param isRawPath If this value is <code>true</code>, the value of <code>filename
     * </code> is used as-is. If it is <code>false</code>, the path is converted into an
     * absolute path and any tokens, if present, are resolved
     * \param fileChangedCallback The callback function that is called once per change of
     * the file on the filesystem.
     */
    File(const char* filename, bool isRawPath = false,
         const FileChangedCallback& fileChangedCallback = FileChangedCallback());

    /**
     * This method constructs a new File object using a given <code>filename</code> as an
     * std string. <code>isRawPath</code> controls if the path is used without changes, or
     * if tokens (\see FileSystem) should be converted first. <code>fileChangedCallback
     * </code> will be called whenever the pointed file changes on the harddisk.
     * \param filename The path to the file this File object should point to
     * \param isRawPath If this value is <code>true</code>, the value of <code>filename
     * </code> is used as-is. If it is <code>false</code>, the path is converted into an
     * absolute path and any tokens, if present, are resolved
     * \param fileChangedCallback The callback function that is called once per change of
     * the file on the filesystem.
     */
    File(const std::string& filename, bool isRawPath = false,
         const FileChangedCallback& fileChangedCallback = FileChangedCallback());

    /**
     * The destructor will automatically stop the filesystem evaluation of any file
     */
    ~File();

    /**
     * Sets a new callback function that will be used for this File object. If there
     * has not been a callback before, there are no race conditions. If there has been a
     * registered callback before and the callback is changed from another thread, a race
     * condition might appear if a file is changed in the file system at the same time.
     * \param callback The new callback function that will be used in this File object
     */
    void setCallback(const FileChangedCallback& callback);
    
    /**
     * Returns the currently active std::function object. This object might be
     * uninitialized if not callback has been registered previously.
     * \return The currently active std::function object used as a callback function
     */
    const FileChangedCallback& callback() const;

    /**
     * Returns the full path to the file as an std::string
     * \return The full path to the file as an std::string
     */
    operator const std::string&() const;

    /**
     * Returns the full path to the file as an std::string
     * \return The full path to the file as an std::string
     */
    const std::string& path() const;

    /**
     * Returns the filename part of the full path. The filename is defined as the part of
     * the path after the last path separator (<code>'/'</code> or <code>'\\\\'</code>)
     * and including the extension. Example (<code>'/home/user/file.txt' -> 'file.txt'
     * </code>)
     * \return The filename part of the full path
     */
    std::string filename() const;
    
    /**
     * Returns the basename part of the full path. The basename is defined as the part of
     * the path between the last path separator (<code>'/'</code> or <code>'\\\\'</code>)
     * and the extension (separated by a '.' ; if existing). Example (<code>
     * '/home/user/file.txt' -> 'file'</code>)
     * \return The basename part of the full path
     */
    std::string baseName() const;
    
    /**
     * Returns the full basename of the path. The full basename is defined as the part of
     * the path before the extension (if existing). Example (<code>'/home/user/file.txt'
     * -> '/home/user/file'</code>)
     * \return The full basename part of the path
     */
    std::string fullBaseName() const;
    
    /**
     * Returns the directory name of the path. The directory name is defined as the part
     * of the path before the last path separator (<code>'/'</code> or <code>'\\\\'
     * </code>) and does not include the separator itself. Example(<code>
     * '/home/user/file.txt' -> '/home/user'>/code>)
     * \return The directory name of the path
     */
    std::string directoryName() const;
    
    /**
     * Returns the extension part of the full path. The extension is defined as the part
     * of the path after the exension separator (<code>'.'</code>). Example (<code>
     * '/home/user/file.txt' -> '/home/user/file'</code>).
     * \return The extension part of the full path
     */
    std::string fileExtension() const;

private:
    /**
     * Registers and starts the platform-dependent listener to file changes on disk. Will
     * remove the old listener in the process
     */
    void installFileChangeListener();
    
    /**
     * Removes the platform-dependent listener. If there is no listener present, this
     * operation is a no-op.
     */
    void removeFileChangeListener();

    std::string _filename; ///< The filename of this File
    FileChangedCallback _fileChangedCallback; ///< The callback that is called

#ifdef WIN32
    void beginRead();
    static void CALLBACK completionHandler(
        DWORD dwErrorCode,
        DWORD dwNumberOfBytesTransferred,
        LPOVERLAPPED lpOverlapped);		

    HANDLE _directoryHandle;
    unsigned char _activeBuffer;
    std::vector<BYTE> _changeBuffer[2];
    OVERLAPPED _overlappedBuffer;
#elif __APPLE__
    static void completionHandler(ConstFSEventStreamRef streamRef,
                                  void *clientCallBackInfo,
                                  size_t numEvents,
                                  void *eventPaths,
                                  const FSEventStreamEventFlags eventFlags[],
                                  const FSEventStreamEventId eventIds[]);
    
    FSEventStreamRef _eventStream;
    __darwin_time_t _lastModifiedTime; // typedef of 'long'
#endif
};

} // namespace filesystem
} // namespace ghoul

#endif // __FILE_H__
