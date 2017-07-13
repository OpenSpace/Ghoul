/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2017                                                               *
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

#include <ghoul/filesystem/directory.h>

#include <ghoul/filesystem/filesystem.h>

#include <algorithm>
#include <stack>

#ifdef WIN32
#include <windows.h>
#include <tchar.h>
#include <direct.h>
#else
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#endif

using std::string;
using std::vector;

namespace ghoul {
namespace filesystem {

Directory::Directory() : _directoryPath(FileSys.absolutePath(".")) {}

Directory::Directory(string path, RawPath isRawPath) {
    if (isRawPath) {
        _directoryPath = path.empty() ? "." : std::move(path);
    }
    else {
        _directoryPath = FileSys.absolutePath(path.empty() ? "." : path);
    }
}

Directory::Directory(const char* path, RawPath isRawPath)
    : Directory(string(path), isRawPath)
{}

Directory::operator const string&() const {
    return _directoryPath;
}

const string& Directory::path() const {
    return _directoryPath;
}

Directory Directory::parentDirectory(AbsolutePath absolutePath) const {
#ifdef WIN32
    if (_directoryPath.back() == FileSystem::PathSeparator) {
        return Directory(
            _directoryPath + "..",
            absolutePath == AbsolutePath::Yes ? RawPath::No : RawPath::Yes
        );
    } 
    else {
        return Directory(
            _directoryPath + FileSystem::PathSeparator + "..",
            absolutePath == AbsolutePath::Yes ? RawPath::No : RawPath::Yes
        );
    }
#else
    (void)absolutePath; // remove unused argument warning ---abock
    size_t length = _directoryPath.length();
    size_t position = _directoryPath.find_last_of(FileSystem::PathSeparator);
    if(position == length && length > 1)
        position = _directoryPath.find_last_of(FileSystem::PathSeparator, length-1);
    
    return Directory(_directoryPath.substr(0, position));
#endif
}

vector<string> Directory::read(Recursive search, Sort sort) const {
    vector<string> result;
    readDirectories(result, _directoryPath, search);
    readFiles(result, _directoryPath, search);
    if (sort == Sort::Yes) {
        std::sort(result.begin(), result.end());
    }
    return result;
}

vector<string> Directory::readFiles(Recursive search, Sort sort) const{
    vector<string> result;
    readFiles(result, _directoryPath, search);
    if (sort) {
        std::sort(result.begin(), result.end());
    }
    return result;
}

void Directory::readFiles(vector<string>& result, const string& path,
                          Recursive recursiveSearch) const
{
    std::stack<string> directories;
#ifdef WIN32
    WIN32_FIND_DATA findFileData = {0};
    const string& directory = path + "\\*";

    HANDLE findHandle = FindFirstFile(directory.c_str(), &findFileData);
    if (findHandle != INVALID_HANDLE_VALUE) {
        do {
            string file(findFileData.cFileName);
            const DWORD isDir = findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
            if (!isDir) {
                result.push_back(path + "\\" + file);
            }
            if (recursiveSearch && isDir && file != "." && file != "..") {
                directories.push(path + "\\" + file);
            }
        } while (FindNextFile(findHandle, &findFileData) != 0);
    }
    FindClose(findHandle);
#else
    DIR* dir = opendir(path.c_str());
    struct dirent* ent;
    if (dir) {
        string name;
        while ((ent = readdir(dir))) {
            name = ent->d_name;
            if ((name != ".") && (name != "..")) {
                if (ent->d_type != DT_DIR) {
                    result.push_back(path + "/" + ent->d_name);
                }
                if (recursiveSearch && (ent->d_type == DT_DIR)) {
                    directories.push(path + "/" + ent->d_name);
                }
            }
        }
        closedir(dir);
    }
#endif
    while (!directories.empty()) {
        const string& d = directories.top();
        readFiles(result, d, recursiveSearch);
        directories.pop();
    }
}

vector<string> Directory::readDirectories(Recursive search, Sort sort) const {
    std::vector<std::string> result;
    readDirectories(result, _directoryPath, search);
    if (sort) {
        std::sort(result.begin(), result.end());
    }
    return result;
}

void Directory::readDirectories(vector<string>& result, const string& path,
                                Recursive recursiveSearch) const
{
    std::stack<string> directories;

#ifdef WIN32
    WIN32_FIND_DATA findFileData = {0};
    std::string directory = path + "\\*";

    HANDLE findHandle = FindFirstFile(directory.c_str(), &findFileData);
    if (findHandle != INVALID_HANDLE_VALUE) {
        do {
            string file(findFileData.cFileName);
            const DWORD isDir = findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
            if (isDir && (file != ".") && (file != "..")) {
                result.push_back(path + "\\" + file);
                if (recursiveSearch) {
                    directories.push(path + "\\" + file);
                }
            }
        } while (FindNextFile(findHandle, &findFileData) != 0);
    }
    FindClose(findHandle);
#else
    DIR* dir = opendir(path.c_str());
    struct dirent* ent;
    if (dir) {
        string name;
        while ((ent = readdir(dir))) {
            name = ent->d_name;
            if ((ent->d_type == DT_DIR) && (name != ".") && (name != "..")) {
                result.push_back(path + "/" + ent->d_name);
                if (recursiveSearch) {
                    directories.push(path + "/" + ent->d_name);
                }
            }
        }
        closedir(dir);
    }
#endif
    while (!directories.empty()) {
        const string& d = directories.top();
        readDirectories(result, d, recursiveSearch);
        directories.pop();
    }
}

std::ostream& operator<<(std::ostream& os, const Directory& d) {
    return os << d.path();
}

} // namespace filesystem
} // namespace ghoul
