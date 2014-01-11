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

#include "systemcapabilities/openglcapabilitiescomponent.h"

#include <ghoul/opengl/ghoul_gl.h>
#include <algorithm>
#include <cassert>
#include <sstream>
#include "logging/logmanager.h"

#ifdef WIN32
    #include <Windows.h>
    #include <tchar.h>
    #pragma comment(lib, "User32.lib")
    #pragma comment(lib, "Kernel32.lib")
    typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
    typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, DWORD);
#else
    #include <sys/utsname.h>
#endif

using std::string;
using std::stringstream;
using std::vector;
using std::wstring;

namespace {
    const std::string _loggerCat = "OpenGLCapabilities";
}

namespace ghoul {
namespace systemcapabilities {

OpenGLCapabilitiesComponent::OpenGLCapabilitiesComponent() 
    : SystemCapabilitiesComponent("OpenGL")
{
    clearCapabilities();
}

OpenGLCapabilitiesComponent::~OpenGLCapabilitiesComponent() {
    deinitialize();
}

void OpenGLCapabilitiesComponent::detectCapabilities() {
    SystemCapabilitiesComponent::detectCapabilities();
    clearCapabilities();

    detectGLSLVersion();
    detectGPUVendor();
    detectGLRenderer();
    detectExtensions();
    detectGLEWVersion();
    detectDriverInformation();

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &_maxTextureSize);
    if (_glewVersion >= Version(1,2,0) || glewIsSupported("GL_EXT_texture3D"))
        _supportTexturing3D = true;

    if (_supportTexturing3D) {
        if (_glewVersion >= Version(2,0,0))
            glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &_maxTextureSize3D);
        else
            glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE_EXT, &_maxTextureSize3D);
    }

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &_numTextureUnits);
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &_maxFramebufferColorAttachments);
}

void OpenGLCapabilitiesComponent::detectGLSLVersion() {
    const char* glslVersion =
        reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    if (glslVersion) {
        bool success = _glslVersion.parseGLSLString(string(glslVersion));
        if (!success)
            LERROR_SAFE("Detection of the GLSL version failed. Detected version: '" 
            << string(glslVersion) << "'");
    }
    else
        LERROR_SAFE("Detection the GLSL version failed. 'glGetString' returned 0.");
}

void OpenGLCapabilitiesComponent::detectGPUVendor() {
    const char* vendor =
        reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    if (vendor)
        _glslCompiler = string(vendor);
    else
        LERROR_SAFE("Detection the GPU Vendor failed. 'glGetString' returned 0.");

    if (_glslCompiler.find("NVIDIA") != string::npos)
        _vendor = Vendor::Nvidia;
    else if (_glslCompiler.find("ATI") != string::npos)
        _vendor = Vendor::ATI;
    else if ((_glslCompiler.find("INTEL") != string::npos) 
        || (_glslCompiler.find("Intel") != string::npos))
    {
        _vendor = Vendor::Intel;
    }
    else {
        LINFO_SAFE("Vendor of graphics card is not in the enum 'Vendor'. Vendor information: "
            << _glslCompiler);
        _vendor = Vendor::Other;
    }
}

void OpenGLCapabilitiesComponent::detectGLRenderer() {
    _glRenderer = string(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
}

void OpenGLCapabilitiesComponent::detectExtensions() {
    GLint nExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &nExtensions);
    for (GLint i = 0; i < nExtensions; ++i) {
        const GLubyte* ext = glGetStringi(GL_EXTENSIONS, i);
        const string extension = string(reinterpret_cast<const char*>(ext));
        _extensions.push_back(extension);
    }
}

void OpenGLCapabilitiesComponent::detectGLEWVersion() {
    _glewVersion._major = GLEW_VERSION_MAJOR;
    _glewVersion._minor = GLEW_VERSION_MINOR;
    _glewVersion._release = GLEW_VERSION_MICRO;
}

void OpenGLCapabilitiesComponent::detectDriverInformation() {
#ifdef GHOUL_USE_WMI
    bool versionSuccess = queryWMI("Win32_VideoController", "DriverVersion", _driverVersion);
    if (!versionSuccess) {
        LERROR_SAFE("Reading of video controller driver version failed.");
        _driverVersion = "";
    }

    string driverDateFull;
    bool dateSuccess = queryWMI("Win32_VideoController", "DriverDate", driverDateFull);
    if (!dateSuccess)
        LERROR_SAFE("Reading of video controller driver date failed.");
    else {
        stringstream dateStream;
        dateStream << driverDateFull.substr(0,4) << "-"
            << driverDateFull.substr(4,2) << "-"
            << driverDateFull.substr(6,2);
        _driverDate = dateStream.str();
    }

    bool adapterRAMSuccess = queryWMI("Win32_VideoController", "AdapterRAM", _adapterRAM);
    if (!adapterRAMSuccess) {
        LERROR_SAFE("Reading of video controller RAM failed.");
        _adapterRAM = 0;
    }
    else {
        // adapterRAM is in bytes
        _adapterRAM = (_adapterRAM / 1024) / 1024;
    }

    bool nameSucess = queryWMI("Win32_VideoController", "Name", _adapterName);
    if (!nameSucess) {
        LERROR_SAFE("Reading of video controller's name failed.");
        _adapterName = "";
    }
#endif
}

void OpenGLCapabilitiesComponent::clearCapabilities() {
    _glslVersion._major = 0;
    _glslVersion._minor = 0;
    _glslVersion._release = 0;
    _glslCompiler.clear();
    _vendor = Vendor::Other;
    _glRenderer = "";
    _extensions.clear();
    _glewVersion._major = 0;
    _glewVersion._minor = 0;
    _glewVersion._release = 0;

    _maxTextureSize = -1;
    _maxTextureSize3D = -1;
    _numTextureUnits = -1;

    _supportTexturing3D = false;

#ifdef GHOUL_USE_WMI
    _driverVersion = "";
    _driverDate = "";
    _adapterRAM = 0;
    _adapterName = "";
#endif
}

std::string OpenGLCapabilitiesComponent::createCapabilitiesString(
    const SystemCapabilitiesComponent::Verbosity& verbosity) const
{
    std::stringstream result;
    result << "OpenGL Version:        " << _glslVersion.toString() << "\n";
    result << "OpenGL Compiler:       " << _glslCompiler << "\n";
    result << "OpenGL Renderer:       " << _glRenderer << "\n";
    result << "GPU Vendor:            " << vendorString() << "\n";
    result << "GLEW Version:          " << _glewVersion.toString() << "\n";
#ifdef GHOUL_USE_WMI
    result << "GPU Name:              " << _adapterName << "\n";
    result << "GPU Driver Version:    " << _driverVersion << "\n";
    result << "GPU Driver Date:       " << _driverDate << "\n";
    result << "GPU RAM:               " << _adapterRAM << " MB\n";
#endif

    if (verbosity >= Verbosity::Default) {
        result << "Max Texture Size:      " << _maxTextureSize << "\n";
        result << "Max 3D Texture Size:   " << _maxTextureSize3D << "\n";
        result << "Num of Texture Units:  " << _numTextureUnits << "\n";
        result << "FBO Color Attachments: " << _maxFramebufferColorAttachments << "\n";
    }

    if (verbosity >= Verbosity::Full) {
        result << "Extensions:            ";
        for (size_t i = 0; i < _extensions.size() - 1; ++i)
            result << _extensions[i] << ", ";
        result << _extensions[_extensions.size() - 1] << "\n";
    }
    return result.str();
}

const OpenGLCapabilitiesComponent::Version& OpenGLCapabilitiesComponent::openGLVersion() const {
    return _glslVersion;
}

const string& OpenGLCapabilitiesComponent::gpuVendorString() const {
    return _glslCompiler;
}

const OpenGLCapabilitiesComponent::Vendor& OpenGLCapabilitiesComponent::gpuVendor() const {
    return _vendor;
}

const vector<string>& OpenGLCapabilitiesComponent::extensions() const {
    return _extensions;
}

bool OpenGLCapabilitiesComponent::isExtensionSupported(const string& extension) const {
    vector<string>::const_iterator result =
        std::find(_extensions.begin(), _extensions.end(), extension);
    return (result != _extensions.end());
}

int OpenGLCapabilitiesComponent::maximumNumberOfTextureUnits() const {
    return _numTextureUnits;
}

std::string OpenGLCapabilitiesComponent::vendorString() const {
    switch (_vendor) {
        case Vendor::Nvidia:
            return "Nvidia";
        case Vendor::ATI:
            return "ATI";
        case Vendor::Intel:
            return "Intel";
        default:
            return "other";
    }
}

/////////////////////////////
/// OpenGLVersion
/////////////////////////////

unsigned int packVersion(unsigned char major, unsigned char minor, unsigned char release);

unsigned int packVersion(unsigned char major,
                         unsigned char minor,
                         unsigned char release)
{
    // safe since: 2^8 * 1000 * 1000 < 2^32
    return
        major * 1000 * 1000 +
        minor * 1000        +
        release;
}

OpenGLCapabilitiesComponent::Version::Version(int major, int minor, int release) 
    : _major(static_cast<unsigned char>(major))
    , _minor(static_cast<unsigned char>(minor))
    , _release(static_cast<unsigned char>(release))
{}

bool OpenGLCapabilitiesComponent::Version::parseGLSLString(const string& version) {
    // version string has one of the formats:
    // <major version>.<minor version>.<release version> <vendor specific information>
    // <major version>.<minor version> [<vendor specific information>]

    stringstream stream;

    size_t separatorMajorMinor = version.find_first_of('.');
    if (separatorMajorMinor == string::npos)
        return false;
    string major = version.substr(0, separatorMajorMinor);
    size_t separatorMinorRelease = version.find_first_of('.', separatorMajorMinor + 1);
    string minor = "";
    string release = "";
    if (separatorMinorRelease != string::npos) {
        // first format
        size_t len = separatorMinorRelease - (separatorMajorMinor + 1);
        minor = version.substr(separatorMajorMinor + 1, len);
        size_t spaceSeparator = version.find_first_of(' ', separatorMinorRelease + 1);
        if (spaceSeparator == string::npos)
            return false;
        len = spaceSeparator - (separatorMinorRelease + 1);
        release = version.substr(separatorMinorRelease + 1, len);
    }
    else {
        // second format
        size_t spaceSeparator = version.find_first_of(' ', separatorMajorMinor + 1);
        if (spaceSeparator == string::npos)
            minor = version.substr(separatorMajorMinor + 1);
        else {
            size_t len = spaceSeparator - (separatorMajorMinor + 1);
            minor = version.substr(separatorMajorMinor + 1, len);
        }
    }

    stream << major;
    int tmpMajor;
    stream >> tmpMajor;
    if (stream.fail())
        return false;

    stream.clear();
    stream << minor;
    int tmpMinor;
    stream >> tmpMinor;
    if (stream.fail())
        return false;

    int tmpRelease = 0;
    if (release != "") {
        stream.clear();
        stream << release;
        stream >> tmpRelease;
        if (stream.fail())
            return false;
    }

    _major = static_cast<unsigned char>(tmpMajor);
    _minor = static_cast<unsigned char>(tmpMinor);
    _release = static_cast<unsigned char>(tmpRelease);

    return true;
}

bool OpenGLCapabilitiesComponent::Version::operator==(const Version& rhs) const {
    return (_major == rhs._major) && (_minor == rhs._minor) && (_release == rhs._release);
}

bool OpenGLCapabilitiesComponent::Version::operator!=(const Version& rhs) const {
    return !(*this == rhs);
}

bool OpenGLCapabilitiesComponent::Version::operator<(const Version& rhs) const {
    const unsigned int numThis = packVersion(_major, _minor, _release);
    const unsigned int numRhs = packVersion(rhs._major, rhs._minor, rhs._release);

    return numThis < numRhs;
}

bool OpenGLCapabilitiesComponent::Version::operator<=(const Version& rhs) const {
    const unsigned int numThis = packVersion(_major, _minor, _release);
    const unsigned int numRhs = packVersion(rhs._major, rhs._minor, rhs._release);

    return numThis <= numRhs;
}

bool OpenGLCapabilitiesComponent::Version::operator>(const Version& rhs) const {
    const unsigned int numThis = packVersion(_major, _minor, _release);
    const unsigned int numRhs = packVersion(rhs._major, rhs._minor, rhs._release);

    return numThis > numRhs;
}

bool OpenGLCapabilitiesComponent::Version::operator>=(const Version& rhs) const {
    const unsigned int numThis = packVersion(_major, _minor, _release);
    const unsigned int numRhs = packVersion(rhs._major, rhs._minor, rhs._release);

    return numThis >= numRhs;
}

std::string OpenGLCapabilitiesComponent::Version::toString() const {
    stringstream stream;
    stream << static_cast<int>(_major) << "." << static_cast<int>(_minor);
    if (_release != 0)
        stream << "." << static_cast<int>(_release);
    return stream.str();
}

} // namespace ghoul
} // namespace systemcapabilities
