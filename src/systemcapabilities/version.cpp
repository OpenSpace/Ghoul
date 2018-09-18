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

#include <ghoul/systemcapabilities/version.h>

namespace {

unsigned int packVersion(int major, int minor, int release) {
    // safe since: 2^8 * 1000 * 1000 < 2^32
    return
        major * 1000 * 1000 +
        minor * 1000 +
        release;
}

} // namespace

namespace ghoul {

template <>
std::string to_string(const ghoul::systemcapabilities::Version& v) {
    if (v.release != 0) {
        return to_string(v.major) + "." + to_string(v.minor) + "." + to_string(v.release);
    } else {
        return to_string(v.major) + "." + to_string(v.minor);
    }
}

} // namespace ghoul

namespace ghoul::systemcapabilities {

bool Version::operator==(const Version& rhs) const {
    return (major == rhs.major) && (minor == rhs.minor) && (release == rhs.release);
}

bool Version::operator!=(const Version& rhs) const {
    return !(*this == rhs);
}

bool Version::operator<(const Version& rhs) const {
    const unsigned int numThis = packVersion(major, minor, release);
    const unsigned int numRhs = packVersion(rhs.major, rhs.minor, rhs.release);

    return numThis < numRhs;
}

bool Version::operator<=(const Version& rhs) const {
    const unsigned int numThis = packVersion(major, minor, release);
    const unsigned int numRhs = packVersion(rhs.major, rhs.minor, rhs.release);

    return numThis <= numRhs;
}

bool Version::operator>(const Version& rhs) const {
    const unsigned int numThis = packVersion(major, minor, release);
    const unsigned int numRhs = packVersion(rhs.major, rhs.minor, rhs.release);

    return numThis > numRhs;
}

bool Version::operator>=(const Version& rhs) const {
    const unsigned int numThis = packVersion(major, minor, release);
    const unsigned int numRhs = packVersion(rhs.major, rhs.minor, rhs.release);

    return numThis >= numRhs;
}

} // namespace ghoul::systemcapabilities
