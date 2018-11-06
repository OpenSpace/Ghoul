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

#ifdef __APPLE__

#include <ghoul/filesystem/filesystem.h>

#include <ghoul/filesystem/file.h>
#include <ghoul/misc/assert.h>
#include <CoreServices/CoreServices.h>

namespace {
    // the maximum latency allowed before a changed is registered
    constexpr const CFAbsoluteTime Latency = 1.0;

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

    void completionHandler(ConstFSEventStreamRef, void*, size_t numEvents,
                           void* eventPaths, const FSEventStreamEventFlags eventFlags[],
                           const FSEventStreamEventId[])
    {
        char** paths = reinterpret_cast<char**>(eventPaths);
        for (size_t i = 0; i < numEvents; ++i) {
            using Events::kFSEventStreamEventFlagItemModified;
            using Events::kFSEventStreamEventFlagItemInodeMetaMod;
            using Events::kFSEventStreamEventFlagItemIsFile;

            const bool mod = eventFlags[i] & kFSEventStreamEventFlagItemModified;
            const bool iNode = eventFlags[i] & kFSEventStreamEventFlagItemInodeMetaMod;
            const bool file = eventFlags[i] & kFSEventStreamEventFlagItemIsFile;

            if ((mod || iNode) && file) {
                ghoul::filesystem::callbackHandler(paths[i]);
            }
        }
    }
} // namespace

namespace ghoul::filesystem {

struct DirectoryHandle {
    FSEventStreamRef _eventStream;
};

void FileSystem::deinitializeInternalApple() {
    for (const auto& d : _directories) {
        DirectoryHandle* dh = d.second;
        FSEventStreamStop(dh->_eventStream);
        FSEventStreamInvalidate(dh->_eventStream);
        FSEventStreamRelease(dh->_eventStream);
        delete dh;
    }
}

void FileSystem::addFileListener(File* file) {
    ghoul_assert(file, "File must not a nullptr");
#ifdef GHL_DEBUG
    auto eqRange = _trackedFiles.equal_range(file->path());
    for (auto it = eqRange.first; it != eqRange.second; ++it) {
        ghoul_assert(it->second != file, "File already registered");
    }
#endif

    std::string d = file->directoryName();
    auto f = _directories.find(d);
    if (f == _directories.end()) {
        bool alreadyTrackingParent = false;
        for (const std::pair<std::string, DirectoryHandle*>& dir : _directories) {
            if (d.length() > dir.first.length() &&
                d.find(dir.first) != std::string::npos)
            {
                alreadyTrackingParent = true;
                break;
            }
        }
        if (!alreadyTrackingParent) {
            DirectoryHandle* handle = new DirectoryHandle;

            // Create the FSEventStream responsible for this directory (Apple's callback
            // system only works on the granularity of the directory)
            CFStringRef path = CFStringCreateWithCString(
                NULL,
                d.c_str(),
                kCFStringEncodingASCII
            );
            CFArrayRef pathsToWatch = CFArrayCreate(
                NULL,
                reinterpret_cast<const void **>(&path),
                1,
                NULL
            );
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
                Latency,
                kFSEventStreamCreateFlagFileEvents
            );

            // Add checking the event stream to the current run loop
            // If there is a performance bottleneck, this could be done on a separate
            // thread?
            FSEventStreamScheduleWithRunLoop(handle->_eventStream,
                CFRunLoopGetCurrent(),
                kCFRunLoopDefaultMode
            );
            // Start monitoring
            FSEventStreamStart(handle->_eventStream);
            _directories[d] = handle;
        }
    }

    _trackedFiles.insert({ file->path(), file });
}

void FileSystem::removeFileListener(File* file) {
    ghoul_assert(file, "File must not be nullptr");

    auto eqRange = _trackedFiles.equal_range(file->path());

    bool found = false;
    for (auto it = eqRange.first; it != eqRange.second; ++it) {
        found |= (it->second == file);
    }
    ghoul_assert(found, "File not previously registered");

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
    if (n == 0) {
        return;
    }

    auto eqRange = FileSys._trackedFiles.equal_range(path);
    for (auto it = eqRange.first; it != eqRange.second; ++it) {
        File* f = (*it).second;
        f->callback()(*f);
    }
}

void FileSystem::triggerFilesystemEventsInternalApple() {}

} // namespace ghoul::filesystem

#endif
