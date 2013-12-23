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

#ifndef __SYSCAPABILITIES_H__
#define __SYSCAPABILITIES_H__

#include <string>
#include <vector>

#ifdef GHOUL_USE_WMI
    #include <wbemidl.h>
    #include <comdef.h>
#endif

namespace ghoul {

class SystemCapabilities {
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
         * \param [in] version The version as a formatted string 
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

    
    enum Vendor {
        VendorNvidia,
        VendorATI,
        VendorIntel,
        VendorOther
    };

    SystemCapabilities();
    virtual ~SystemCapabilities();

    static void init();
    static void deinit();
    static SystemCapabilities& ref();
    static bool isInitialized();

    void detectCapabilities();

    void logCapabilities() const;

    const Version& openGLVersion() const;
    const std::string& gpuVendorString() const;
    const Vendor& gpuVendor() const;

    const std::vector<std::string>& extensions() const;
    bool isExtensionSupported(const std::string& extension) const;

    int maximumNumberOfTextureUnits() const;

protected:
    SystemCapabilities(const SystemCapabilities& rhs);

    void detectOS();
    void clearCapabilities();

    void initializeWMI();
    void deinitializeWMI();

#ifdef GHOUL_USE_WMI
    bool isWMIinitialized() const;

    VARIANT* queryWMI(const std::string& wmiClass, const std::string& attribute) const;

    bool queryWMI(const std::string& wmiClass,
        const std::string& attribute, std::string& value) const;
    bool queryWMI(const std::string& wmiClass, const std::string& attribute, int& value) const;
    bool queryWMI(const std::string& wmiClass, const std::string& attribute,
        unsigned int& value) const;
    bool queryWMI(const std::string& wmiClass, const std::string& attribute,
        unsigned long long& value) const;
#endif

    std::string _operatingSystem; ///< Information about the Operating system
    unsigned int _installedMainMemory; ///< The amount of RAM that is installed on this machine

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
    
    
    bool _supportNVHalfFloat : 1; // GL_NV_half_float
    bool _supportDebugOutput : 1; // GL_ARB_debug_output
    bool _supportDepthBufferFloat : 1; // GL_ARB_depth_buffer_float
    bool _supportFrameBufferObject : 1; // GL_ARB_framebuffer_object
    bool _supportShadingInclude : 1; // GL_ARB_shading_language_include
    bool _supportTessellationShader : 1; // GL_ARB_tessellation_shader
    bool _supportTimerQuery : 1; // GL_ARB_timer_query
    bool _supportNVXGpuMemoryInfo : 1; // GL_NVX_gpu_memory_info
    bool _supportNVCopyImage : 1; // GL_NV_copy_image
    bool _supportNPoTTextures : 1; // GL_ARB_texture_non_power_of_two
    bool _supportTexturing3D : 1; // GL_EXT_texture3D
    bool _supportTextureRectangles : 1; // GL_ARB_texture_rectangle
    bool _supportTextureFilterAnisotropic : 1; // GL_EXT_texture_filter_anisotropic
   

#ifdef GHOUL_USE_WMI
    IWbemLocator* _iwbemLocator;
    IWbemServices* _iwbemServices;

    std::string _driverVersion;
    std::string _driverDate;
    unsigned int _adapterRAM;
    std::string _adapterName;
#endif

private:
    static SystemCapabilities* _systemCapabilities;  ///< singleton member
};

}

#define SysCap (ghoul::SystemCapabilities::ref())

#endif // __SYSCAPABILITIES_H__
