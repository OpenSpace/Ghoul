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

#include <ghoul/filesystem/filesystem.h>

#include <ghoul/logging/logmanager.h>

#include <algorithm>
#include <cassert>
#include <regex>
#include <stdio.h>

#ifdef WIN32
#include <direct.h>
#include <windows.h>
#include <Shlobj.h>
#else
#include <dirent.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#endif

#if !defined(WIN32) && !defined(__APPLE__)
#include <sys/inotify.h>
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
 const uint32_t mask = IN_ALL_EVENTS | IN_IGNORED | IN_Q_OVERFLOW | IN_UNMOUNT | IN_ISDIR;
#endif

using std::string;
namespace {
    const string _loggerCat = "FileSystem";
}

namespace {
#ifdef WIN32
	const char pathSeparator = '\\';
	const unsigned int changeBufferSize = 16384u;

#define _CRT_SECURE_NO_WARNINGS
#elif __APPLE__
	const char pathSeparator = '/';
	// the maximum latency allowed before a changed is registered
	const CFAbsoluteTime latency = 1.0;
#else
	const char pathSeparator = '/';
#endif
}

namespace ghoul {
namespace filesystem {

FileSystem* FileSystem::_fileSystem = nullptr;
const std::string FileSystem::TokenOpeningBraces = "${";
const std::string FileSystem::TokenClosingBraces = "}";
#ifdef WIN32
const char FileSystem::PathSeparator = '\\';
#else
const char FileSystem::PathSeparator = '/';
#endif

FileSystem::FileSystem() {}

void FileSystem::initialize() {
    assert(_fileSystem == nullptr);
    if (_fileSystem == nullptr)
        _fileSystem = new FileSystem;

#if defined(WIN32)
#elif defined(__APPLE__)
#else
    _fileSystem->_inotifyHandle = inotify_init();
    _fileSystem->_keepGoing = true;
    _fileSystem->_t = std::thread(inotifyWatcher);
#endif
}

void FileSystem::deinitialize() {
    assert(_fileSystem != nullptr);
#ifdef WIN32
	for (auto d : _fileSystem->_directories) {
		DirectoryHandle* dh = d.second;
		CancelIo(dh->_handle);
		CloseHandle(dh->_handle);
		delete dh;
	}
#elif __APPLE__
    for (auto d : _fileSystem->_directories) {
        DirectoryHandle* dh = d.second;
        FSEventStreamStop(dh->_eventStream);
        FSEventStreamInvalidate(dh->_eventStream);
        FSEventStreamRelease(dh->_eventStream);
        delete dh;
    }
#else
	_fileSystem->_keepGoing = false;
	if (_fileSystem->_t.joinable())
		_fileSystem->_t.join();
	close( _fileSystem->_inotifyHandle );
#endif
    delete _fileSystem;
}

FileSystem& FileSystem::ref() {
    assert(_fileSystem != nullptr);
    return *_fileSystem;
}

string FileSystem::absolutePath(string path) const {
    expandPathTokens(path);

    static const int PATH_BUFFER_SIZE = 4096;
    char* buffer = nullptr;
#ifdef WIN32
    buffer = new char[PATH_BUFFER_SIZE];
    const DWORD success = GetFullPathName(path.c_str(), PATH_BUFFER_SIZE, buffer, 0);
    if (success == 0) {
        delete[] buffer;
        buffer = nullptr;
    }
#else
    char errorBuffer[PATH_BUFFER_SIZE];
    buffer = realpath(path.c_str(), errorBuffer);
    if (buffer == NULL) {
        LERROR("Error resolving the real path. Problem part: '" << errorBuffer << "'");
        return path;
    }
#endif

    if (buffer) {
        path = string(buffer);
#ifdef WIN32
        delete[] buffer;
#endif
        return path;
    }

    return path;
}

string FileSystem::relativePath(string path,
                                const Directory& baseDirectory) const
{
    if (path.empty()) {
        LERROR("'path' must contain a path");
        return path;
    }
    string&& pathAbsolute = cleanupPath(absolutePath(path));
    string&& directoryAbsolute = cleanupPath(absolutePath(baseDirectory));

    // Return identity path if absolutes are equal
    if (pathAbsolute == directoryAbsolute)
        return ".";

    // Check for different drives on Windows
    if (pathAbsolute[0] != directoryAbsolute[0])
        return path;

    // Find the common part in the 'path' and 'baseDirectory'
    size_t commonBasePosition = commonBasePathPosition(pathAbsolute, directoryAbsolute);
    string&& directoryRemainder = directoryAbsolute.substr(commonBasePosition);
    string relativePath = pathAbsolute.substr(commonBasePosition);
    if (relativePath[0] == PathSeparator)
        relativePath = relativePath.substr(1);

    // Construct the relative path by iteratively subtracting the additional folders from
    // 'directoryRemainder'
    size_t position = directoryRemainder.find(PathSeparator);
    while (position != string::npos) {
        if (relativePath.empty())
            relativePath = "..";
        else
            relativePath = ".." + (PathSeparator + relativePath);
        position = directoryRemainder.find(PathSeparator, position + 1);
    }
    return relativePath;
}
    
std::string FileSystem::pathByAppendingComponent(std::string path,
												 std::string component) const
{
	return path + PathSeparator + component;
}

Directory FileSystem::currentDirectory() const {
#ifdef WIN32
    // Get the size of the directory
    DWORD size = GetCurrentDirectory(0, NULL);
    char* buffer = new char[size];
    DWORD success = GetCurrentDirectory(size, buffer);
    if (success == 0) {
        // Log error
        DWORD error = GetLastError();
        LPTSTR errorBuffer = nullptr;
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL,
                      error,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (LPTSTR)&errorBuffer,
                      0,
                      NULL);
        if (errorBuffer != nullptr) {
            string error(errorBuffer);
            LocalFree(errorBuffer);
            LERROR("Error retrieving current directory: " << error);
        }
        return Directory();
    }
    string&& currentDir = std::move(string(buffer));
    delete[] buffer;
#else
    char* buffer = new char[MAXPATHLEN];
    char* result = getcwd(buffer, MAXPATHLEN);
    if (result == nullptr) {
        LERROR("Error retrieving current directory: " << errno);
        return Directory();
    }
    string currentDir = string(buffer);
    delete[] buffer;
#endif
    return Directory(currentDir);
}
    
void FileSystem::setCurrentDirectory(const Directory& directory) const {
#ifdef WIN32
    const BOOL success = SetCurrentDirectory(directory.path().c_str());
    if (success == 0) {
        // Log error
        DWORD error = GetLastError();
        LPTSTR errorBuffer = nullptr;
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL,
                      error,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (LPTSTR)&errorBuffer,
                      0,
                      NULL);
        if (errorBuffer != nullptr) {
            string error(errorBuffer);
            LocalFree(errorBuffer);
            LERROR("Error setting current directory: " << error);
        }
    }
#else
    const int success = chdir(directory.path().c_str());
    if (success != 0)
        LERROR("Error setting current directory: " << errno);
#endif
}

bool FileSystem::fileExists(const File& path) const {
	return fileExists(path.path(),true);
}

bool FileSystem::fileExists(std::string path, bool isRawPath) const {
	if (!isRawPath)
		path = absPath(path);
#ifdef WIN32
	const DWORD attributes = GetFileAttributes(path.c_str());
	if (attributes == INVALID_FILE_ATTRIBUTES) {
		const DWORD error = GetLastError();
		if ((error != ERROR_FILE_NOT_FOUND) && (error != ERROR_PATH_NOT_FOUND)) {
			LPTSTR errorBuffer = nullptr;
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				error,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&errorBuffer,
				0,
				NULL);
			if (errorBuffer != nullptr) {
				string error(errorBuffer);
				LocalFree(errorBuffer);
				LERROR("Error retrieving file attributes: " << error);
			}
		}
		return false;
	}
	else
		return (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
	struct stat buffer;
	const int statResult = stat(path.c_str(), &buffer);
	if (statResult != 0)
		return false;
	const int isFile = S_ISREG(buffer.st_mode);
	return (isFile != 0);
#endif
}

bool FileSystem::directoryExists(const Directory& path) const {
#ifdef WIN32
    const DWORD attributes = GetFileAttributes(path.path().c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        const DWORD error = GetLastError();
        if ((error != ERROR_FILE_NOT_FOUND) && (error != ERROR_PATH_NOT_FOUND)) {
            LPTSTR errorBuffer = nullptr;
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                error,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&errorBuffer,
                0,
                NULL);
            if (errorBuffer != nullptr) {
                string error(errorBuffer);
                LocalFree(errorBuffer);
                LERROR("Error retrieving file attributes: " << error);
            }
        }
        return false;
    }
    else
        return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
    const string& dirPath = path.path();
    struct stat buffer;
    const int statResult = stat(dirPath.c_str(), &buffer);
    if (statResult != 0)
        return false;
    const int isDir = S_ISDIR(buffer.st_mode);
    return (isDir != 0);
#endif
}
    
bool FileSystem::deleteFile(const File& path) const {
    const bool isFile = fileExists(path);
    if (isFile) {
        const int removeResult = remove(path.path().c_str());
        return removeResult == 0;
    }
    else
        return false;
}
    
bool FileSystem::createDirectory(const Directory& path, bool recursive) const {
	if (recursive) {
		std::vector<Directory> directories;
		Directory d = path;
        while (!FileSys.directoryExists(d)) {
            //LERROR("Adding path to v: " << d.path());
			directories.push_back(d);
			d = d.parentDirectory();
		}

		bool success = true;
		std::for_each(
			directories.rbegin(),
			directories.rend(),
			[&success, this](const Directory& d)
			   {
				if (!success)
					return;
				else
					success = createDirectory(d, false);
				});

		return success;
	}
	else {
#ifdef WIN32
		BOOL success = CreateDirectory(path.path().c_str(), NULL);
		if (success)
			return true;
		else {
			DWORD error = GetLastError();
			if (ERROR == ERROR_ALREADY_EXISTS)
				return true;
			else {
				LPTSTR errorBuffer = nullptr;
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
							  FORMAT_MESSAGE_ALLOCATE_BUFFER |
							  FORMAT_MESSAGE_IGNORE_INSERTS,
							  NULL,
							  error,
							  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							  (LPTSTR)&errorBuffer,
							  0,
							  NULL);
				if (errorBuffer != nullptr) {
					string error(errorBuffer);
					LocalFree(errorBuffer);
					LERROR("Error creating directory '" << path << "': " << error);
				}
				else 
					LERROR("Error creating directory '" << path << "'");

				return false;
			}
		}
#else
		int success = mkdir(path.path().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		if (success == 0)
			return true;
		else {
			if (errno == EEXIST)
				return true;
			else {
				LERROR("Error creating directory '" << path << "': " << errno);
				return false;
			}
		}
#endif
	}
}

bool FileSystem::deleteDirectory(const Directory& path) const {
    const bool isDir = directoryExists(path);
    if (!isDir)
        return false;
#ifdef WIN32
    const int rmDirResult = _rmdir(path.path().c_str());
    auto dirs =
    return rmDirResult != -1;
#else
    const string& dirPath = path;
    DIR* directory = opendir(dirPath.c_str());
    bool success = false;
    
    if (directory) {
        struct dirent* p;
        
        success = true;
        
        while (success) {
            p = readdir(directory);
            if (p == nullptr)
                break;
            
            const string name = string(p->d_name);
            if (name == "." || name == "..")
                continue;

            struct stat statbuf;
            const string fullName = dirPath + "/" + name;
            const int statResult = stat(fullName.c_str(), &statbuf);
            if (statResult == 0) {
                if (S_ISDIR(statbuf.st_mode))
                    success &= deleteDirectory(fullName);
                else {
                    const int removeSuccess = remove(fullName.c_str());
                    success &= (removeSuccess == 0);
                }
            }
        }
        closedir(directory);
    }
    
    if (success) {
        const int rmdirSuccess = rmdir(dirPath.c_str());
        success = (rmdirSuccess == 0);
    }
    
    return success;
#endif
}

void FileSystem::registerPathToken(string token, string path, bool override) {
#ifdef GHL_DEBUG
    if (token.empty()) {
        LERROR("Token cannot not be empty");
        return;
    }
    
    std::string&& beginning = token.substr(0, TokenOpeningBraces.size());
    std::string&& ending = token.substr(token.size() - TokenClosingBraces.size());
    if ((beginning != TokenOpeningBraces) || (ending != TokenClosingBraces)) {
        LERROR("Token has to start with '" + TokenOpeningBraces +
                    "' and end with '" + TokenClosingBraces + "'");
        return;
    }
    
    
	if (!override) {
		if (_fileSystem->_tokenMap.find(token) != _fileSystem->_tokenMap.end()) {
			LERROR("Token already bound to path '" +
						_fileSystem->_tokenMap[token] + "'");
			return;
		}
	}
#endif
	if (override) {
		auto it = _fileSystem->_tokenMap.find(token);
		_fileSystem->_tokenMap.erase(it);
	}
	_fileSystem->_tokenMap.emplace(token, path);
}
    
string FileSystem::cleanupPath(string path) const {
#ifdef WIN32
    // In Windows, replace all '/' by '\\' for conformity
    std::replace(path.begin(), path.end(), '/', '\\');
    std::string&& drivePart = path.substr(0, 3);
    std::regex&& driveRegex = std::regex("([[:lower:]][\\:][\\\\])");
    const bool hasCorrectSize = path.size() >= 3;
    if (hasCorrectSize && std::regex_match(drivePart, driveRegex))
        std::transform(path.begin(), path.begin() + 1,
                        path.begin(), toupper);
#else
    // Remove all double separators (will automatically be done on Windows)
#endif
    size_t position = 0;
    while (position != string::npos) {
        char dualSeparator[2];
        dualSeparator[0] = PathSeparator;
        dualSeparator[1] = PathSeparator;
        position = path.find(dualSeparator);
        if (position != string::npos)
            path = std::move(path.substr(0, position) + path.substr(position + 1));
    }
    
    // Remove trailing separator
    if (path[path.size() - 1] == PathSeparator)
        path = std::move(path.substr(0, path.size() - 1));
    
    return path;
}
    
size_t FileSystem::commonBasePathPosition(const string& p1, const string& p2) const {
    // 'currentPosition' stores the position until which the two paths are the same,
    // 'nextPosition' is a look-ahead. If the look-ahead is equal as well,
    // 'currentPosition' is replaced by this. At the end of the loop 'currentPosition'
    // contains the last position until which both paths are the same
    size_t currentPosition = 0;
    size_t nextPosition = p1.find(PathSeparator);
    while (nextPosition != string::npos) {
        const int result = p1.compare(0, nextPosition, p2, 0 , nextPosition);
        if (result == 0) {
            currentPosition = nextPosition;
            nextPosition = p1.find(PathSeparator, nextPosition + 1);
        }
        else
            break;
    }
    const int result = p1.compare(0, p1.length(), p2, 0 , p1.length());
    if (result == 0)
        currentPosition = p1.length();
    return currentPosition;
}
    
bool FileSystem::hasTokens(const string& path) const {
    const bool hasOpeningBrace = path.find(TokenOpeningBraces) != string::npos;
    const bool hasClosingBrace = path.find(TokenClosingBraces) != string::npos;
    return hasOpeningBrace && hasClosingBrace;
}

bool FileSystem::expandPathTokens(std::string& path) const {
    while (hasTokens(path)) {
        string::size_type beginning = path.find(TokenOpeningBraces);
        string::size_type closing = path.find(TokenClosingBraces);
        string::size_type closingLocation = closing + TokenClosingBraces.size();
        const std::string currentToken = path.substr(beginning, closingLocation);
        const std::string& replacement = resolveToken(currentToken);
        if (replacement == currentToken) {
            // replacement == currentToken will be true if the respective token could not
            // be found;  resolveToken will print an error in that case
            return false;
        }
        path.replace(beginning, closing + TokenClosingBraces.size() - beginning, replacement);
    }
    return true;
}

std::vector<std::string> FileSystem::tokens() const {
	std::vector<std::string> tokens;
	for (auto token : _tokenMap) {
		tokens.push_back(token.first);
	}
	return tokens;
}

void FileSystem::addFileListener(File* file) {
	assert(file != nullptr);
#if defined(WIN32)			// Windows
	//LDEBUG("Trying to insert  " << file);
	std::string d = file->directoryName();
	auto f = _directories.find(d);
	if (f == _directories.end()) {
		LDEBUG("started watching: " << d);
		DirectoryHandle* handle = new DirectoryHandle;
		handle->_activeBuffer = 0;
		handle->_handle = nullptr;

		handle->_handle = CreateFile(
			d.c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			NULL);

		if (handle->_handle == INVALID_HANDLE_VALUE) {
			LERROR("Directory handle for '" << d << "' could not be obtained");
			return;
		}

		_directories[d] = handle;
		beginRead(handle);
	}

#ifdef GHL_DEBUG
	auto eqRange = _trackedFiles.equal_range(file->path());
	// Erase (b,15) pair
	for (auto it = eqRange.first; it != eqRange.second; ++it) {
		if (it->second == file) {
			LERROR("Already tracking fileobject");
			return;
		}
	}
#endif
	_trackedFiles.insert({ file->path(), file });

#elif defined(__APPLE__)	// OS X
    //LDEBUG("Trying to insert  " << file);
    std::string d = file->directoryName();
    auto f = _directories.find(d);
    if (f == _directories.end()) {
    
        bool alreadyTrackingParent = false;
        for(auto dir: _directories) {
            if (d.length() > dir.first.length() && d.find_first_of(dir.first) == 0) {
                alreadyTrackingParent = true;
                break;
            }
        }
        if(!alreadyTrackingParent) {
            LDEBUG("started watching: " << d);
            DirectoryHandle* handle = new DirectoryHandle;
            
            // Create the FSEventStream responsible for this directory (Apple's callback system
            // only works on the granularity of the directory)
            CFStringRef path = CFStringCreateWithCString(NULL,
                                                         d.c_str(),
                                                         kCFStringEncodingASCII);
            CFArrayRef pathsToWatch = CFArrayCreate(NULL, (const void **)&path, 1, NULL);
            FSEventStreamContext callbackInfo;
            callbackInfo.version = 0;
            callbackInfo.info = nullptr;
            callbackInfo.release = NULL;
            callbackInfo.retain = NULL;
            callbackInfo.copyDescription = NULL;
            
            handle->_eventStream = FSEventStreamCreate(
                                                       NULL,
                                                       &completionHandler,
                                                       &callbackInfo,
                                                       pathsToWatch,
                                                       kFSEventStreamEventIdSinceNow,
                                                       latency,
                                                       kFSEventStreamCreateFlagFileEvents);
            
            // Add checking the event stream to the current run loop
            // If there is a performance bottleneck, this could be done on a separate thread?
            FSEventStreamScheduleWithRunLoop( handle->_eventStream,
                                             CFRunLoopGetCurrent(),
                                             kCFRunLoopDefaultMode);
            // Start monitoring
            FSEventStreamStart( handle->_eventStream);
            _directories[d] = handle;
        }
    }
    
#ifdef GHL_DEBUG
    auto eqRange = _trackedFiles.equal_range(file->path());
    // Erase (b,15) pair
    for (auto it = eqRange.first; it != eqRange.second; ++it) {
        if (it->second == file) {
            LERROR("Already tracking fileobject");
            return;
        }
    }
#endif
    _trackedFiles.insert({ file->path(), file });
#else						// Linux
	const std::string filename = fileobject->path();
	LDEBUGC("inotifyWatcher", "Wathcing: " << filename);
	int wd = inotify_add_watch(_inotifyHandle, filename.c_str(), mask);
	_inotifyFiles.insert(std::pair<int, File*>(wd, fileobject));
#endif
}

void FileSystem::removeFileListener(File* file) {
	assert(file != nullptr);
#if defined(WIN32) || defined(__APPLE__)
	auto eqRange = _trackedFiles.equal_range(file->path());
	for (auto it = eqRange.first; it != eqRange.second; ++it) {
		//LDEBUG("comparing for removal, " << file << "==" << it->second);
		if (it->second == file) {
			//LWARNING("Removing tracking of " << file);
			_trackedFiles.erase(it);
			return;
		}
	}

#else						// Linux
	std::map<int,File*>::iterator it;
	for(it = _inotifyFiles.begin();it != _inotifyFiles.end(); it++) {
		if(it->second == fileobject) {
			( void ) inotify_rm_watch( _inotifyHandle, it->first );
			_inotifyFiles.erase(it);
			return;
		}
	}
#endif
    LWARNING("Could not find tracked '" << file <<"' for path '"<< file->path() << "'");
}

void FileSystem::triggerFilesystemEvents() {
#ifdef WIN32
	// Sleeping for 0 milliseconds will trigger any pending asynchronous procedure calls 
	SleepEx(0, TRUE);
#endif
#if defined(__APPLE__)
    //osxPollEvents();
    
    for(auto d: _directories) {
        FSEventStreamFlushSync(d.second->_eventStream);
        //FSEventStreamFlushAsync(d.second->_eventStream);
    }
    
#endif
}


bool FileSystem::hasToken(const std::string& path, const std::string& token) const {
    if (!hasTokens(path))
        return false;
    else {
        string::size_type beginning = path.find(TokenOpeningBraces);
        string::size_type closing = path.find(TokenClosingBraces);
        while ((beginning != string::npos) && (closing != string::npos)) {
            string::size_type closingLocation = closing + TokenClosingBraces.size();
            const std::string currentToken = path.substr(beginning, closingLocation);
            if (currentToken == token)
                return true;
            else {
                beginning = path.find(TokenOpeningBraces, closing);
                closing = path.find(TokenClosingBraces, beginning);
            }
        }
        return false;
    }
}
    
std::string FileSystem::resolveToken(const std::string& token) const {
    const std::map<std::string, std::string>::const_iterator it = _tokenMap.find(token);
    if (it == _tokenMap.end()) {
        LERROR("Token '" + token + "' could not be resolved");
        return token;
    }
    else
        return it->second;
}

#ifdef WIN32
void CALLBACK FileSystem::completionHandler(
	DWORD /*dwErrorCode*/, 
	DWORD /*dwNumberOfBytesTransferred*/,
	LPOVERLAPPED lpOverlapped)
{
	//File* file = static_cast<File*>(lpOverlapped->hEvent);
	DirectoryHandle* directoryHandle = static_cast<DirectoryHandle*>(lpOverlapped->hEvent);

	unsigned char currentBuffer = directoryHandle->_activeBuffer;

	// Change active buffer (ping-pong buffering)
	directoryHandle->_activeBuffer = (directoryHandle->_activeBuffer + 1) % 2;
	// Restart change listener as soon as possible
	FileSys.beginRead(directoryHandle);

	char* buffer = reinterpret_cast<char*>(&(directoryHandle->_changeBuffer[currentBuffer][0]));
	// data might have queued up, so we need to check all changes
	//LERROR("callback");
	while (true) {
		// extract the information which file has changed
		FILE_NOTIFY_INFORMATION& information = (FILE_NOTIFY_INFORMATION&)*buffer;
		char* currentFilenameBuffer = new char[information.FileNameLength];
		size_t i;
		wcstombs_s(&i, currentFilenameBuffer, information.FileNameLength,
			information.FileName, information.FileNameLength);
		//std::wcstombs(currentFilenameBuffer,
		//information.FileName, information.FileNameLength);
		const string& currentFilename(currentFilenameBuffer);
		delete[] currentFilenameBuffer;
		//LDEBUG("callback: " << currentFilename);

		std::string fullPath;
		for (auto d : FileSys._directories) {
			if (d.second == directoryHandle)
				fullPath = d.first + pathSeparator + currentFilename;
		}
		//LDEBUG("callback: " << fullPath);

		size_t n = FileSys._trackedFiles.count(fullPath);
		if (n > 0) {
			/*
			LWARNING("Tracked files");
			for (auto ff : FileSys._trackedFiles) {
				LDEBUG(ff.first);
			}
			*/
			//LDEBUG("Loop for " << fullPath);
			auto eqRange = FileSys._trackedFiles.equal_range(fullPath);
			for (auto it = eqRange.first; it != eqRange.second; ++it) {
				File* f = (*it).second;
				f->_fileChangedCallback(*f);
				//LWARNING("calling for: " << f);
			}
			//LDEBUG("end Loop for " << fullPath);
			break;
		}
		else {
			if (!information.NextEntryOffset)
				// we are done with all entries and didn't find our file
				break;
			else
				//continue with the next entry
				buffer += information.NextEntryOffset;
		}

	}
}

void FileSystem::beginRead(DirectoryHandle* directoryHandle) {


	HANDLE handle = directoryHandle->_handle;
	unsigned char activeBuffer = directoryHandle->_activeBuffer;
	std::vector<BYTE>* changeBuffer = directoryHandle->_changeBuffer;
	OVERLAPPED* overlappedBuffer = &directoryHandle->_overlappedBuffer;

	ZeroMemory(overlappedBuffer, sizeof(OVERLAPPED));
	overlappedBuffer->hEvent = directoryHandle;

	changeBuffer[activeBuffer].resize(changeBufferSize);
	ZeroMemory(&(changeBuffer[activeBuffer][0]), changeBufferSize);

	DWORD returnedBytes;
	BOOL success = ReadDirectoryChangesW(
		handle,
		&changeBuffer[activeBuffer][0],
		static_cast<DWORD>(changeBuffer[activeBuffer].size()),
		false,
		FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE,
		&returnedBytes,
		overlappedBuffer,
		&completionHandler);

	if (success == 0) {
		LERROR("no begin read");
		const DWORD error = GetLastError();
		LPTSTR errorBuffer = nullptr;
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&errorBuffer,
			0,
			NULL);
		if (errorBuffer != nullptr) {
			std::string errorString(errorBuffer);
			LocalFree(errorBuffer);
			LERROR("Error reading directory changes: " << errorString);
		}
		else {
			LERROR("Error reading directory changes: " << error);
		}
	}
}

#elif __APPLE__

void FileSystem::completionHandler(
	ConstFSEventStreamRef ,//streamRef,
	void * ,//clientCallBackInfo,
	size_t numEvents,
	void *eventPaths,
	const FSEventStreamEventFlags eventFlags[],
	const FSEventStreamEventId[] )//eventIds[])
{
    char **paths = reinterpret_cast<char**>(eventPaths);
    for (size_t i=0; i<numEvents; i++) {
        //std::string ename = EventEnumToName(static_cast<Events>(eventFlags[i]));
        
        //printf("%s\n%s\n", path.c_str(), ename.c_str());
        if(! eventFlags[i] & Events::kFSEventStreamEventFlagItemModified)
            continue;
        
        if(! eventFlags[i] & Events::kFSEventStreamEventFlagItemIsFile)
            continue;
        
        std::string path = paths[i];
        size_t n = FileSys._trackedFiles.count(path);
        if (n == 0)
            continue;
        
        auto eqRange = FileSys._trackedFiles.equal_range(path);
        for (auto it = eqRange.first; it != eqRange.second; ++it) {
            File* f = (*it).second;
            f->_fileChangedCallback(*f);
        }
    }
}
std::string FileSystem::EventEnumToName(Events e) {
    std::string name;
    if(e & kFSEventStreamEventFlagMustScanSubDirs)
        name += "| kFSEventStreamEventFlagMustScanSubDirs";
    if(e & kFSEventStreamEventFlagUserDropped)
        name += "| kFSEventStreamEventFlagUserDropped";
    if(e & kFSEventStreamEventFlagKernelDropped)
        name += "| kFSEventStreamEventFlagKernelDropped";
    if(e & kFSEventStreamEventFlagEventIdsWrapped)
        name += "| kFSEventStreamEventFlagEventIdsWrapped";
    if(e & kFSEventStreamEventFlagHistoryDone)
        name += "| kFSEventStreamEventFlagHistoryDone";
    if(e & kFSEventStreamEventFlagRootChanged)
        name += "| kFSEventStreamEventFlagRootChanged";
    if(e & kFSEventStreamEventFlagMount)
        name += "| kFSEventStreamEventFlagMount";
    if(e & kFSEventStreamEventFlagUnmount)
        name += "| kFSEventStreamEventFlagUnmount";
    
    
    if(e & kFSEventStreamEventFlagItemCreated)
        name += "| kFSEventStreamEventFlagItemCreated";
    if(e & kFSEventStreamEventFlagItemRemoved)
        name += "| kFSEventStreamEventFlagItemRemoved";
    if(e & kFSEventStreamEventFlagItemInodeMetaMod)
        name += "| kFSEventStreamEventFlagItemInodeMetaMod";
    if(e & kFSEventStreamEventFlagItemRenamed)
        name += "| kFSEventStreamEventFlagItemRenamed";
    if(e & kFSEventStreamEventFlagItemModified)
        name += "| kFSEventStreamEventFlagItemModified";
    if(e & kFSEventStreamEventFlagItemFinderInfoMod)
        name += "| kFSEventStreamEventFlagItemFinderInfoMod";
    if(e & kFSEventStreamEventFlagItemChangeOwner)
        name += "| kFSEventStreamEventFlagItemChangeOwner";
    if(e & kFSEventStreamEventFlagItemXattrMod)
        name += "| kFSEventStreamEventFlagItemXattrMod";
    if(e & kFSEventStreamEventFlagItemIsFile)
        name += "| kFSEventStreamEventFlagItemIsFile";
    if(e & kFSEventStreamEventFlagItemIsDir)
        name += "| kFSEventStreamEventFlagItemIsDir";
    if(e & kFSEventStreamEventFlagItemIsSymlink)
        name += "| kFSEventStreamEventFlagItemIsSymlink";
    
    if (name.length() > 2) {
        name = name.substr(2);
    }
    return name;
}

#else // Linux

void FileSystem::inotifyWatcher() {

	int fd = FileSys._inotifyHandle;
    char buffer[BUF_LEN];
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    fd_set rfds;
    while(FileSys._keepGoing) {
        FD_ZERO (&rfds);
        FD_SET (fd, &rfds);
        if(select (FD_SETSIZE, &rfds, NULL, NULL, &tv) < 1) continue; 
        
        int length = read( fd, buffer, BUF_LEN );
        if ( length < 0 ) continue;
        
        int offset = 0;
        while (offset < length) {
            struct inotify_event *event = (inotify_event*)(buffer + offset);
            switch (event->mask )
            {

                case IN_MODIFY:
                case IN_ATTRIB:
                {
                    int wd = event->wd;
                    //printf ("Calling _fileChangedCallback, %i, %i\n", fd, wd);
                    std::map<int,File*>::iterator it;
                    it = FileSys._inotifyFiles.find(wd);
                    if(it != FileSys._inotifyFiles.end()) {
                        File* fileobject = it->second;
                        fileobject->_fileChangedCallback(*fileobject);
                    }
                    
                }
                    //printf ("IN_MODIFY\n");
                    //printf ("IN_ATTRIB\n");

                    break;

                case IN_IGNORED:
                {
                    int wd = event->wd;
                    //printf ("Calling _fileChangedCallback, %i, %i\n", fd, wd);
                    std::map<int,File*>::iterator it;
                    it = FileSys._inotifyFiles.find(wd);
                    if(it != FileSys._inotifyFiles.end()) {
                        File* fileobject = it->second;
                        FileSys._inotifyFiles.erase (wd);
                        ( void ) inotify_rm_watch( fd, wd );
                        wd = inotify_add_watch( fd, fileobject->path().c_str(), mask);
                        FileSys._inotifyFiles.insert ( std::pair<int,File*>(wd,fileobject) );
                        
                    }
                    
                }
                    //printf ("IN_IGNORED\n");
                    break;
                default:
                    break;
            }
            offset += EVENT_SIZE + event->len;
        }
    }
}

#endif

} // namespace filesystem
} // namespace ghoul
