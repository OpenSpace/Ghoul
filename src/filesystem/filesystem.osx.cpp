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

#if defined(__APPLE__)
#include <ghoul/filesystem/filesystem.h>

#include <ghoul/filesystem/cachemanager.h>
#include <ghoul/logging/logmanager.h>

#include <algorithm>
#include <cassert>
#include <regex>
#include <cstdio>

#include <dirent.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>

namespace {
	// the maximum latency allowed before a changed is registered
	const CFAbsoluteTime latency = 1.0;
}

namespace ghoul {
namespace filesystem {

const char FileSystem::PathSeparator = '/';

void FileSystem::appleDeinitialize() {
	for (auto d : _directories) {
		DirectoryHandle* dh = d.second;
		FSEventStreamStop(dh->_eventStream);
		FSEventStreamInvalidate(dh->_eventStream);
		FSEventStreamRelease(dh->_eventStream);
		delete dh;
	}
}

void FileSystem::addFileListener(File* file) {
	assert(file != nullptr);
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
			FSEventStreamScheduleWithRunLoop(handle->_eventStream,
				CFRunLoopGetCurrent(),
				kCFRunLoopDefaultMode);
			// Start monitoring
			FSEventStreamStart(handle->_eventStream);
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
}

void FileSystem::removeFileListener(File* file) {
	assert(file != nullptr);
	auto eqRange = _trackedFiles.equal_range(file->path());
	for (auto it = eqRange.first; it != eqRange.second; ++it) {
		//LDEBUG("comparing for removal, " << file << "==" << it->second);
		if (it->second == file) {
			//LWARNING("Removing tracking of " << file);
			_trackedFiles.erase(it);
			return;
		}
	}
}

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

} // namespace filesystem
} // namespace ghoul

#endif
