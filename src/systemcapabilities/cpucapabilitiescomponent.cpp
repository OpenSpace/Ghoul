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

#include "systemcapabilities/cpucapabilitiescomponent.h"

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

    // This is for the 'warning C4996: 'GetVersionExA': was declared deprecated' which
    // is a known bug for VS2013
    #pragma warning(disable: 4996)
#else
    #include <sys/utsname.h>
#endif

namespace {
    const std::string _loggerCat = "CPUCapabilitiesComponents";
}

namespace ghoul {
namespace systemcapabilities {

CPUCapabilitiesComponent::CPUCapabilitiesComponent()
    : SystemCapabilitiesComponent()
{
    clearCapabilities();
}

CPUCapabilitiesComponent::~CPUCapabilitiesComponent() {
    deinitialize();
}

void CPUCapabilitiesComponent::detectCapabilities() {
    clearCapabilities();
    detectOS();
    detectMemory();
}

void CPUCapabilitiesComponent::clearCapabilities() {
    _operatingSystem = "";
    _installedMainMemory = 0;
}

void CPUCapabilitiesComponent::detectOS() {
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
        LERROR("Retrieving OS version failed. 'GetVersionEx' returned 0.");
        LERROR("Last Error: " << GetLastError());
        return;
    }
    HMODULE module = GetModuleHandle(TEXT("kernel32.dll"));
    if (module == 0) {
        LERROR("Kernel32.dll handle could not be found. 'GetModuleHandle' returned 0.");
        LERROR("Last Error: " << GetLastError());
        return;
    }
    PGNSI procedureGetNativeSystemInfo = (PGNSI) GetProcAddress(
        module,
        "GetNativeSystemInfo");
    if (procedureGetNativeSystemInfo != 0)
        procedureGetNativeSystemInfo(&systemInfo);
    else
        GetSystemInfo(&systemInfo);

    std::stringstream resultStream;
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
        LERROR("OS detection failed. 'uname' returned non-null value");
        return;
    }
    std::stringstream resultStream;
    resultStream << name.sysname << " " << name.release << " "
        << name.version << " " << name.machine;

    _operatingSystem = resultStream.str();
#endif
}

void CPUCapabilitiesComponent::detectMemory() {
#ifdef WIN32
    std::string memory;
    bool success = queryWMI("Win32_ComputerSystem", "TotalPhysicalMemory", memory);
    if (!success)
        LERROR("Reading of main RAM failed.");
    else {
        std::stringstream convert;
        convert << memory;
        unsigned long long value;
        convert >> value;
        _installedMainMemory = static_cast<unsigned int>((value / 1024) / 1024);
    }
#endif
}

std::vector<SystemCapabilitiesComponent::CapabilityInformation>
    CPUCapabilitiesComponent::capabilities(
                        const SystemCapabilitiesComponent::Verbosity& /*verbosity*/) const
{
    std::vector<SystemCapabilitiesComponent::CapabilityInformation> result;
    result.emplace_back("Operating System", _operatingSystem);
    result.emplace_back("Main Memory", installedMainMemoryAsString());
    return result;
}

const std::string& CPUCapabilitiesComponent::operatingSystem() const {
    return _operatingSystem;
}

unsigned int CPUCapabilitiesComponent::installedMainMemory() const {
    return _installedMainMemory;
}

std::string CPUCapabilitiesComponent::installedMainMemoryAsString() const {
    std::stringstream s;
    s << _installedMainMemory << " MB";
    return s.str();
}

std::string CPUCapabilitiesComponent::name() const {
    return "CPU";
}

} // namespace systemcapabilities
} // namespace ghoul
