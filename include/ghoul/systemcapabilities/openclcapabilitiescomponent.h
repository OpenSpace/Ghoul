/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2014                                                               *
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

#ifndef __OPENCLCAPABILITIESCOMPONENT_H__
#define __OPENCLCAPABILITIESCOMPONENT_H__

#include "systemcapabilitiescomponent.h"

#include <string>
#include <vector>

namespace ghoul {
namespace systemcapabilities {

/**
 * This subclass of SystemCapabilitiesComponent detects graphics and OpenGL-related
 * capabilities, like the OpenGL version, supported extensions, or the driver version.
 */
class OpenCLCapabilitiesComponent : public SystemCapabilitiesComponent {
public:
    /**
     * This struct stores the detected version of the GLSL driver
     */
    struct Version {
        /// Constructor initializing values
        Version(int major = 0, int minor = 0, int release = 0);
        
        /**
         * Returns the stored CL version in the format <code>major.minor.release</code>.
         * \return The CL version as a string
         */
        std::string toString() const;

        /**
         * Returns <code>true</code>, if major, minor, and release version parts are the
         * same.
         * \return <code>True</code> if both versions are the same
         */
        bool operator==(const Version& rhs) const;

        /**
         * Returns <code>true</code>, if major, minor, or release version parts are
         * different.
         * \return <code>True</code> if either major, minor, or release version is
         * different
         */
        bool operator!=(const Version& rhs) const;

        /**
         * First compares major version against major version, then minor against minor
         * and finally release against release.
         * \return <code>True</code> if <code>this</code> is a smaller version than
         * <code>rhs</code>
         */
        bool operator<(const Version& rhs) const;

        /**
         * First compares major version against major version, then minor against minor
         * and finally release against release.
         * \return <code>True</code> if <code>this</code> is a smaller or equal version
         * than <code>rhs</code>
         */
        bool operator<=(const Version& rhs) const;

        /**
         * First compares major version against major version, then minor against minor
         * and finally release against release.
         * \return <code>True</code> if <code>this</code> is a greater version than
         * <code>rhs</code>
         */
        bool operator>(const Version& rhs) const;

        /**
         * First compares major version against major version, then minor against minor
         * and finally release against release.
         * \return <code>True</code> if <code>this</code> is a greater or equal version
         * than <code>rhs</code>
         */
        bool operator>=(const Version& rhs) const;

        unsigned char _major; ///< The <code>major</code> part of the version
        unsigned char _minor; ///< The <code>minor</code> part of the version
        unsigned char _release; ///< The <code>release</code> part of the version
    };


    OpenCLCapabilitiesComponent();
    ~OpenCLCapabilitiesComponent();

    std::vector<CapabilityInformation> capabilities(
        const SystemCapabilitiesComponent::Verbosity& verbosity) const override;

    /**
     * Returns the maximum OpenCL version that is supported on this platform. This means
     * that all the lower version will be supported as well,
     * \return The maximum OpenCL version
     */
    //const Version& openCLVersion() const;
    
    /**
     * Returns the <code>OpenCL</code> string.
     * \return The <code>OpenCL</code> string
     */
    const std::string name() const override;
    
protected:
    void detectCapabilities() override;
    void clearCapabilities() override;
    
    Version _clVersion;
};

} // namespace systemcapabilities
} // namespace ghoul

#endif // __OPENCLCAPABILITIESCOMPONENT_H__
