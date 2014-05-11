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

#ifndef __OPENGLCAPABILITIESCOMPONENT_H__
#define __OPENGLCAPABILITIESCOMPONENT_H__

#include "systemcapabilitiescomponent.h"

#include <string>
#include <vector>

namespace ghoul {
namespace systemcapabilities {

/**
 * This subclass of SystemCapabilitiesComponent detects graphics and OpenGL-related
 * capabilities, like the OpenGL version, supported extensions, or the driver version.
 */
class OpenGLCapabilitiesComponent : public SystemCapabilitiesComponent {
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
         * <code>major.minor vendor-specific information</code> and should normally be
         * retrieved using the <code>glGetString(GL_SHADING_LANGUAGE_VERSION)</code>
         * method.
         * \param version The version as a formatted string 
         * \return <code>True</code> if the string could be successfully parsed;
         * <code>false</code> otherwise. If the parsing failed, the version parts will not
         * be changed.
         */
        bool parseGLSLString(const std::string& version);

        /**
         * Returns the stored GLSL version in the format <code>major.minor.release</code>.
         * \return The GLSL version as a string
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

    /// This enum stores the possible vendors of graphics cards that can be detected
    enum class Vendor {
        Nvidia, ///< Nvidia
        ATI, ///< AMD/ATI
        Intel, ///< Intel
        Other ///< vendor could not be detected
    };

    OpenGLCapabilitiesComponent();
    ~OpenGLCapabilitiesComponent();

    std::vector<CapabilityInformation> capabilities(
        const SystemCapabilitiesComponent::Verbosity& verbosity) const override;

    /**
     * Returns the maximum OpenGL version that is supported on this platform. This means
     * that all the lower version will be supported as well,
     * \return The maximum OpenGL version
     */
    const Version& openGLVersion() const;

    /**
     * Returns the value of a call to <code>glGetString(GL_VENDOR)</code>. This will give
     * detailed information about the vendor of the main graphics card. This string can be
     * used if the automatic Vendor detection failed
     * \return The value of a call to <code>glGetString(GL_VENDOR)</code>
     */
    const std::string& glslCompiler() const;

    /**
     * Returns the vendor of the main graphics card.
     * \return The vendor of the main graphics card
     */
    const Vendor& gpuVendor() const;

    /**
     * Returns the vendor of the main graphics card converted into a string.
     * \return The vendor of the main graphics card converted into a string
     */
    std::string gpuVendorString() const;

    /**
     * Returns all available extensions as a list of names.
     * \return All available extensions as a list of names
     */
    const std::vector<std::string>& extensions() const;

    /**
     * Checks is a specific <code>extension</code> is supported or not
     * \return <code>true</code> if the <code>extension</code> is supported;
     * <code>false</code> otherwise
     */
    bool isExtensionSupported(const std::string& extension) const;

    /**
     * Returns the maximum number of texture units that are available on the main
     * graphics card
     * \return The maximum number of texture units
     */
    int maximumNumberOfTextureUnits() const;

    /**
     * Returns the <code>OpenGL</code> string.
     * \return The <code>OpenGL</code> string
     */
    std::string name() const override;

protected:
    void detectCapabilities() override;
    void clearCapabilities() override;

    /// Detect the maximum supported GLSL Version
    void detectGLSLVersion();
    /// Detect the vendor of the main GPU
    void detectGPUVendor();
    /// Get the vendor string from OpenGL
    void detectGLRenderer();
    /// Detect all available extensions
    void detectExtensions();
    /// Detect the available GLEW version
    void detectGLEWVersion();
    /// Use WMI (on Windows) to retrieve information about the installed driver
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

//#ifdef GHOUL_USE_WMI
    std::string _driverVersion; ///< Stores the version of the installed driver
    std::string _driverDate; ///< Stores the date of the installed driver
    unsigned int _adapterRAM; ///< How many MB of memory is installed on the main GPU
    std::string _adapterName; ///< The name of the main GPU
//#endif
};

} // namespace systemcapabilities
} // namespace ghoul

#endif // __OPENGLCAPABILITIESCOMPONENT_H__
