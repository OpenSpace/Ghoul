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

#if !defined(WIN32) && !defined(__APPLE__)
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

#include <sys/inotify.h>
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

using std::string;

namespace {
    const string _loggerCat = "FileSystem";
	const uint32_t mask = IN_ALL_EVENTS | IN_IGNORED | IN_Q_OVERFLOW | IN_UNMOUNT | IN_ISDIR;
}

namespace ghoul {
namespace filesystem {

void FileSystem::initializeInternalLinux() {
    _inotifyHandle = inotify_init();
    _keepGoing = true;
    _t = std::thread(inotifyWatcher);
}

void FileSystem::deinitializeInternalLinux() {
	_keepGoing = false;
	if (_t.joinable())
		_t.join();
	close(_inotifyHandle);
}

void FileSystem::addFileListener(File* file) {
	assert(file != nullptr);
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
	assert(file != nullptr);

    const std::string filename = file->path();
    int wd = inotify_add_watch(_inotifyHandle, filename.c_str(), mask);
    int count = _trackedFiles.count(wd);
    auto eqRange = _trackedFiles.equal_range(wd);
    for (auto it = eqRange.first; it != eqRange.second; ++it) {
        if (it->second == file) {
            if(count == 1)
                ( void ) inotify_rm_watch( _inotifyHandle, it->first );
            _trackedFiles.erase(it);
            return;
        }
    }
    LWARNING("Could not find tracked '" << file <<"' for path '"<< file->path() << "'");
}

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
                    auto eqRange = FileSys._trackedFiles.equal_range(wd);
                    for (auto it = eqRange.first; it != eqRange.second; ++it) {
                        File* fileobject = it->second;
                        fileobject->_fileChangedCallback(*fileobject);
                    }
                    
                }

                    break;

                case IN_IGNORED:
                {
                    int wd = event->wd;

                    auto eqRange = FileSys._trackedFiles.equal_range(wd);
                    auto it = eqRange.first;

                    // remove tracking of the removed descriptor
                    ( void ) inotify_rm_watch( fd, wd );

                    // if there are files tracking
                    if(it != eqRange.second) {
                        // add new tracking
                        int new_wd = inotify_add_watch( fd, it->second->path().c_str(), mask);

                        // save all files
                        std::vector<File*> v;
                        for (;it != eqRange.second; ++it) {
                            v.push_back(it->second);
                        }

                        // erase all previous files and add them again
                        FileSys._trackedFiles.erase(eqRange.first, eqRange.second);
                        for(auto f: v) {
                            FileSys._trackedFiles.emplace(new_wd, f);
                        }
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

} // namespace filesystem
} // namespace ghoul

#endif




