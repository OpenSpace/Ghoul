/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2020                                                               *
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

#ifndef __GHOUL___DIRECTORY___H__
#define __GHOUL___DIRECTORY___H__

#include <ghoul/misc/boolean.h>
#include <string>
#include <vector>

namespace ghoul::filesystem {

/**
 * This class represents a directory in the file system. The directory is identified by
 * its path. Using the default constructor will create a Directory object pointing to the
 * absolute path of the current working directory. The other constructors can either
 * create and absolute path or use the provided path as-is.
 * The Directory has the possibility to list all files (#read), or selectively only
 * read files (#readFiles), read directories (#readDirectories), or get the parent
 * (#parentDirectory).
 */
class Directory {
public:
    BooleanType(RawPath);
    BooleanType(AbsolutePath);
    BooleanType(Recursive);
    BooleanType(Sort);

    /**
     * This constructor creates a Directory object pointing to the absolute path of the
     * current working directory.
     */
    Directory();

    /**
     * This constructor creates a Directory object pointing to the \p path. If
     * \p isRawPath is <code>true</code>, the \p path is used as-is and not modified. If
     * it is <code>false</code>, the default setting, the provided \p path is
     * automatically changed into the absolute path representation, removing all tokens,
     * if present, in the process.
     *
     * \param path The path this Directory should point to. Can contain tokens and be
     *        specified relative to the current working directory
     * \param isRawPath If <code>false</code> the \p path parameter is converted into an
     *        absolute path, removing all tokens in the process. If <code>true</code>,
     *        the path is used as-is without changes. This might make the Directory object
     *        outdated if the current working directory is subsequently changed.
     */
    Directory(std::string path, RawPath isRawPath = RawPath::Yes);

    /**
     * This constructor creates a Directory object pointing to the \p path. If
     * \p isRawPath is <code>true</code>, the \p path is used as-is and not modified. If
     * it is <code>false</code>, the default setting, the provided \p path is
     * automatically changed into the absolute path representation, removing all tokens,
     * if present, in the process.
     *
     * \param path The path this Directory should point to as a null terminated string.
     *        Can contain tokens and be specified relative to the current working
     *        directory
     * \param isRawPath If <code>false</code> the \p path parameter is converted into an
     *        absolute path, removing all tokens in the process. If <code>true</code>,
     *        the path is used as-is without changes. This might make the Directory object
     *        outdated if the current working directory is subsequently changed.
     */
    Directory(const char* path, RawPath isRawPath = RawPath::Yes);

    /**
     * Operator that returns the path this directory points to. This can, depending on the
     * constructor that was used, be either an absolute path or a relative path. If the
     * current working directory has been changed, and the Directory has been created
     * with a relative path, it might be outdated and point to a different location than
     * intended.
     *
     * \return The stored path this Directory points to
     */
    operator const std::string&() const;

    /**
     * Returns the path this directory points to. This can, depending on the constructor
     * that was used, be either an absolute path or a relative path. If the current
     * working directory has been changed, and the Directory has been created with a
     * relative path, it might be outdated and point to a different location than
     * intended.
     *
     * \return The stored path this Directory points to
     */
    const std::string& path() const;

    /**
     * Returns the Directory object that points to the parent directory of the current
     * object. The path to this new object is created by appending <code>..</code> to the
     * path of the current object. The \p absolutePath parameter determines if the new
     * object is created using the absolute or relative path.
     *
     * \param absolutePath Determines if the path to the new object should be converted
     *        into an absolute path or remain relative
     * \return The Directory object pointing to the parent directory of the current
     *         directory.
     */
    Directory parentDirectory(AbsolutePath absolutePath = AbsolutePath::No) const;

    /**
     * This method creates a list of all files and subdirectories in the current directory
     * and returns the paths to each. If \p recursiveSearch is <code>true</code>, each
     * subdirectory will be searched as well and all results will be combined. The
     * parameter \p sort determines if the end result will be sorted by name.
     *
     * \param recursiveSearch Determines if the subdirectories will be searched as well as
     *        the current directory.
     * \param sort If <code>true</code> the final result will be sorted by name
     * \return The paths to all files and directories in the current directory (and
     *         subdirectories if \p recursiveSearch is <code>true</code>)
     */
    std::vector<std::string> read(Recursive recursiveSearch = Recursive::No,
        Sort sort = Sort::No) const;

    /**
     * This method creates a list of all files in the current directory and returns the
     * path to each. If \p recursiveSearch is <code>true</code>, each subdirectory will be
     * searched as well and all results will be combined. The parameter \p sort determines
     * if the end result will be sorted by name.
     *
     * \param recursiveSearch Determines if the subdirectories will be searched as well as
     *        the current directory.
     * \param sort If <code>true</code> the final result will be sorted by name
     * \return The paths to all files in the current directory (and subdirectories if
     *         \p recursiveSearch is <code>true</code>)
     */
    std::vector<std::string> readFiles(Recursive recursiveSearch = Recursive::No,
        Sort sort = Sort::No) const;

    /**
     * This method creates a list of all subdirectories in the current directory and
     * returns the path to each. If \p recursiveSearch is <code>true</code>, each
     * subdirectory will be searched as well and all results will be combined. The
     * parameter \p sort determines if the end result will be sorted by name.
     *
     * \param recursiveSearch Determines if the subdirectories will be searched as well as
     *        the current directory.
     * \param sort If <code>true</code> the final result will be sorted by name
     * \return The paths to all directories in the current directory (and subdirectories
     *         if \p recursiveSearch is <code>true</code>)
     */
    std::vector<std::string> readDirectories(Recursive recursiveSearch = Recursive::No,
        Sort sort = Sort::No) const;

private:
    /**
     * Internal function that does the directory marching of all files that are in the
     * directory point to by \p path. It will combine all results in the \p result
     * parameter and will, if \p recursiveSearch is <code>true</code> recursively call
     * itself; thus reusing the \p result parameter.
     *
     * \param result The result vector that will contain all files in the directory
     * \param path The path of the directory whose files should be listed
     * \param recursiveSearch If <code>true</code>, this method will be called recursively
     *        for each directory in the current directory, combining all results in the
     *        \p result> vector
     */
    void internalReadFiles(std::vector<std::string>& result, const std::string& path,
        Recursive recursiveSearch = Recursive::No) const;

    /**
     * Internal function that does the directory marching of all subdirectories that are
     * in the directory point to by \p path. It will combine all results in the \p result
     * parameter and will, if \p recursiveSearch is <code>true</code> recursively call
     * itself; thus reusing the \p result parameter.
     *
     * \param result The result vector that will contain all files in the directory
     * \param path The path of the directory whose files should be listed
     * \param recursiveSearch If <code>true</code>, this method will be called recursively
     *        for each directory in the current directory, combining all results in the
     *        \p result vector.
     */
    void internalReadDirectories(std::vector<std::string>& result,
        const std::string& path, Recursive recursiveSearch = Recursive::No) const;

    /// The path in the filesystem to this Directory object. May be absolute or relative
    std::string _directoryPath;
};

} // namespace ghoul::filesystem

#endif // __GHOUL___DIRECTORY___H__
