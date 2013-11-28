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

#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include <map>
#include <string>
#include <vector>

#include <ghoul/filesystem/directory.h>

namespace ghoul {
namespace filesystem {

class File;

class FileSystem {
public:
    static void initialize();
    static void deinitialize();
    static FileSystem& ref();
    static bool isInitialized();

    std::string absolutePath(const std::string& path) const;
    std::string relativePath(const std::string& path, const Directory& baseDirectory = Directory()) const;

    //std::string fileName(const std::string& path) const;
    //std::string baseName(const std::string& path) const;
    //std::string fullBaseName(const std::string& path) const;
    //std::string directoryName(const std::string& path) const;
    //std::string parentDirectory(const std::string& directory) const;
    //std::string fileExtension(const std::string& path) const;
    Directory currentDirectory() const;
    bool changeDirectory(const Directory& directory) const;
    void setCurrentDirectory(const Directory& directory) const;

    bool fileExists(const File& path) const;
    bool deleteFile(const File& path) const;

    void registerPathToken(const std::string& token, const std::string& path);
    //static void unregisterPathToken(const std::string& token);

    //File& open(const std::string& filename, bool listenForFileChange);
    //bool exists(const std::string& filename);

//private:
    std::string cleanupPath(std::string path) const;
    size_t commonBasePathPosition(const std::string& p1, const std::string& p2) const;


    bool hasTokens(const std::string& path) const;
    bool expandPathTokens(std::string& path) const;
    std::vector<std::string> unresolvedTokens(const std::string& path) const;
    bool hasToken(const std::string& path, const std::string& token) const;
    const std::string resolveToken(const std::string& token) const;

    FileSystem();

    FileSystem(const FileSystem& rhs); // don't implement
    FileSystem& operator=(const FileSystem& rhs); // don't implement

    std::map<std::string, std::string> _tokenMap;
    static FileSystem* _fileSystem;
};

#define FileSys (ghoul::filesystem::FileSystem::ref())

} // namespace filesystem
} // namespace ghoul

#endif // __FILESYSTEM_H__
