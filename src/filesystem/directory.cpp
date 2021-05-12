/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2021                                                               *
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
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX

#include <direct.h>
#include <tchar.h>
#include <windows.h>
#else
#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#endif

using std::string;
using std::vector;

namespace ghoul::filesystem {

Directory::Directory() : _directoryPath(FileSys.absolutePath(".")) {}

Directory::Directory(string path) {
    _directoryPath = FileSys.absolutePath(path.empty() ? "." : path);
}

Directory::Directory(const char* path)
    : Directory(string(path))
{}

const string& Directory::path() const {
    return _directoryPath;
}

vector<string> Directory::read(Recursive recursiveSearch, Sort sort) const {
    vector<string> result;
    internalReadDirectories(result, _directoryPath, recursiveSearch);
    internalReadFiles(result, _directoryPath, recursiveSearch);
    if (sort) {
        std::sort(result.begin(), result.end());
    }
    return result;
}

vector<string> Directory::readFiles(Recursive recursiveSearch, Sort sort) const {
    vector<string> result;
    internalReadFiles(result, _directoryPath, recursiveSearch);
    if (sort) {
        std::sort(result.begin(), result.end());
    }
    return result;
}

void Directory::internalReadFiles(vector<string>& result, const string& path,
                                  Recursive recursiveSearch) const
{
    std::stack<string> directories;
#ifdef WIN32
    WIN32_FIND_DATA findFileData = { 0 };
    string directory = path + "\\*";

    HANDLE findHandle = FindFirstFile(directory.c_str(), &findFileData);
    if (findHandle != INVALID_HANDLE_VALUE) { // NOLINT
        do {
            string file(findFileData.cFileName);
            const DWORD isDir = findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
            if (!isDir) {
                result.push_back(path + "\\" + file);
            }
            if (recursiveSearch && isDir && (file != ".") && (file != "..")) {
                directories.push(path + "\\" + file);
            }
        } while (FindNextFile(findHandle, &findFileData) != 0);
    }
    FindClose(findHandle);
#else // ^^^^ WIN32 // !WIN32 vvvv
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
#endif // WIN32
    while (!directories.empty()) {
        const string& d = directories.top();
        internalReadFiles(result, d, recursiveSearch);
        directories.pop();
    }
}

vector<string> Directory::readDirectories(Recursive recursiveSearch, Sort sort) const {
    std::vector<std::string> result;
    internalReadDirectories(result, _directoryPath, recursiveSearch);
    if (sort) {
        std::sort(result.begin(), result.end());
    }
    return result;
}

void Directory::internalReadDirectories(vector<string>& result, const string& path,
                                        Recursive recursiveSearch) const
{
    std::stack<string> directories;

#ifdef WIN32
    WIN32_FIND_DATA findFileData = {0};
    std::string directory = path + "\\*";

    HANDLE findHandle = FindFirstFile(directory.c_str(), &findFileData);
    if (findHandle != INVALID_HANDLE_VALUE) { // NOLINT
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
#else // ^^^^ WIN32 // !WIN32 vvvv
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
#endif // WIN32
    while (!directories.empty()) {
        const string& d = directories.top();
        internalReadDirectories(result, d, recursiveSearch);
        directories.pop();
    }
}

} // namespace ghoul::filesystem
