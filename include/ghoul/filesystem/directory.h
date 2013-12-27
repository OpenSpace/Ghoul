/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012 Alexander Bock                                                     *
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

#ifndef __DIRECTORY_H__
#define __DIRECTORY_H__

#include <functional>
#include <string>
#include <vector>

namespace ghoul {
namespace filesystem {

/**
 * This class represents a directory in the file system. It is identified by its path. 
 * Using the default constructor will create a Directory object pointing to the absolute
 * path of the current working directory. The other constructors can either create and
 * absolute path or use the provided path as-is.
 * The Directory has the possibility to list all files (\see read), or selectively only
 * read files (\see readFiles) or read directories (\see readDirectories)
 * \todo Implement an operator so that a Directory object can be used in a stream.
 */
class Directory {
public:
    /**
     * This constructor creates a Directory object pointing to the absolute path of the
     * current working directory.
     */
    Directory();
    
    /**
     * This constructor creates a Directory object pointing to the <code>path</code>. If
     * <code>isRawPath</code> is <code>true</code>, the <code>path</code> is used as-is
     * and not modified. If it is <code>false</code>, the default setting, the provided
     * <code>path</code> is automatically changed into the absolute path representation, 
     * removing all tokens, if present, in the process
     * \param path The path this Directory should point to. Can contain tokens and be 
     * specified relative to the current working directory
     * \param isRawPath If <code>false</code> the <code>path</code> parameter is converted
     * into an absolute path, removing all tokens in the process. If <code>true</code>,
     * the path is used as-is without changes. This might make the Directory object
     * outdated if the current working directory is subsequently changed.
     */
    Directory(const char* path, bool isRawPath = false);
    
    /**
     * This constructor creates a Directory object pointing to the <code>path</code>. If
     * <code>isRawPath</code> is <code>true</code>, the <code>path</code> is used as-is
     * and not modified. If it is <code>false</code>, the default setting, the provided
     * <code>path</code> is automatically changed into the absolute path representation,
     * removing all tokens, if present, in the process.
     * \param path The path this Directory should point to. Can contain tokens and be
     * specified relative to the current working directory
     * \param isRawPath If <code>false</code> the <code>path</code> parameter is converted
     * into an absolute path, removing all tokens in the process. If <code>true</code>,
     * the path is used as-is without changes. This might make the Directory object
     * outdated if the current working directory is subsequently changed.
     */
    Directory(const std::string& path, bool isRawPath = false);

    /**
     * Operator that returns the path this directory points to. This can, depending on the
     * constructor that was used, be either an absolute path or a relative path. If the
     * current working directory has been changed, and the Directory has been created
     * with a relative path, it might be outdated and point to a different location than
     * intended.
     * \return The stored path this Directory points to
     */
    operator const std::string&() const;

    /**
     * Returns the path this directory points to. This can, depending on the constructor
     * that was used, be either an absolute path or a relative path. If the current
     * working directory has been changed, and the Directory has been created with a
     * relative path, it might be outdated and point to a different location than
     * intended.
     * \return The stored path this Directory points to
     */
    const std::string& path() const;

    /**
     * Returns the Directory object that points to the parent directory of the current
     * object. The path to this new object is created by appending <code>..</code> to the
     * path of the current object. The <code>absolutePath</code> parameter determines if
     * the new object is created using the absolute or relative path.
     * \param absolutePath Determines if the path to the new object should be converted
     * into an absolute path or remain relative
     * \return The Directory object pointing to the parent directory of the current
     * directory.
     */
    Directory parentDirectory(bool absolutePath = false) const;
    
    /**
     * This method creates a list of all files and subdirectories in the current directory
     * and returns the paths to each. If <code>recursiveSearch</code> is <code>true
     * </code>, each subdirectory will be searched as well and all results will be 
     * combined. The parameter <code>sort</code> determines if the end result will be
     * sorted by name.
     * \param recursiveSearch Determines if the subdirectories will be searched as well as
     * the current directory.
     * \param sort If <code>true</code> the final result will be sorted by name
     * \return The paths to all files and directories in the current directory (and
     * subdirectories if <code>recursiveSearch</code> is <code>true</code>
     */
    std::vector<std::string> read(bool recursiveSearch = false, bool sort = false) const;

    /**
     * This method creates a list of all files in the current directory and returns the
     * paths to each. If <code>recursiveSearch</code> is <code>true</code>, each
     * subdirectory will be searched as well and all results will be combined. The
     * parameter <code>sort</code> determines if the endresult will be sorted by name.
     * \param recursiveSearch Determines if the subdirectories will be searched as well as
     * the current directory.
     * \param sort If <code>true</code> the final result will be sorted by name
     * \return The paths to all files in the current directory (and subdirectories if
     * <code>recursiveSearch</code> is <code>true</code>
     */
    std::vector<std::string> readFiles(bool recursiveSearch = false,
                                       bool sort = false) const;

    /**
     * This method creates a list of all subdirectories in the current directory and
     * returns the paths to each. If <code>recursiveSearch</code> is <code>true</code>,
     * each subdirectory will be searched as well and all results will be combined. The
     * parameter <code>sort</code> determines if the end result will be sorted by name.
     * \param recursiveSearch Determines if the subdirectories will be searched as well as
     * the current directory.
     * \param sort If <code>true</code> the final result will be sorted by name
     * \return The paths to all directories in the current directory (and subdirectories
     * if <code>recursiveSearch</code> is <code>true</code>
     */
std::vector<std::string> readDirectories(bool recursiveSearch = false,
                                         bool sort = false) const;

private:
    /**
     * Internal function that does the directory marching of all files that are in the
     * directory point to by <code>path</code>. It will combine all results in the <code>
     * result</code> parameter and will, if <code>recursiveSearch</code> is <code>true
     * </code> recursively call itself; thus reusing the <code>result</code> parameter.
     * \param result The result vector that will contain all files in the directory
     * \param path The path of the directory whose files should be listed
     * \param recursiveSearch If <code>true</code>, this method will be called recursively 
     * for each directory in the current directory, combining all results in the <code>
     * result</code> vector.
     */
     void readFiles(
        std::vector<std::string>& result,
        const std::string& path,
        bool recursiveSearch = false) const;

    /**
     * Internal function that does the directory marching of all subdirectories that are
     * in the directory point to by <code>path</code>. It will combine all results in the
     * <code>result</code> parameter and will, if <code>recursiveSearch</code> is <code>
     * true</code> recursively call itself; thus reusing the <code>result</code>
     * parameter.
     * \param result The result vector that will contain all files in the directory
     * \param path The path of the directory whose files should be listed
     * \param recursiveSearch If <code>true</code>, this method will be called recursively 
     * for each directory in the current directory, combining all results in the <code>
     * result</code> vector.
     */
    void readDirectories(
        std::vector<std::string>& result,
        const std::string& path,
        bool recursiveSearch = false) const;

    /// The path in the filesystem to this Directory object. May be absolute or relative
    std::string _directoryPath;
};

} // namespace filesystem
} // namespace ghoul

#endif // __DIRECTORY_H__
