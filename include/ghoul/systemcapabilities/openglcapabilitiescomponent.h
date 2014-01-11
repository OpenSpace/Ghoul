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

#ifndef __OPENGLCAPABILITIESCOMPONENT_H__
#define __OPENGLCAPABILITIESCOMPONENT_H__

#include "systemcapabilities/systemcapabilitiescomponent.h"

#include <string>
#include <vector>

namespace ghoul {
namespace systemcapabilities {

class OpenGLCapabilitiesComponent : public SystemCapabilitiesComponent{
public:
    /**
     * This struct stores the detected version of the GLSL driver
     */
    struct Version {
        /// Constructor initializing values
        Version(int major = 0, int minor = 0, int release = 0);

        /**
         * Parses the version string, which should be either of the format
         * <code>major.minor.release vendor-specific information</code> or
         * <code>major.minor vendor-specific information</code> and should normally be retrieved
         * using the <code>glGetString(GL_SHADING_LANGUAGE_VERSION)</code> method.
         * \param version The version as a formatted string 
         * \return <code>True</code> if the string could be successfully parsed; <code>false</code>
         * otherwise. If the parsing failed, the version parts will not be changed.
         */
        bool parseGLSLString(const std::string& version);

        /**
         * Returns the stored GLSL version in the format <code>major.minor.release</code>.
         * \return The GLSL version as a string
         */
        std::string toString() const;

        /**
         * Returns true, if major, minor, and release version parts are the same
         * \return <code>True</code> if both versions are the same
         */
        bool operator==(const Version& rhs) const;

        /**
         * Returns true, if major, minor, or release version parts are different
         * \return <code>True</code> if either major, minor, or release version is different
         */
        bool operator!=(const Version& rhs) const;

        /**
         * First compares major version against major version, then minor against minor and
         * finally release against release
         * \return <code>True</code> if <code>this</code> is a smaller version than
         * <code>rhs</code>
         */
        bool operator<(const Version& rhs) const;

        /**
         * First compares major version against major version, then minor against minor and
         * finally release against release
         * \return <code>True</code> if <code>this</code> is a smaller or equal version than
         * <code>rhs</code>
         */
        bool operator<=(const Version& rhs) const;

        /**
         * First compares major version against major version, then minor against minor and
         * finally release against release
         * \return <code>True</code> if <code>this</code> is a greater version than
         * <code>rhs</code>
         */
        bool operator>(const Version& rhs) const;

        /**
         * First compares major version against major version, then minor against minor and
         * finally release against release
         * \return <code>True</code> if <code>this</code> is a greater or equal version than
         * <code>rhs</code>
         */
        bool operator>=(const Version& rhs) const;

        unsigned char _major; ///< The <code>major</code> part of the version
        unsigned char _minor; ///< The <code>minor</code> part of the version
        unsigned char _release; ///< The <code>release</code> part of the version
    };

    
    enum class Vendor {
        Nvidia,
        ATI,
        Intel,
        Other
    };

    OpenGLCapabilitiesComponent();
    ~OpenGLCapabilitiesComponent();

    std::string createCapabilitiesString(
        const SystemCapabilitiesComponent::Verbosity& verbosity) const override;

    const Version& openGLVersion() const;
    const std::string& gpuVendorString() const;
    const Vendor& gpuVendor() const;
    std::string vendorString() const;

    const std::vector<std::string>& extensions() const;
    bool isExtensionSupported(const std::string& extension) const;

    int maximumNumberOfTextureUnits() const;

protected:
    void detectCapabilities() override;
    void clearCapabilities() override;

    void detectGLSLVersion();
    void detectGPUVendor();
    void detectGLRenderer();
    void detectExtensions();
    void detectGLEWVersion();
    void detectDriverInformation();

    Version _glslVersion; ///< GLSL Version
    std::string _glslCompiler; ///< GPU vendor
    Vendor _vendor; ///< GPU vendor
    std::string _glRenderer; ///< GL_RENDERER
    std::vector<std::string> _extensions; ///< supported GLSL extensions
    Version _glewVersion; ///< GLEW Version
    
    int _maxTextureSize; ///< The maximum size a texture can have
    int _maxTextureSize3D; ///< GL_MAX_3D_TEXTURE_SIZE
    int _numTextureUnits; ///< The maximum number of texture units
    int _maxFramebufferColorAttachments; ///< The maximum number of color attachments for an FBO
    
    bool _supportTexturing3D; // GL_EXT_texture3D

#ifdef GHOUL_USE_WMI
    std::string _driverVersion;
    std::string _driverDate;
    unsigned int _adapterRAM;
    std::string _adapterName;
#endif
};

} // namespace systemcapabilities
} // namespace ghoul

#endif // __OPENGLCAPABILITIESCOMPONENT_H__
