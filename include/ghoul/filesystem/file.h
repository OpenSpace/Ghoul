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

#ifndef __FILE_H__
#define __FILE_H__

#include <functional>
#include <string>

#include <ghoul/filesystem/filesystem.h>

#ifdef _WIN32
#include <windows.h>
#endif

namespace ghoul {
namespace filesystem {

class File {
public:
    File(const char* filename, bool isRawPath = false, const std::function<void ()>& fileChangedCallback = [](){});
    File(const std::string& filename, bool isRawPath = false, const std::function<void ()>& fileChangedCallback = [](){});

    void setCallback(const std::function<void ()> callback);
    const std::function<void ()>& callback() const;

    operator const std::string&() const;

    std::string filename() const;
    std::string baseName() const;
    std::string fullBaseName() const;
    std::string directoryName() const;
    std::string fileExtension() const;

private:
    void installFileChangeListener();
    void removeFileChangeListener();

    std::string _filename;
    std::function<void ()> _fileChangedCallback;

#ifdef _WIN32
    void beginRead();
    static void CALLBACK completionHandler(
        DWORD dwErrorCode,							// completion code
        DWORD dwNumberOfBytesTransferred,			// number of bytes transferred
        LPOVERLAPPED lpOverlapped);		

    HANDLE _directoryHandle;
    unsigned char _activeBuffer;
    std::vector<BYTE> _changeBuffer[2];
    OVERLAPPED _overlappedBuffer;

#endif
};

} // namespace filesystem
} // namespace ghoul

#endif // __FILE_H__