/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2018                                                               *
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

#ifndef __GHOUL___VERSION___H__
#define __GHOUL___VERSION___H__

#include <ghoul/misc/stringconversion.h>
#include <string>

namespace ghoul::systemcapabilities {

/**
* This struct stores the detected version of the GLSL driver
*/
struct Version {
    /**
    * Returns <code>true</code>, if major, minor, and release version parts are the
    * same.
    *
    * \return <code>True</code> if both versions are the same
    */
    bool operator==(const Version& rhs) const;

    /**
    * Returns <code>true</code>, if major, minor, or release version parts are
    * different.
    *
    * \return <code>True</code> if either major, minor, or release version is
    *         different
    */
    bool operator!=(const Version& rhs) const;

    /**
    * First compares major version against major version, then minor against minor
    * and finally release against release.
    *
    * \return <code>True</code> if <code>this</code> is a smaller version than
    *         <code>rhs</code>
    */
    bool operator<(const Version& rhs) const;

    /**
    * First compares major version against major version, then minor against minor
    * and finally release against release.
    *
    * \return <code>True</code> if <code>this</code> is a smaller or equal version
    *         than <code>rhs</code>
    */
    bool operator<=(const Version& rhs) const;

    /**
    * First compares major version against major version, then minor against minor
    * and finally release against release.
    *
    * \return <code>True</code> if <code>this</code> is a greater version than
    *         <code>rhs</code>
    */
    bool operator>(const Version& rhs) const;

    /**
    * First compares major version against major version, then minor against minor
    * and finally release against release.
    *
    * \return <code>True</code> if <code>this</code> is a greater or equal version
    *         than <code>rhs</code>
    */
    bool operator>=(const Version& rhs) const;

    /// The <code>major</code> part of the version
    int major;
    /// The <code>minor</code> part of the version
    int minor;
    /// The <code>release</code> part of the version
    int release;
};

} // namespace ghoul::systemcapabilities

namespace ghoul {

template <>
std::string to_string(const ghoul::systemcapabilities::Version& v);

} // namespace ghoul

#endif // __GHOUL___VERSION___H__
