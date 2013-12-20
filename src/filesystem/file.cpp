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


#include <ghoul/filesystem/file.h>

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/logging/logmanager.h>

using std::function;
using std::string;

namespace ghoul {
namespace filesystem {

namespace {
    const string _loggerCat = "File";
#ifdef _WIN32
    const char pathSeparator = '\\';
    const unsigned int changeBufferSize = 16384u;
#else
    const char pathSeparator = '/';
#endif
}


File::File(const char* filename, bool isRawPath, const std::function<void ()>& fileChangedCallback)
    : _fileChangedCallback(fileChangedCallback)
#ifdef _WIN32
    , _activeBuffer(0)
#endif
{
    if (isRawPath)
        _filename = string(filename);
    else
        _filename = FileSys.absolutePath(string(filename));

    //if (_fileChangedCallback)
        installFileChangeListener();
}

File::File(const string& filename, bool isRawPath, const function<void ()>& fileChangedCallback)
    : _fileChangedCallback(fileChangedCallback)
#ifdef _WIN32
    , _activeBuffer(0)
#endif
{
    if (isRawPath)
        _filename = filename;
    else
        _filename = FileSys.absolutePath(filename);

    if (_fileChangedCallback) {
        installFileChangeListener();
    }
}

void File::setCallback(const function<void ()> callback) {
    if (_fileChangedCallback)
        removeFileChangeListener();
    _fileChangedCallback = callback;
    if (_fileChangedCallback)
        installFileChangeListener();
}

const function<void ()>& File::callback() const {
    return _fileChangedCallback;
}

File::operator const string&() const {
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
    const string& fileName = filename();
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

void File::installFileChangeListener() {
#ifdef _WIN32
    const string& directory = directoryName();

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
        LERROR_SAFE("Directory handle for file '" << _filename << "' could not be obtained");
        return;
    }

    beginRead();
#else

#endif
}

void File::removeFileChangeListener() {
#ifdef _WIN32
    CancelIo(_directoryHandle);
    CloseHandle(_directoryHandle);
    _directoryHandle = nullptr;
#else

#endif
}

#ifdef _WIN32
void CALLBACK File::completionHandler(DWORD dwErrorCode, DWORD /*dwNumberOfBytesTransferred*/, LPOVERLAPPED lpOverlapped) {
    File* file = static_cast<File*>(lpOverlapped->hEvent);

    unsigned char currentBuffer = file->_activeBuffer;

    // Change active buffer
    file->_activeBuffer = (file->_activeBuffer + 1) % 2;
    // Restart change listener as soon as possible
    file->beginRead();

    const string thisFilename = file->filename();

    char* buffer = (char*)(&(file->_changeBuffer[currentBuffer][0]));
    while (true) {
        FILE_NOTIFY_INFORMATION& information = (FILE_NOTIFY_INFORMATION&)*buffer;
        char* currentFilenameBuffer = new char[information.FileNameLength];
        std::wcstombs(currentFilenameBuffer, information.FileName, information.FileNameLength);
        const string& currentFilename(currentFilenameBuffer);
        delete[] currentFilenameBuffer;

        if (currentFilename == thisFilename) {
            file->_fileChangedCallback();
            break;
        }
        else {
            if (!information.NextEntryOffset)
                break;
            else
                buffer += information.NextEntryOffset;
        }
    }
}

void File::beginRead() {
    ZeroMemory(&_overlappedBuffer, sizeof(OVERLAPPED));
    _overlappedBuffer.hEvent = this;

    _changeBuffer[_activeBuffer].resize(changeBufferSize);

    DWORD returnedBytes;
    BOOL success = ReadDirectoryChangesW(
        _directoryHandle,
        &_changeBuffer[_activeBuffer][0],
        _changeBuffer[_activeBuffer].size(),
        false,
        FILE_NOTIFY_CHANGE_LAST_WRITE,
        &returnedBytes,
        &_overlappedBuffer,
        &completionHandler);
}

#endif

} // namespace filesystem
} // namespace ghoul
