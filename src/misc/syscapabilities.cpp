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

#include "misc/syscapabilities.h"

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

    //#ifdef GHOUL_USE_WMI

    //    #include <WinBase.h>
    //#endif
#else
    #include <sys/utsname.h>
#endif

using std::string;
using std::stringstream;
using std::vector;
using std::wstring;

namespace {
    const std::string _loggerCat = "SystemCapabilities";
}

namespace ghoul {

SystemCapabilities* SystemCapabilities::_systemCapabilities = nullptr;

SystemCapabilities::SystemCapabilities() 
#ifdef GHOUL_USE_WMI
    : _iwbemLocator(nullptr)
    , _iwbemServices(nullptr)
#endif
{
    initializeWMI();
    detectCapabilities();
}

SystemCapabilities::~SystemCapabilities() {
    deinitializeWMI();
}

void SystemCapabilities::init() {
    if (_systemCapabilities == nullptr)
        _systemCapabilities = new SystemCapabilities;
}

void SystemCapabilities::deinit() {
    delete _systemCapabilities;
    _systemCapabilities = nullptr;
}

bool SystemCapabilities::isInitialized() {
    return (_systemCapabilities != nullptr);
}

SystemCapabilities& SystemCapabilities::ref() {
    assert(_systemCapabilities != nullptr);
    return *_systemCapabilities;
}

SystemCapabilities::SystemCapabilities(const SystemCapabilities&) {}

void SystemCapabilities::clearCapabilities() {
    _operatingSystem = "";
    _installedMainMemory = 0;

    _glslVersion._major = 0;
    _glslVersion._minor = 0;
    _glslVersion._release = 0;
    _glslCompiler.clear();
    _vendor = VendorOther;
    _glRenderer = "";
    _extensions.clear();
    _glewVersion._major = 0;
    _glewVersion._minor = 0;
    _glewVersion._release = 0;

    _maxTextureSize = -1;
    _maxTextureSize3D = -1;
    _numTextureUnits = -1;

    _supportNVHalfFloat = false;
    _supportDebugOutput = false;
    _supportDepthBufferFloat = false;
    _supportFrameBufferObject = false;
    _supportShadingInclude = false;
    _supportTessellationShader = false;
    _supportTimerQuery = false;
    _supportNVXGpuMemoryInfo = false;
    _supportNVCopyImage = false;
    _supportNPoTTextures = false;
    _supportTexturing3D = false;
    _supportTextureRectangles = false;
    _supportTextureFilterAnisotropic = false;

#ifdef GHOUL_USE_WMI
    _driverVersion = "";
    _driverDate = "";
    _adapterRAM = 0;
    _adapterName = "";
#endif
}

void SystemCapabilities::detectCapabilities() {
    clearCapabilities();

    // GLSL Version
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

    // GPU Vendor
    const char* vendor =
        reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    if (vendor)
        _glslCompiler = string(vendor);
    else
        LERROR_SAFE("Detection the GPU Vendor failed. 'glGetString' returned 0.");

    if (_glslCompiler.find("NVIDIA") != string::npos)
        _vendor = VendorNvidia;
    else if (_glslCompiler.find("ATI") != string::npos)
        _vendor = VendorATI;
    else if ((_glslCompiler.find("INTEL") != string::npos) 
        || (_glslCompiler.find("Intel") != string::npos))
    {
        _vendor = VendorIntel;
    }
    else {
        LINFO_SAFE("Vendor of graphics card is not in the enum 'Vendor'. Vendor information: "
            << _glslCompiler);
        _vendor = VendorOther;
    }
    
    _glRenderer = string(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    // Extensions
    GLint nExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &nExtensions);
    for (GLint i = 0; i < nExtensions; ++i) {
        const GLubyte* ext = glGetStringi(GL_EXTENSIONS, i);
        const string extension = string(reinterpret_cast<const char*>(ext));
        _extensions.push_back(extension);
    }
    
    // GLEW Version
    _glewVersion._major = GLEW_VERSION_MAJOR;
    _glewVersion._minor = GLEW_VERSION_MINOR;
    _glewVersion._release = GLEW_VERSION_MICRO;

    // Extensions
    if (glewIsSupported("GL_NV_half_float"))
        _supportNVHalfFloat = true;
    if (glewIsSupported("GL_ARB_debug_output"))
        _supportDebugOutput = true;
    if (glewIsSupported("GL_ARB_depth_buffer_float"))
        _supportDepthBufferFloat = true;
    if (glewIsSupported("GL_ARB_framebuffer_object"))
        _supportFrameBufferObject = true;
    if (glewIsSupported("GL_ARB_shading_language_include"))
        _supportShadingInclude = true;
    if (glewIsSupported("GL_ARB_tessellation_shader"))
        _supportTessellationShader = true;
    if (glewIsSupported("GL_ARB_timer_query"))
        _supportTimerQuery = true;
    if (glewIsSupported("GL_NVX_gpu_memory_info"))
        _supportNVXGpuMemoryInfo = true;
    if (glewIsSupported("GL_NV_copy_image"))
        _supportNVCopyImage = true;
    if (glewIsSupported("GL_ARB_texture_non_power_of_two"))
        _supportNPoTTextures = true;
    if (glewIsSupported("GL_ARB_texture_rectangle"))
        _supportTextureRectangles = true;
    if (glewIsSupported("GL_EXT_texture_filter_anisotropic"))
        _supportTextureFilterAnisotropic = true;

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

    if (_supportFrameBufferObject)
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &_maxFramebufferColorAttachments);

    detectOS();

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

    string memory;
    bool mainMemorySuccess = queryWMI("Win32_ComputerSystem", "TotalPhysicalMemory", memory);
    if (!mainMemorySuccess)
        LERROR_SAFE("Reading of main RAM failed.");
    else {
        stringstream convert;
        convert << memory;
        unsigned long long value;
        convert >> value;
        _installedMainMemory = static_cast<unsigned int>((value / 1024) / 1024);

    }
#endif
}

void SystemCapabilities::logCapabilities() const {
    LINFO_SAFE("Operating System:      " << _operatingSystem);
    LINFO_SAFE("Main Memory:           " << _installedMainMemory << " MB");
    LINFO_SAFE("OpenGL Version:        " << _glslVersion.toString());
    LINFO_SAFE("OpenGL Compiler:       " << _glslCompiler);
    LINFO_SAFE("OpenGL Renderer:       " << _glRenderer);
    LINFO_SAFE("GLEW Version:          " << _glewVersion.toString());
#ifdef GHOUL_USE_WMI
    LINFO_SAFE("GPU Name:              " << _adapterName);
    LINFO_SAFE("GPU Driver Version:    " << _driverVersion);
    LINFO_SAFE("GPU Driver Date:       " << _driverDate);
    LINFO_SAFE("GPU RAM:               " << _adapterRAM << " MB");
#endif
    LINFO_SAFE("Max Texture Size:      " << _maxTextureSize);
    LINFO_SAFE("Max 3D Texture Size:   " << _maxTextureSize3D);
    LINFO_SAFE("Num of Texture Units:  " << _numTextureUnits);
    LINFO_SAFE("FBO Color Attachments: " << _maxFramebufferColorAttachments);
    stringstream features;
    features << "Features: ";
    if (_supportNVHalfFloat)
        features << "GL_NV_half_float ";
    if (_supportDebugOutput)
        features << "GL_ARB_debug_output ";
    if (_supportDepthBufferFloat)
        features << "GL_ARB_depth_buffer_float ";
    if (_supportFrameBufferObject)
        features << "GL_ARB_framebuffer_object ";
    if (_supportShadingInclude)
        features << "GL_ARB_shading_language_include ";
    if (_supportTessellationShader)
        features << "GL_ARB_tessellation_shader ";
    if (_supportTimerQuery)
        features << "GL_ARB_timer_query ";
    if (_supportNVXGpuMemoryInfo)
        features << "GL_NVX_gpu_memory_info ";
    if (_supportNVCopyImage)
        features << "GL_NV_copy_image ";
    if (_supportNPoTTextures)
        features << "GL_ARB_texture_non_power_of_two ";
    if (_supportTexturing3D)
        features << "GL_EXT_texture3D ";
    if (_supportTextureRectangles)
        features << "GL_ARB_texture_rectangle ";
    if (_supportTextureFilterAnisotropic)
        features << "GL_EXT_texture_filter_anisotropic ";
    LINFO_SAFE(features.str());
}


void SystemCapabilities::detectOS() {
#ifdef WIN32
    // This code is taken from
    // http://msdn.microsoft.com/en-us/library/windows/desktop/ms724429%28v=vs.85%29.aspx
    // All rights remain with their original copyright owners
    OSVERSIONINFOEX osVersionInfo;
    SYSTEM_INFO systemInfo;
    
    ZeroMemory(&systemInfo, sizeof(SYSTEM_INFO));
    ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    BOOL osVersionInfoEx = GetVersionEx((OSVERSIONINFO*) &osVersionInfo);

    if (osVersionInfoEx == 0) {
        LERROR_SAFE("Retrieving OS version failed. 'GetVersionEx' returned 0.");
        LERROR_SAFE("Last Error: " << GetLastError());
        return;
    }
    HMODULE module = GetModuleHandle(TEXT("kernel32.dll"));
    if (module == 0) {
        LERROR_SAFE("Kernel32.dll handle could not be found. 'GetModuleHandle' returned 0.");
        LERROR_SAFE("Last Error: " << GetLastError());
        return;
    }
    PGNSI procedureGetNativeSystemInfo = (PGNSI) GetProcAddress(
        GetModuleHandle(TEXT("kernel32.dll")),
        "GetNativeSystemInfo");
    if (procedureGetNativeSystemInfo != 0)
        procedureGetNativeSystemInfo(&systemInfo);
    else
        GetSystemInfo(&systemInfo);

    stringstream resultStream;
    if (osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT && osVersionInfo.dwMajorVersion > 4) {
        resultStream << "Microsoft ";
        if (osVersionInfo.dwMajorVersion == 6) {
            if (osVersionInfo.dwMinorVersion == 0) {
                if (osVersionInfo.wProductType == VER_NT_WORKSTATION)
                    resultStream << "Windows Vista ";
                else
                    resultStream << "Windows Server 2008 ";
            }
            else if (osVersionInfo.dwMinorVersion == 1) {
                if (osVersionInfo.wProductType == VER_NT_WORKSTATION)
                    resultStream << "Windows 7 ";
                else
                    resultStream << "Windows Server 2008 R2 ";
            }
            else if (osVersionInfo.dwMinorVersion == 2) {
                if (osVersionInfo.wProductType == VER_NT_WORKSTATION)
                    resultStream << "Windows 8 ";
                else
                    resultStream << "Windows Server 2012 ";
            }
        }
        else if (osVersionInfo.dwMajorVersion == 5 && osVersionInfo.dwMinorVersion == 2) {
            if (GetSystemMetrics(SM_SERVERR2))
                resultStream << "Windows Server 2003 R2";
            else if (osVersionInfo.wSuiteMask & VER_SUITE_STORAGE_SERVER)
                resultStream << "Windows Storage Server 2003";
            else if (osVersionInfo.wProductType == VER_NT_WORKSTATION &&
                systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
                resultStream << "Windows XP Professional x64 Edition";
            else
                resultStream << "Windows Server 2003";
        }
        else if (osVersionInfo.dwMajorVersion == 5 && osVersionInfo.dwMinorVersion == 1) {
            resultStream << "Windows XP ";
            if (osVersionInfo.wSuiteMask & VER_SUITE_PERSONAL)
                resultStream << "Home Edition";
            else
                resultStream << "Professional";
        }
        else if (osVersionInfo.dwMajorVersion == 5 && osVersionInfo.dwMinorVersion == 0) {
            resultStream << "Windows 2000 ";
            if (osVersionInfo.wProductType == VER_NT_WORKSTATION)
                resultStream << "Professional";
            else {
                if (osVersionInfo.wSuiteMask & VER_SUITE_DATACENTER)
                    resultStream << "Datacenter Server";
                else if (osVersionInfo.wSuiteMask & VER_SUITE_ENTERPRISE)
                    resultStream << "Advanced Server";
                else
                    resultStream << "Server";
            }
        }

        if (_tcslen(osVersionInfo.szCSDVersion) > 0)
            resultStream << " " << osVersionInfo.szCSDVersion;

        resultStream << " (build " << osVersionInfo.dwBuildNumber << ")";
        
        if (osVersionInfo.dwMajorVersion >= 6) {
            if (systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
                resultStream << ", 64-bit";
            else if (systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
                resultStream << ", 32-bit";
        }
    }
    else
        resultStream << "OS detection failed. Version of Windows is too old.";

    _operatingSystem = resultStream.str();
#else
    utsname name;
    if (uname(&name) != 0) {
        LERROR_SAFE("OS detection failed. 'uname' returned non-null value");
        return;
    }
    stringstream resultStream;
    resultStream << name.sysname << " " << name.release << " "
        << name.version << " " << name.machine;

    _operatingSystem = resultStream.str();
#endif
}

const SystemCapabilities::Version& SystemCapabilities::openGLVersion() const {
    return _glslVersion;
}

const string& SystemCapabilities::gpuVendorString() const {
    return _glslCompiler;
}

const SystemCapabilities::Vendor& SystemCapabilities::gpuVendor() const {
    return _vendor;
}

const vector<string>& SystemCapabilities::extensions() const {
    return _extensions;
}

bool SystemCapabilities::isExtensionSupported(const string& extension) const {
    vector<string>::const_iterator result =
        std::find(_extensions.begin(), _extensions.end(), extension);
    return (result != _extensions.end());
}

/////////////////////////////
/// Windows Management Instrumentation
/////////////////////////////

void SystemCapabilities::initializeWMI() {
#ifdef GHOUL_USE_WMI
    const string _loggerCat = "SystemCapabilities.WMI";
    // This code is based on
    // http://msdn.microsoft.com/en-us/library/aa390423.aspx
    // All rights remain with their original copyright owners

    if (isWMIinitialized()) {
        LWARNING_SAFE("The WMI initialization has already been initialized.");
        return;
    }

    HRESULT hRes = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    if (FAILED(hRes)) {
        LERROR_SAFE("WMI initialization failed. 'CoInitializeEx' failed." 
            << " Error Code: 0x" << hRes);
        return;
    }
    hRes = CoInitializeSecurity(
        NULL, 
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
        );
    if (FAILED(hRes))
        LDEBUG_SAFE("CoInitializeSecurity failed with error code: 0x" << hRes);
    else
        LDEBUG_SAFE("CoInitializeSecurity successful.");

    hRes = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        reinterpret_cast<LPVOID*>(&_iwbemLocator));
    if (FAILED(hRes)) {
        LERROR_SAFE("WMI initialization failed. Failed to create IWbemLocator object." 
            << " Error Code: 0x" << hRes);
        _iwbemLocator = 0;
        CoUninitialize();
        return;
    }

    LDEBUG_SAFE("IWbemLocator object successfully created.");

    hRes = _iwbemLocator->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
        NULL,                    // User name. NULL = current user
        NULL,                    // User password. NULL = current
        0,                       // Locale. NULL indicates current
        NULL,                    // Security flags.
        0,                       // Authority (for example, Kerberos)
        0,                       // Context object 
        &_iwbemServices          // pointer to IWbemServices proxy
        );
    if (FAILED(hRes)) {
        LERROR_SAFE("WMI initialization failed. Failed to connect to WMI server."
            << " Error Code: 0x" << hRes);
        _iwbemLocator->Release();
        CoUninitialize();
        _iwbemLocator = 0;
        _iwbemServices = 0;
        return;
    }

    LDEBUG_SAFE("Connected to ROOT\\CIMV2 WMI namespace.");

    hRes = CoSetProxyBlanket(
        _iwbemServices,              // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name 
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities 
        );
    if (FAILED(hRes)) {
        LERROR_SAFE("WMI initialization failed. Could not set proxy blanket. Error Code: 0x"
            << hRes);
        _iwbemServices->Release();
        _iwbemServices = 0;
        _iwbemLocator->Release();
        _iwbemLocator = 0;
        CoUninitialize();
        return;
    }
    LDEBUG_SAFE("WMI successfully initialized.");
#endif
}

void SystemCapabilities::deinitializeWMI() {
#ifdef GHOUL_USE_WMI
    if (!isWMIinitialized()) {
        LWARNING_SAFE("WMI is not initialized.");
        return;
    }

    LDEBUG_SAFE("Deinitializing WMI.");
    if (_iwbemLocator)
        _iwbemLocator->Release();
    _iwbemLocator = 0;
    if (_iwbemServices)
        _iwbemServices->Release();
    _iwbemServices = 0;

    CoUninitialize();
#endif
}

#ifdef GHOUL_USE_WMI
wstring str2wstr(const string& str) {
    int stringLength = static_cast<int>(str.length() + 1);
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), stringLength, 0, 0); 
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), stringLength, buf, len);
    wstring r(buf);
    delete[] buf;
    return r;
}

std::string wstr2str(const wstring& wstr) {
    int stringLength = static_cast<int>(wstr.length() + 1);
    int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), stringLength, 0, 0, 0, 0); 
    char* buf = new char[len];
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), stringLength, buf, len, 0, 0);
    string r(buf);
    delete[] buf;
    return r;
}

bool SystemCapabilities::isWMIinitialized() const {
    return ((_iwbemLocator != 0) && (_iwbemServices != 0));
}

VARIANT* SystemCapabilities::queryWMI(const std::string& wmiClass,
                                      const std::string& attribute) const
{
    if (!isWMIinitialized()) {
        LERROR_SAFE("WMI is not initialized.");
        return 0;
    }

    VARIANT* result = 0;
    IEnumWbemClassObject* enumerator = 0;
    string query = "SELECT " + attribute + " FROM " + wmiClass;
    HRESULT hRes = _iwbemServices->ExecQuery(
        bstr_t("WQL"),
        bstr_t(query.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
        NULL,
        &enumerator);
    if (FAILED(hRes)) {
        LERROR_SAFE("WMI query failed. Error Code: 0x" << hRes);
        return 0;
    }

    IWbemClassObject* pclsObject = 0;
    ULONG returnValue;
    if (enumerator) {
        HRESULT hr = enumerator->Next(
            WBEM_INFINITE,
            1,
            &pclsObject,
            &returnValue);
        if (returnValue) {
            result = new VARIANT;
            hr = pclsObject->Get(
                LPCWSTR(str2wstr(attribute).c_str()),
                0,
                result,
                0,
                0);
        }
    }

    if (!result)
        LINFO_SAFE("No WMI query result.");

    if (enumerator)
        enumerator->Release();
    if (pclsObject)
        pclsObject->Release();

    return result;
}

bool SystemCapabilities::queryWMI(
                                  const std::string& wmiClass,
                                  const std::string& attribute,
                                  std::string& value) const 
{
    VARIANT* variant = queryWMI(wmiClass, attribute);
    if (!variant)
        return false;
    else {
        value = wstr2str(wstring(variant->bstrVal));
        VariantClear(variant);
        return true;
    }
}

bool SystemCapabilities::queryWMI(
                                     const std::string& wmiClass,
                                     const std::string& attribute,
                                     int& value) const
{
    VARIANT* variant = queryWMI(wmiClass, attribute);
    if (!variant)
        return false;
    else {
        value = variant->intVal;
        VariantClear(variant);
        return true;
    }
}

bool SystemCapabilities::queryWMI(const std::string& wmiClass, const std::string& attribute,
              unsigned int& value) const
{
    VARIANT* variant = queryWMI(wmiClass, attribute);
    if (!variant)
        return false;
    else {
        value = variant->uintVal;
        VariantClear(variant);
        return true;
    }
}

bool SystemCapabilities::queryWMI(const std::string& wmiClass, const std::string& attribute,
                                  unsigned long long& value) const
{
    VARIANT* variant = queryWMI(wmiClass, attribute);
    if (!variant)
        return false;
    else {
        value = variant->ullVal;
        VariantClear(variant);
        return true;
    }
}

#endif

int SystemCapabilities::maximumNumberOfTextureUnits() const {
    return _numTextureUnits;
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

SystemCapabilities::Version::Version(int major, int minor, int release) 
    : _major(static_cast<unsigned char>(major))
    , _minor(static_cast<unsigned char>(minor))
    , _release(static_cast<unsigned char>(release))
{}

bool SystemCapabilities::Version::parseGLSLString(const string& version) {
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

bool SystemCapabilities::Version::operator==(const Version& rhs) const {
    return (_major == rhs._major) && (_minor == rhs._minor) && (_release == rhs._release);
}

bool SystemCapabilities::Version::operator!=(const Version& rhs) const {
    return !(*this == rhs);
}

bool SystemCapabilities::Version::operator<(const Version& rhs) const {
    const unsigned int numThis = packVersion(_major, _minor, _release);
    const unsigned int numRhs = packVersion(rhs._major, rhs._minor, rhs._release);

    return numThis < numRhs;
}

bool SystemCapabilities::Version::operator<=(const Version& rhs) const {
    const unsigned int numThis = packVersion(_major, _minor, _release);
    const unsigned int numRhs = packVersion(rhs._major, rhs._minor, rhs._release);

    return numThis <= numRhs;
}

bool SystemCapabilities::Version::operator>(const Version& rhs) const {
    const unsigned int numThis = packVersion(_major, _minor, _release);
    const unsigned int numRhs = packVersion(rhs._major, rhs._minor, rhs._release);

    return numThis > numRhs;
}

bool SystemCapabilities::Version::operator>=(const Version& rhs) const {
    const unsigned int numThis = packVersion(_major, _minor, _release);
    const unsigned int numRhs = packVersion(rhs._major, rhs._minor, rhs._release);

    return numThis >= numRhs;
}

std::string SystemCapabilities::Version::toString() const {
    stringstream stream;
    stream << static_cast<int>(_major) << "." << static_cast<int>(_minor);
    if (_release != 0)
        stream << "." << static_cast<int>(_release);
    return stream.str();
}


} // namespace ghoul
