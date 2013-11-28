/**************************************************************************************************
 * GHOUL                                                                                          *
 * General Helpful Open Utility Library                                                           *
 *                                                                                                *
 * Copyright (c) 2012 Alexander Bock                                                              *
 *                                                                                                *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software  *
 * and associated documentation files (the "Software"), to deal in the Software without           *
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,     *
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the  *
 * Software is furnished to do so, subject to the following conditions:                           *
 *                                                                                                *
 * The above copyright notice and this permission notice shall be included in all copies or       *
 * substantial portions of the Software.                                                          *
 *                                                                                                *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING  *
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND     *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 *************************************************************************************************/

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

namespace {
#ifdef _WIN32
    const char pathSeparator = '\\';
#else
    const char pathSeparator = '/';
#endif
}

Directory::Directory() 
    : _directoryPath(".")
{}

Directory::Directory(const char* path, bool isRawPath) {
    // Move into Directory(const string&, bool) as soon as delegating constructors are supported
    if (isRawPath)
        _directoryPath = string(path);
    else
        _directoryPath = FileSys.absolutePath(string(path));
}

Directory::Directory(const string& path, bool isRawPath) {
    if (isRawPath)
        _directoryPath = path.empty() ? "." : path;
    else
        _directoryPath = FileSys.absolutePath(path.empty() ? "." : path);
}

Directory::operator const string&() const {
    return _directoryPath;
}

const std::string& Directory::path() const {
    return _directoryPath;
}

Directory Directory::parentDirectory() const {
    if (_directoryPath.back() == pathSeparator) {
        string::size_type separator = _directoryPath.rfind(pathSeparator, _directoryPath.back());
        return Directory(_directoryPath.substr(0, separator));
    }
    else {
        string::size_type separator = _directoryPath.rfind(pathSeparator);
        return Directory(_directoryPath.substr(0, separator));
    }
}

vector<string> Directory::read(bool recursiveSearch, bool sort) const {
    vector<string> result;
    readDirectories(result, _directoryPath, recursiveSearch);
    readFiles(result, _directoryPath, recursiveSearch);

    if (sort)
        std::sort(result.begin(), result.end());

    return result;
}

vector<string> Directory::readFiles(bool recursiveSearch, bool sort) const {
    vector<string> result;
    readFiles(result, _directoryPath, recursiveSearch);
    if (sort)
        std::sort(result.begin(), result.end());
    return result;
}

void Directory::readFiles(vector<string>& result, const string& path, bool recursiveSearch) const {
    std::stack<string> directories;
#ifdef WIN32
    WIN32_FIND_DATA findFileData = {0};
    const string& directory = path + "\\*";

    HANDLE findHandle = FindFirstFile(directory.c_str(), &findFileData);
    if (findHandle != INVALID_HANDLE_VALUE) {
        do {
            string file(findFileData.cFileName);
            const DWORD isDirectory = findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
            if (!isDirectory)
                result.push_back(path + "/" + file);
            if (recursiveSearch && isDirectory && file != "." && file != "..")
                directories.push(path + "/" + file);
        } while (FindNextFile(findHandle, &findFileData) != 0);
    }
    FindClose(findHandle);
#else
    DIR* dir = opendir(path.c_str());
    struct dirent* ent;
    if (dir != NULL) {
        string name;
        while ((ent = readdir(dir))) {
            name = ent->d_name;
            if ((name != ".") && (name != "..")) {
                if (ent->d_type != DT_DIR) 
                    result.push_back(path + "/" + ent->d_name);
                if (recursiveSearch && (ent->d_type == DT_DIR))
                    directories.push(path + "/" + ent->d_name);
            }
        }
    }
    closedir(dir);
#endif
    while (!directories.empty()) {
        const string& directory = directories.top();
        readFiles(result, directory, recursiveSearch);
        directories.pop();
    }
}

vector<string> Directory::readDirectories(bool recursiveSearch, bool sort) const {
    vector<string> result;
    readDirectories(result, _directoryPath, recursiveSearch);
    if (sort)
        std::sort(result.begin(), result.end());
    return result;
}

void Directory::readDirectories(
    vector<string>& result,
    const string& path,
    bool recursiveSearch) const
{
    std::stack<string> directories;

#ifdef WIN32
    WIN32_FIND_DATA findFileData = {0};
    std::string directory = path + "\\*";

    HANDLE findHandle = FindFirstFile(directory.c_str(), &findFileData);
    if (findHandle != INVALID_HANDLE_VALUE) {
        do {
            string file(findFileData.cFileName);
            const DWORD isDirectory = findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
            if (isDirectory && (file != ".") && (file != "..")) {
                result.push_back(path + "\\" + file);
                if (recursiveSearch)
                    directories.push(path + "\\" + file);
            }
        } while (FindNextFile(findHandle, &findFileData) != 0);
    }
    FindClose(findHandle);
#else
    DIR* dir = opendir(path.c_str());
    struct dirent* ent;
    if (dir != NULL) {
        string name;
        while ((ent = readdir(dir))) {
            name = ent->d_name;
            if ((ent->d_type == DT_DIR) && (name != ".") && (name != "..")) {
                result.push_back(path + "/" + ent->d_name);
                if (recursiveSearch)
                    directories.push(path + "/" + ent->d_name);
            }
        }
    }
    closedir(dir);
#endif
    while (!directories.empty()) {
        const string& directory = directories.top();
        readDirectories(result, directory, recursiveSearch);
        directories.pop();
    }
}

} // filesystem
} // ghoul



