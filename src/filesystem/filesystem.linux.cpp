/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2019                                                               *
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

#if !defined(WIN32) && !defined(__APPLE__)

#include <ghoul/filesystem/filesystem.h>

#include <ghoul/fmt.h>
#include <ghoul/filesystem/cachemanager.h>
#include <ghoul/filesystem/file.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/assert.h>
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <dirent.h>
#include <pwd.h>
#include <regex>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

using std::string;

namespace {
    constexpr const char* _loggerCat = "FileSystem";
    const uint32_t mask = IN_ALL_EVENTS | IN_IGNORED | IN_Q_OVERFLOW |
                          IN_UNMOUNT | IN_ISDIR;
} // namespace

namespace ghoul::filesystem {

void FileSystem::initializeInternalLinux() {
    _inotifyHandle = inotify_init();
    _keepGoing = true;
    _t = std::thread(inotifyWatcher);
}

void FileSystem::deinitializeInternalLinux() {
    _keepGoing = false;
    if (_t.joinable()) {
        _t.join();
    }
    close(_inotifyHandle);
}

void FileSystem::addFileListener(File* file) {
    ghoul_assert(file != nullptr, "File cannot be nullptr");
    const std::string filename = file->path();
    int wd = inotify_add_watch(_inotifyHandle, filename.c_str(), mask);
    auto eqRange = _trackedFiles.equal_range(wd);
    for (auto it = eqRange.first; it != eqRange.second; ++it) {
        if (it->second == file) {
            LERROR("Already tracking fileobject");
            return;
        }
    }
    _trackedFiles.emplace(wd, file);
}

void FileSystem::removeFileListener(File* file) {
    ghoul_assert(file != nullptr, "File cannot be nullptr");

    const std::string filename = file->path();
    int wd = inotify_add_watch(_inotifyHandle, filename.c_str(), mask);
    auto eqRange = _trackedFiles.equal_range(wd);
    for (auto it = eqRange.first; it != eqRange.second; ++it) {
        if (it->second == file) {
            _trackedFiles.erase(it);
            return;
        }
    }
    LWARNING(fmt::format(
        "Could not find tracked '{}' for path '{}'",
        reinterpret_cast<void*>(file), file->path()
    ));
}

void FileSystem::inotifyWatcher() {
    int fd = FileSys._inotifyHandle;
    char buffer[BUF_LEN];
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    fd_set rfds;
    while (FileSys._keepGoing) {
        FD_ZERO (&rfds);
        FD_SET (fd, &rfds);
        if (select(FD_SETSIZE, &rfds, nullptr, nullptr, &tv) < 1) {
            continue;
        }

        ssize_t length = read(fd, buffer, BUF_LEN );
        if (length < 0) {
            continue;
        }

        long unsigned int offset = 0;
        while (offset < static_cast<long unsigned int>(length)) {
            struct inotify_event* e = reinterpret_cast<inotify_event*>(buffer + offset);
            switch (e->mask) {
                case IN_MODIFY:
                case IN_ATTRIB:
                {
                    int wd = e->wd;
                    auto eqRange = FileSys._trackedFiles.equal_range(wd);
                    for (auto it = eqRange.first; it != eqRange.second; ++it) {
                        File* fileobject = it->second;
                        fileobject->callback()(*fileobject);
                    }
                    break;
                }
                case IN_IGNORED:
                {
                    int wd = e->wd;

                    auto eqRange = FileSys._trackedFiles.equal_range(wd);
                    auto it = eqRange.first;

                    // remove tracking of the removed descriptor
                    inotify_rm_watch(fd, wd);

                    // if there are files tracking
                    if (it != eqRange.second) {
                        // add new tracking
                        int new_wd = inotify_add_watch(
                            fd,
                            it->second->path().c_str(),
                            mask
                        );

                        // save all files
                        std::vector<File*> v;
                        for (; it != eqRange.second; ++it) {
                            v.push_back(it->second);
                        }

                        // erase all previous files and add them again
                        FileSys._trackedFiles.erase(eqRange.first, eqRange.second);
                        for (auto f : v) {
                            FileSys._trackedFiles.emplace(new_wd, f);
                        }
                    }
                    break;
                }
                default:
                    break;
            }
            offset += EVENT_SIZE + e->len;
        }
    }
}

} // namespace ghoul::filesystem

#endif
