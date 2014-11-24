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

#include <CoreServices/CoreServices.h>
#include <sys/stat.h>

using std::string;

namespace {
    const string _loggerCat = "FileSystem";
	// the maximum latency allowed before a changed is registered
	const CFAbsoluteTime latency = 1.0;
    
    enum Events {
        kFSEventStreamEventFlagNone = 0x00000000,
        kFSEventStreamEventFlagMustScanSubDirs = 0x00000001,
        kFSEventStreamEventFlagUserDropped = 0x00000002,
        kFSEventStreamEventFlagKernelDropped = 0x00000004,
        kFSEventStreamEventFlagEventIdsWrapped = 0x00000008,
        kFSEventStreamEventFlagHistoryDone = 0x00000010,
        kFSEventStreamEventFlagRootChanged = 0x00000020,
        kFSEventStreamEventFlagMount = 0x00000040,
        kFSEventStreamEventFlagUnmount = 0x00000080,
        // These flags are only set if you specified the FileEvents
        // flags when creating the stream.
        kFSEventStreamEventFlagItemCreated = 0x00000100,
        kFSEventStreamEventFlagItemRemoved = 0x00000200,
        kFSEventStreamEventFlagItemInodeMetaMod = 0x00000400,
        kFSEventStreamEventFlagItemRenamed = 0x00000800,
        kFSEventStreamEventFlagItemModified = 0x00001000,
        kFSEventStreamEventFlagItemFinderInfoMod = 0x00002000,
        kFSEventStreamEventFlagItemChangeOwner = 0x00004000,
        kFSEventStreamEventFlagItemXattrMod = 0x00008000,
        kFSEventStreamEventFlagItemIsFile = 0x00010000,
        kFSEventStreamEventFlagItemIsDir = 0x00020000,
        kFSEventStreamEventFlagItemIsSymlink = 0x00040000
    };
    
    void completionHandler(
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
            
            ghoul::filesystem::callbackHandler(paths[i]);
        }
    }
    
    std::string EventEnumToName(Events e) {
        std::string name;
        if(e & Events::kFSEventStreamEventFlagMustScanSubDirs)
            name += "| kFSEventStreamEventFlagMustScanSubDirs";
        if(e & Events::kFSEventStreamEventFlagUserDropped)
            name += "| kFSEventStreamEventFlagUserDropped";
        if(e & Events::kFSEventStreamEventFlagKernelDropped)
            name += "| kFSEventStreamEventFlagKernelDropped";
        if(e & Events::kFSEventStreamEventFlagEventIdsWrapped)
            name += "| kFSEventStreamEventFlagEventIdsWrapped";
        if(e & Events::kFSEventStreamEventFlagHistoryDone)
            name += "| kFSEventStreamEventFlagHistoryDone";
        if(e & Events::kFSEventStreamEventFlagRootChanged)
            name += "| kFSEventStreamEventFlagRootChanged";
        if(e & Events::kFSEventStreamEventFlagMount)
            name += "| kFSEventStreamEventFlagMount";
        if(e & Events::kFSEventStreamEventFlagUnmount)
            name += "| kFSEventStreamEventFlagUnmount";
        
        
        if(e & Events::kFSEventStreamEventFlagItemCreated)
            name += "| kFSEventStreamEventFlagItemCreated";
        if(e & Events::kFSEventStreamEventFlagItemRemoved)
            name += "| kFSEventStreamEventFlagItemRemoved";
        if(e & Events::kFSEventStreamEventFlagItemInodeMetaMod)
            name += "| kFSEventStreamEventFlagItemInodeMetaMod";
        if(e & Events::kFSEventStreamEventFlagItemRenamed)
            name += "| kFSEventStreamEventFlagItemRenamed";
        if(e & Events::kFSEventStreamEventFlagItemModified)
            name += "| kFSEventStreamEventFlagItemModified";
        if(e & Events::kFSEventStreamEventFlagItemFinderInfoMod)
            name += "| kFSEventStreamEventFlagItemFinderInfoMod";
        if(e & Events::kFSEventStreamEventFlagItemChangeOwner)
            name += "| kFSEventStreamEventFlagItemChangeOwner";
        if(e & Events::kFSEventStreamEventFlagItemXattrMod)
            name += "| kFSEventStreamEventFlagItemXattrMod";
        if(e & Events::kFSEventStreamEventFlagItemIsFile)
            name += "| kFSEventStreamEventFlagItemIsFile";
        if(e & Events::kFSEventStreamEventFlagItemIsDir)
            name += "| kFSEventStreamEventFlagItemIsDir";
        if(e & Events::kFSEventStreamEventFlagItemIsSymlink)
            name += "| kFSEventStreamEventFlagItemIsSymlink";
        
        if (name.length() > 2) {
            name = name.substr(2);
        }
        return name;
    }
    
    
}

namespace ghoul {
namespace filesystem {

struct DirectoryHandle {
    FSEventStreamRef _eventStream;
};

void FileSystem::deinitializeInternalApple() {
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
			LWARNING("Already tracking fileobject");
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

void callbackHandler(const std::string& path) {
    FileSys.callbackHandler(path);
}

void FileSystem::callbackHandler(const std::string& path) {
    size_t n = FileSys._trackedFiles.count(path);
    if (n == 0)
        return;
    
    auto eqRange = FileSys._trackedFiles.equal_range(path);
    for (auto it = eqRange.first; it != eqRange.second; ++it) {
        File* f = (*it).second;
        f->_fileChangedCallback(*f);
    }
}

void FileSystem::triggerFilesystemEventsInternalApple() {
    for(auto d: _directories) {
        FSEventStreamFlushSync(d.second->_eventStream);
    }
}

} // namespace filesystem
} // namespace ghoul

#endif
