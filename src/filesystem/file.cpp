/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2024                                                               *
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

#include <ghoul/filesystem/file.h>

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/format.h>
#include <ghoul/misc/assert.h>
#include <filesystem>

#ifdef WIN32
#include <Windows.h>
#else // ^^^^ WIN32 // !WIN32 vvvv
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#endif // WIN32

#ifdef WIN32
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // _CRT_SECURE_NO_WARNINGS
#endif // WIN32

namespace ghoul::filesystem {

File::File(std::filesystem::path filename) {
    ghoul_assert(!filename.empty(), "Filename must not be empty");
    _filename = std::move(filename);
}

File::File(const File& file)
    : _filename(file._filename)
    , _fileChangedCallback(file._fileChangedCallback)
{
    if (_fileChangedCallback) {
        installFileChangeListener();
    }
}

File::~File() {
    if (_fileChangedCallback) {
        removeFileChangeListener();
    }
}

void File::setCallback(FileChangedCallback callback) {
    if (_fileChangedCallback) {
        removeFileChangeListener();
    }
    _fileChangedCallback = std::move(callback);
    if (_fileChangedCallback) {
        installFileChangeListener();
    }
}

const std::filesystem::path& File::path() const {
    return _filename;
}

void File::installFileChangeListener() {
    _indx = FileSys.addFileListener(path(), _fileChangedCallback);
}

void File::removeFileChangeListener() const {
    FileSys.removeFileListener(_indx);
}

} // namespace ghoul::filesystem
