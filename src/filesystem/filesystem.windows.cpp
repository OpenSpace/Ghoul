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

#if defined(WIN32)
#include <ghoul/filesystem/filesystem.h>

#include <ghoul/filesystem/cachemanager.h>
#include <ghoul/logging/logmanager.h>

#include <algorithm>
#include <cassert>
#include <regex>
#include <cstdio>

#include <direct.h>
#include <windows.h>
#include <Shlobj.h>

using std::string;

namespace {
    const string _loggerCat = "FileSystem";

	void CALLBACK completionHandler(
		DWORD dwErrorCode,
		DWORD dwNumberOfBytesTransferred,
		LPOVERLAPPED lpOverlapped);

	const unsigned int changeBufferSize = 16384u;

#define _CRT_SECURE_NO_WARNINGS
}

namespace ghoul {
namespace filesystem {
	struct DirectoryHandle {
		HANDLE _handle;
		unsigned char _activeBuffer;
		std::vector<BYTE> _changeBuffer[2];
		OVERLAPPED _overlappedBuffer;
	};

void FileSystem::deinitializeInternalWindows() {
	for (auto d : _fileSystem->_directories) {
		DirectoryHandle* dh = d.second;
		CancelIo(dh->_handle);
		CloseHandle(dh->_handle);
		delete dh;
	}
}

void FileSystem::addFileListener(File* file) {
	assert(file != nullptr);
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
	for (auto it = eqRange.first; it != eqRange.second; ++it) {
		if (it->second == file) {
			LERROR("Already tracking fileobject");
			return;
		}
	}
#endif
	_trackedFiles.emplace(file->path(), file);
}

void FileSystem::removeFileListener(File* file) {
	assert(file != nullptr);
	auto eqRange = _trackedFiles.equal_range(file->path());
	for (auto it = eqRange.first; it != eqRange.second; ++it) {
		if (it->second == file) {
			_trackedFiles.erase(it);
			return;
		}
	}
    LWARNING("Could not find tracked '" << file <<"' for path '"<< file->path() << "'");
}

void FileSystem::callbackHandler(DirectoryHandle* directoryHandle, const std::string& file) {
	std::string fullPath;
	for (auto d : FileSys._directories) {
		if (d.second == directoryHandle)
			fullPath = d.first + PathSeparator + file;
	}

	size_t n = FileSys._trackedFiles.count(fullPath);
	if (n > 0) {
		auto eqRange = FileSys._trackedFiles.equal_range(fullPath);
		for (auto it = eqRange.first; it != eqRange.second; ++it) {
			File* f = (*it).second;
			f->_fileChangedCallback(*f);
		}
	}
}

void callbackHandler(DirectoryHandle* directoryHandle, const std::string& file) {
	FileSys.callbackHandler(directoryHandle, file);
}

void readStarter(DirectoryHandle* directoryHandle) {
	FileSys.beginRead(directoryHandle);
}
void CALLBACK completionHandler(
	DWORD /*dwErrorCode*/, 
	DWORD /*dwNumberOfBytesTransferred*/,
	LPOVERLAPPED lpOverlapped)
{
	DirectoryHandle* directoryHandle = static_cast<DirectoryHandle*>(lpOverlapped->hEvent);

	unsigned char currentBuffer = directoryHandle->_activeBuffer;

	// Change active buffer (ping-pong buffering)
	directoryHandle->_activeBuffer = (directoryHandle->_activeBuffer + 1) % 2;

	// Restart change listener as soon as possible
	readStarter(directoryHandle);

	char* buffer = reinterpret_cast<char*>(&(directoryHandle->_changeBuffer[currentBuffer][0]));

	// data might have queued up, so we need to check all changes
	while (true) {
		// extract the information which file has changed
		FILE_NOTIFY_INFORMATION& information = 
			reinterpret_cast<FILE_NOTIFY_INFORMATION&>(*buffer);
		
		if (information.Action == FILE_ACTION_MODIFIED) {

			char* currentFilenameBuffer = new char[information.FileNameLength];

			// Convert from DWORD to char*
			size_t i;
			wcstombs_s(&i, currentFilenameBuffer, information.FileNameLength,
				information.FileName, information.FileNameLength);
			if (i > 0) {
				// make sure the last char is string terminating
				currentFilenameBuffer[i - 1] = '\0';
				const string currentFilename(currentFilenameBuffer, i - 1);

				//switch (information.Action) {
				//case FILE_ACTION_ADDED:
				//	LDEBUG("Action:                 FILE_ACTION_ADDED");
				//	break;
				//case FILE_ACTION_REMOVED:
				//	LDEBUG("Action:                 FILE_ACTION_REMOVED");
				//	break;
				//case FILE_ACTION_MODIFIED:
				//	LDEBUG("Action:                 FILE_ACTION_MODIFIED");
				//	break;
				//case FILE_ACTION_RENAMED_OLD_NAME:
				//	LDEBUG("Action:                 FILE_ACTION_RENAMED_OLD_NAME");
				//	break;
				//case FILE_ACTION_RENAMED_NEW_NAME:
				//	LDEBUG("Action:                 FILE_ACTION_RENAMED_NEW_NAME");
				//	break;
				//default:
				//	LDEBUG("Action:                 UNKNOWN");
				//	break;
				//}
				//LDEBUG("FileNameLength:         " << information.FileNameLength);
				//LDEBUG("file:                   " << currentFilename);
				//LDEBUG("currentFilenamelength:  " << currentFilename.length());
				//LDEBUG("NextEntryOffset:        " << information.NextEntryOffset);

				callbackHandler(directoryHandle, currentFilename);
			}
			delete[] currentFilenameBuffer;
		}
		if (!information.NextEntryOffset)
			// we are done with all entries and didn't find our file
			break;
		else
			//continue with the next entry
			buffer += information.NextEntryOffset;
	}
	//LWARNING("================");
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
		FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE | 
		FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_CREATION,
		&returnedBytes,
		overlappedBuffer,
		&completionHandler);

	if (success == 0) {
		LERROR("Could not begin read directory");
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

} // namespace filesystem
} // namespace ghoul

#endif
