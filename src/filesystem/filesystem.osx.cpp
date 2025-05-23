/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2025                                                               *
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
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/assert.h>
#include <CoreServices/CoreServices.h>

namespace {
    constexpr std::string_view _loggerCat = "FileSystem";
    // the maximum latency allowed before a changed is registered
    constexpr CFAbsoluteTime Latency = 1.0;

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
        for (size_t i = 0; i < numEvents; i++) {
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

int FileSystem::FileChangeInfo::NextIdentifier = 0;

struct DirectoryHandle {
    FSEventStreamRef _eventStream;
};

void FileSystem::deinitializeInternalApple() {
    for (const std::pair<const std::string, DirectoryHandle*>& d : _directories) {
        DirectoryHandle* dh = d.second;
        if (dh) {
            FSEventStreamStop(dh->_eventStream);
            FSEventStreamInvalidate(dh->_eventStream);
            FSEventStreamRelease(dh->_eventStream);
        }
        delete dh;
    }
}

int FileSystem::addFileListener(std::filesystem::path path,
                                File::FileChangedCallback callback)
{
    std::string d = path.parent_path().string();

    const auto f = _directories.find(d);
    if (f == _directories.end()) {
        bool alreadyTrackingParent = false;
        for (const std::pair<const std::string, DirectoryHandle*>& dir : _directories) {
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
            CFStringRef p = CFStringCreateWithCString(
                nullptr,
                d.c_str(),
                kCFStringEncodingASCII
            );
            CFArrayRef pathsToWatch = CFArrayCreate(
                nullptr,
                reinterpret_cast<const void**>(&p),
                1,
                nullptr
            );
            FSEventStreamContext callbackInfo;
            callbackInfo.version = 0;
            callbackInfo.info = nullptr;
            callbackInfo.release = nullptr;
            callbackInfo.retain = nullptr;
            callbackInfo.copyDescription = nullptr;

            handle->_eventStream = FSEventStreamCreate(
                nullptr,
                &completionHandler,
                &callbackInfo,
                pathsToWatch,
                kFSEventStreamEventIdSinceNow,
                Latency,
                kFSEventStreamCreateFlagFileEvents
            );

            // Add checking the event stream to the current run loop. If there is a
            // performance bottleneck, this could be done on a separate thread?
            FSEventStreamScheduleWithRunLoop(
                handle->_eventStream,
                CFRunLoopGetCurrent(),
                kCFRunLoopDefaultMode
            );
            // Start monitoring
            FSEventStreamStart(handle->_eventStream);
            _directories[d] = handle;
        }
    }

    int idx = FileChangeInfo::NextIdentifier;
    FileChangeInfo info;
    info.identifier = idx;
    info.path = std::move(path);
    info.callback = std::move(callback);
    _trackedFiles.push_back(std::move(info));

    FileChangeInfo::NextIdentifier += 1;
    return idx;
}

void FileSystem::removeFileListener(int callbackIdentifier) {
    for (size_t i = 0; i < _trackedFiles.size(); i += 1) {
        if (_trackedFiles[i].identifier == callbackIdentifier) {
            _trackedFiles.erase(_trackedFiles.begin() + i);
            return;
        }
    }

    LWARNING(std::format("Could not find callback identifier '{}'", callbackIdentifier));
}

void callbackHandler(const std::string& path) {
    FileSys.callbackHandler(path);
}

void FileSystem::callbackHandler(const std::string& path) {
    for (const FileChangeInfo& info : FileSys._trackedFiles) {
        if (info.path == path) {
            info.callback();
        }
    }
}

} // namespace ghoul::filesystem

#endif
