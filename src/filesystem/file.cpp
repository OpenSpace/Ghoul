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

#include <ghoul/filesystem/file.h>

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/logging/logmanager.h>

#include <ctime>

using std::function;
using std::string;

namespace ghoul {
namespace filesystem {

namespace {
const string _loggerCat = "File";
#ifdef WIN32
const char pathSeparator = '\\';
const unsigned int changeBufferSize = 16384u;

#define _CRT_SECURE_NO_WARNINGS
#elif __APPLE__
const char pathSeparator = '/';
// the maximum latency allowed before a changed is registered
const CFAbsoluteTime latency = 3.0;
#else
const char pathSeparator = '/';
#endif
}

File::File(const char* filename, bool isRawPath,
           FileChangedCallback fileChangedCallback)
    : _fileChangedCallback(std::move(fileChangedCallback))
#ifdef WIN32
    , _directoryHandle(nullptr)
    , _activeBuffer(0)
#elif __APPLE__
    , _eventStream(nullptr)
    , _lastModifiedTime(0)
#endif
{
    if (isRawPath)
        _filename = string(filename);
    else
        _filename = FileSys.absolutePath(string(filename));

    if (_fileChangedCallback)
        installFileChangeListener();
}

File::File(std::string filename, bool isRawPath,
           FileChangedCallback fileChangedCallback)
    : _fileChangedCallback(std::move(fileChangedCallback))
#ifdef WIN32
    , _directoryHandle(nullptr)
    , _activeBuffer(0)
#elif __APPLE__
    , _eventStream(nullptr)
    , _lastModifiedTime(0)
#endif
{
    if (isRawPath)
        _filename = std::move(filename);
    else
        _filename = std::move(FileSys.absolutePath(std::move(filename)));

    if (_fileChangedCallback)
        installFileChangeListener();
}
    
File::~File() {
    removeFileChangeListener();
}

void File::setCallback(FileChangedCallback callback) {
    if (_fileChangedCallback)
        removeFileChangeListener();
    _fileChangedCallback = std::move(callback);
    if (_fileChangedCallback)
        installFileChangeListener();
}

const File::FileChangedCallback& File::callback() const {
    return _fileChangedCallback;
}

File::operator const std::string&() const {
    return _filename;
}

const std::string& File::path() const {
    return _filename;
}

std::string File::filename() const {
    string::size_type separator = _filename.rfind(pathSeparator);
    if (separator != string::npos)
        return _filename.substr(separator + 1);
    else
        return _filename;
}

string File::baseName() const {
    string&& fileName = filename();
    string::size_type dot = fileName.rfind(".");
    if (dot != string::npos)
        return fileName.substr(0, dot);
    else
        return fileName;
}

string File::fullBaseName() const {
    string::size_type dot = _filename.rfind(".");
    if (dot != string::npos)
        return _filename.substr(0, dot);
    else
        return _filename;
}

string File::directoryName() const {
    string::size_type separator = _filename.rfind(pathSeparator);
    if (separator != string::npos)
        return _filename.substr(0, separator);
    else
        return _filename;
}

string File::fileExtension() const {
    string::size_type dot = _filename.rfind(".");
    if (dot != string::npos)
        return _filename.substr(dot + 1);
    else
        return _filename;
}
    
std::string File::lastModifiedDate() const {
	if (!FileSys.fileExists(_filename)) {
		LERROR("Error retrieving last-modified date for file '" << _filename << "'." <<
			"File did not exist");
		return "";
	}
#ifdef WIN32
	HANDLE fileHandle = CreateFile(
		_filename.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		NULL,
		NULL);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        LERROR("File handle for '" << _filename << "' could not be obtained");
        return "";
    }

	LPFILETIME lastWriteTime = NULL;
	BOOL success = GetFileTime(fileHandle, NULL, NULL, lastWriteTime);
    if (success == 0) {
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
            std::string error(errorBuffer);
			LERROR("Could not retrieve last-modified date for file '" << _filename <<
				"':" << error);
            LocalFree(errorBuffer);
        }
		return "";
    }
	else {
		LPSYSTEMTIME time = NULL;
		FileTimeToSystemTime(lastWriteTime, time);
		return std::to_string(time->wYear) + "-" + std::to_string(time->wMonth) + "-" +
			std::to_string(time->wDay) + "T" + std::to_string(time->wHour) + ":" +
			std::to_string(time->wMinute) + ":" + std::to_string(time->wSecond) + "." +
			std::to_string(time->wMilliseconds);
	}
#else
	struct stat attrib;
	stat(_filename.c_str(), &attrib);
	struct tm* time = gmtime(&(attrib.st_ctime));
	char buffer[128];
	strftime(buffer, 128, "%Y-%m-%dT%H:%M:%S");
	return buffer;
#endif
}


void File::installFileChangeListener() {
    removeFileChangeListener();
#ifdef WIN32
	string&& directory = directoryName();
    // Create a handle to the directory that is non-blocking
    _directoryHandle = CreateFile(
        directory.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL);

    if (_directoryHandle == INVALID_HANDLE_VALUE) {
        LERROR("Directory handle for '" << _filename << "' could not be obtained");
        return;
    }

    beginRead();
#elif __APPLE__
	string&& directory = directoryName();
    // Get the current last-modified time
    struct stat fileinfo;
    stat(_filename.c_str(), &fileinfo);
    _lastModifiedTime = fileinfo.st_mtimespec.tv_sec;
    
    // Create the FSEventStream responsible for this directory (Apple's callback system
    // only works on the granularity of the directory)
    CFStringRef path = CFStringCreateWithCString(NULL,
                                                 directory.c_str(),
                                                 kCFStringEncodingASCII);
    CFArrayRef pathsToWatch = CFArrayCreate(NULL, (const void **)&path, 1, NULL);
    FSEventStreamContext callbackInfo;
    callbackInfo.version = 0;
    callbackInfo.info = this;
    callbackInfo.release = NULL;
    callbackInfo.retain = NULL;
    callbackInfo.copyDescription = NULL;
    
    _eventStream = FSEventStreamCreate(
                    NULL,
                    &completionHandler,
                    &callbackInfo,
                    pathsToWatch,
                    kFSEventStreamEventIdSinceNow,
                    latency,
                    kFSEventStreamEventFlagItemModified);

    // Add checking the event stream to the current run loop
    // If there is a performance bottleneck, this could be done on a separate thread?
    FSEventStreamScheduleWithRunLoop(_eventStream,
                                     CFRunLoopGetCurrent(),
                                     kCFRunLoopDefaultMode);
    // Start monitoring
    FSEventStreamStart(_eventStream);
#else // Linux
    FileSys.inotifyAddListener(this);
#endif
}

void File::removeFileChangeListener() {
#ifdef WIN32
    if (_directoryHandle != nullptr) {
        CancelIo(_directoryHandle);
        CloseHandle(_directoryHandle);
        _directoryHandle = nullptr;
    }
#elif __APPLE__
    if (_eventStream != nullptr) {
        FSEventStreamStop(_eventStream);
        FSEventStreamInvalidate(_eventStream);
        FSEventStreamRelease(_eventStream);
        _eventStream = nullptr;
    }
#else
    FileSys.inotifyRemoveListener(this);
#endif
}

#ifdef WIN32
void CALLBACK File::completionHandler(DWORD /*dwErrorCode*/, DWORD,
                                      LPOVERLAPPED lpOverlapped)
{
    File* file = static_cast<File*>(lpOverlapped->hEvent);

    unsigned char currentBuffer = file->_activeBuffer;

    // Change active buffer (ping-pong buffering)
    file->_activeBuffer = (file->_activeBuffer + 1) % 2;
    // Restart change listener as soon as possible
    file->beginRead();

    string&& thisFilename = file->filename();

    char* buffer = reinterpret_cast<char*>(&(file->_changeBuffer[currentBuffer][0]));
    // data might have queued up, so we need to check all changes
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

        if (currentFilename == thisFilename) {
            // if it is the file we are interested in, call the callback
            file->_fileChangedCallback(*file);
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

void File::beginRead() {
    ZeroMemory(&_overlappedBuffer, sizeof(OVERLAPPED));
    _overlappedBuffer.hEvent = this;

    _changeBuffer[_activeBuffer].resize(changeBufferSize);
	ZeroMemory(&(_changeBuffer[_activeBuffer][0]), changeBufferSize);

    DWORD returnedBytes;
    BOOL success = ReadDirectoryChangesW(
        _directoryHandle,
        &_changeBuffer[_activeBuffer][0],
        static_cast<DWORD>(_changeBuffer[_activeBuffer].size()),
        false,
        FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE,
        &returnedBytes,
        &_overlappedBuffer,
        &completionHandler);

    if (success == 0) {
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
            std::string error(errorBuffer);
            LERROR("Error reading directory changes: " << error);
            LocalFree(errorBuffer);
        }
    }
}
    
#elif __APPLE__

void File::completionHandler(
                             ConstFSEventStreamRef,
                             void *clientCallBackInfo,
                             size_t numEvents,
                             void *eventPaths,
                             const FSEventStreamEventFlags[],
                             const FSEventStreamEventId[])
{
    File* fileObj = reinterpret_cast<File*>(clientCallBackInfo);
    char** paths = reinterpret_cast<char**>(eventPaths);
    for (size_t i=0; i<numEvents; i++) {
        const string path = string(paths[i]);
        const string directory = fileObj->directoryName() + '/';
        if (path == directory) {
            struct stat fileinfo;
            stat(fileObj->_filename.c_str(), &fileinfo);
            if (fileinfo.st_mtimespec.tv_sec != fileObj->_lastModifiedTime) {
                fileObj->_lastModifiedTime = fileinfo.st_atimespec.tv_sec;
                fileObj->_fileChangedCallback(*fileObj);
            }
        }
    }
}
#else // Linux

#endif

std::ostream& operator<<(std::ostream& os, const File& f) {
    return os << f.path();
}

} // namespace filesystem
} // namespace ghoul
