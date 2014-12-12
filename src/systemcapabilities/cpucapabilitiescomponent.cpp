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

    // These is for the 'warning C4996: 'GetVersionExA': was declared deprecated' which
    // is a known bug for VS2013
	#pragma warning(push)
    #pragma warning(disable: 4996)
	#pragma warning(suppress: 28159)

#else
    #ifdef __APPLE__
        #include <sys/sysctl.h>
    #else
        #include <sys/types.h>
        #include <sys/sysinfo.h>
        #include <cstring>
    #endif

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
	detectCPU();
}

void CPUCapabilitiesComponent::clearCapabilities() {
    _operatingSystem = "";
    _installedMainMemory = 0;
	_cpu = "";
	_cores = 0;
	_cacheLineSize = 0;
	_L2Associativity = 0;
	_cacheSize = 0;
	_extensions = "";
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
#if defined(WIN32)
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
#elif defined(__APPLE__)
    
    int mib[2];
    size_t len;
    char *p;
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    sysctl(mib, 2, NULL, &len, NULL, 0);
    p = new char[len];
    sysctl(mib, 2, p, &len, NULL, 0);
    
    int64_t value;
    std::memcpy(&value, p, sizeof(int64_t));
    _installedMainMemory = static_cast<unsigned int>((value / 1024) / 1024);
    delete[] p;
#else
    struct sysinfo memInfo;
    sysinfo (&memInfo);
    _installedMainMemory = static_cast<unsigned int>((memInfo.totalram / 1024) / 1024);
#endif
}

void CPUCapabilitiesComponent::detectCPU() {
#ifdef WIN32
	const char* szFeatures[] =
	{
		"fpu",
		"vme",
		"de",
		"pse",
		"tsc",
		"msr",
		"pae",
		"mce",
		"cx8",
		"apic",
		"Unknown1",
		"sep",
		"mtrr",
		"pge",
		"mca",
		"cmov",
		"pat",
		"pse36",
		"psn",
		"clflush",
		"Unknown2",
		"ds",
		"acpi", //  @TODO: "Thermal Monitor and Clock Ctrl", is this correct? ---jonasstrandstedt
		"mmx",
		"fxsr",
		"sse",
		"sse2",
		"ss",
		"ht",
		"tm",
		"Unknown4",
		"pbe"
	};

	char CPUString[0x20];
	char CPUBrandString[0x40];
	int CPUInfo[4] = { -1 };
	int nSteppingID = 0;
	int nModel = 0;
	int nFamily = 0;
	int nProcessorType = 0;
	int nExtendedmodel = 0;
	int nExtendedfamily = 0;
	int nBrandIndex = 0;
	int nCLFLUSHcachelinesize = 0;
	int nAPICPhysicalID = 0;
	int nFeatureInfo = 0;
	unsigned    nIds, nExIds, i;
	bool    bSSE3NewInstructions = false;
	bool    bMONITOR_MWAIT = false;
	bool    bCPLQualifiedDebugStore = false;
	bool    bThermalMonitor2 = false;


	// __cpuid with an InfoType argument of 0 returns the number of
	// valid Ids in CPUInfo[0] and the CPU identification string in
	// the other three array elements. The CPU identification string is
	// not in linear order. The code below arranges the information 
	// in a human readable form.
	__cpuid(CPUInfo, 0);
	nIds = CPUInfo[0];
	memset(CPUString, 0, sizeof(CPUString));
	*((int*)CPUString) = CPUInfo[1];
	*((int*)(CPUString + 4)) = CPUInfo[3];
	*((int*)(CPUString + 8)) = CPUInfo[2];

	// Get the information associated with each valid Id
	for (i = 0; i <= nIds; ++i)
	{
		__cpuid(CPUInfo, i);

		// Interpret CPU feature information.
		if (i == 1)
		{
			nSteppingID = CPUInfo[0] & 0xf;
			nModel = (CPUInfo[0] >> 4) & 0xf;
			nFamily = (CPUInfo[0] >> 8) & 0xf;
			nProcessorType = (CPUInfo[0] >> 12) & 0x3;
			nExtendedmodel = (CPUInfo[0] >> 16) & 0xf;
			nExtendedfamily = (CPUInfo[0] >> 20) & 0xff;
			nBrandIndex = CPUInfo[1] & 0xff;
			nCLFLUSHcachelinesize = ((CPUInfo[1] >> 8) & 0xff) * 8;
			nAPICPhysicalID = (CPUInfo[1] >> 24) & 0xff;
			bSSE3NewInstructions = (CPUInfo[2] & 0x1) || false;
			bMONITOR_MWAIT = (CPUInfo[2] & 0x8) || false;
			bCPLQualifiedDebugStore = (CPUInfo[2] & 0x10) || false;
			bThermalMonitor2 = (CPUInfo[2] & 0x100) || false;
			nFeatureInfo = CPUInfo[3];
		}
	}

	// Calling __cpuid with 0x80000000 as the InfoType argument
	// gets the number of valid extended IDs.
	__cpuid(CPUInfo, 0x80000000);
	nExIds = CPUInfo[0];
	memset(CPUBrandString, 0, sizeof(CPUBrandString));

	// Get the information associated with each extended ID.
	for (i = 0x80000000; i <= nExIds; ++i)
	{
		__cpuid(CPUInfo, i);

		// Interpret CPU brand string and cache information.
		if (i == 0x80000002)
			memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000003)
			memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000004)
			memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000006)
		{
			_cacheLineSize = CPUInfo[2] & 0xff;
			_L2Associativity = (CPUInfo[2] >> 12) & 0xf;
			_cacheSize = (CPUInfo[2] >> 16) & 0xffff;
		}
	}

	// Get extensions list
	std::stringstream extensions;
	if (nFeatureInfo || bSSE3NewInstructions ||
		bMONITOR_MWAIT || bCPLQualifiedDebugStore ||
		bThermalMonitor2)
	{

        if (bSSE3NewInstructions)
            extensions << "sse3 ";
        if (bMONITOR_MWAIT)
            extensions << "mwait "; // @TODO:  "MONITOR/MWAIT" is this correct? ---jonasstrandstedt
        if (bCPLQualifiedDebugStore)
            extensions << "ds_cpl ";
        if (bThermalMonitor2)
            extensions << "tm2 ";
		i = 0;
		nIds = 1;
		while (i < (sizeof(szFeatures) / sizeof(const char*)))
		{
			if (nFeatureInfo & nIds)
			{
				extensions << szFeatures[i] << " ";
			}

			nIds <<= 1;
			++i;
		}
	}

	// Set CPU name
	_cpu = CPUBrandString;

	// Set extensions and remove trailing ", "
	_extensions = extensions.str();
	if (_extensions.length() > 1)
		_extensions = _extensions.substr(0, _extensions.length()-1);

	// Get the cores
	SYSTEM_INFO systemInfo;
	GetNativeSystemInfo(&systemInfo);
	_cores = systemInfo.dwNumberOfProcessors;
#elif defined(__APPLE__)
    int mib[2];
    size_t len;
    char *p;
    int intValue;
    
    mib[0] = CTL_HW;
    mib[1] = USER_CS_PATH;
    mib[1] = HW_MODEL;
    mib[1] = HW_MACHINE_ARCH;
    sysctl(mib, 2, NULL, &len, NULL, 0);
    p = new char[len];
    sysctl(mib, 2, p, &len, NULL, 0);
    _cpu = p;
    delete[] p;
    
    // CPU name
    sysctlbyname( "machdep.cpu.brand_string", NULL, &len, NULL, 0 );
    p = new char[len];
    sysctlbyname( "machdep.cpu.brand_string", p, &len, NULL, 0 );
    _cpu = p;
    delete[] p;
    
    // CPU features/extensions
    sysctlbyname( "machdep.cpu.features", NULL, &len, NULL, 0 );
    p = new char[len];
    sysctlbyname( "machdep.cpu.features", p, &len, NULL, 0 );
    _extensions = p;
    delete[] p;
    
    // It works using reinterpret_cast<char*>(&intValue) directly
    // since the expected size is an integer. But to avoid risks
    // of memory corruption I have chosen the C-style detection
    // and conversion.
    
    // Number of cores
    mib[0] = CTL_HW;
    mib[1] = HW_AVAILCPU;
    sysctl(mib, 2, NULL, &len, NULL, 0);
    p = new char[len];
    sysctl(mib, 2, p, &len, NULL, 0);
    std::memcpy(&intValue, p, sizeof(int));
    _cores = static_cast<unsigned int>(intValue);
    delete[] p;
    
    // Cacheline size
    mib[0] = CTL_HW;
    mib[1] = HW_CACHELINE;
    sysctl(mib, 2, NULL, &len, NULL, 0);
    p = new char[len];
    sysctl(mib, 2, p, &len, NULL, 0);
    std::memcpy(&intValue, p, sizeof(int));
    _cacheLineSize = static_cast<unsigned int>(intValue);
    delete[] p;
    
    // Cache size
    mib[0] = CTL_HW;
    mib[1] = HW_L2CACHESIZE;
    sysctl(mib, 2, NULL, &len, NULL, 0);
    p = new char[len];
    sysctl(mib, 2, p, &len, NULL, 0);
    std::memcpy(&intValue, p, sizeof(int));
    _cacheSize = static_cast<unsigned int>(intValue);
    delete[] p;
    
    // L2 associativity
    sysctlbyname( "machdep.cpu.cache.L2_associativity", NULL, &len, NULL, 0 );
    p = new char[len];
    sysctlbyname( "machdep.cpu.cache.L2_associativity", p, &len, NULL, 0 );
    std::memcpy(&intValue, p, sizeof(int));
    _L2Associativity = static_cast<unsigned int>(intValue);
    delete[] p;
#else
    FILE* file;
    const unsigned int maxSize = 2048;
    char line[maxSize];

    // We must use c-style file opening because /proc is no ordinary filesystem
    file = fopen("/proc/cpuinfo", "r");
    if(file) {
        while(fgets(line, maxSize, file) != NULL){
            if (strncmp(line, "processor", 9) == 0) ++_cores;
            if (strncmp(line, "model name", 10) == 0) {
                _cpu = line;
                _cpu = _cpu.substr(18, _cpu.length()-19);
            }
            if (strncmp(line, "cache size", 10) == 0) {
                std::string tmp = line;
                tmp = tmp.substr(13, tmp.length() - 14);
                _cacheSize = static_cast<unsigned int>(strtol(tmp.c_str(), NULL, 0));

            }
            if (strncmp(line, "flags", 5) == 0) {
                _extensions = line;
                _extensions = _extensions.substr(9, _extensions.length() - 10);
            }
            memset(line, 0, maxSize);
        }
        fclose(file);
    }

    file = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
    if(file) {
        if(fgets(line, maxSize, file) != NULL){
            _cacheLineSize = static_cast<unsigned int>(strtol(line, NULL, 0));
        }
        fclose(file);
    }

    file = fopen("/sys/devices/system/cpu/cpu0/cache/index0/ways_of_associativity", "r");
    if(file) {
        if(fgets(line, maxSize, file) != NULL){
            _L2Associativity = static_cast<unsigned int>(strtol(line, NULL, 0));
        }
        fclose(file);
    }
#endif
}

std::vector<SystemCapabilitiesComponent::CapabilityInformation>
    CPUCapabilitiesComponent::capabilities(
                        const SystemCapabilitiesComponent::Verbosity& /*verbosity*/) const
{
    std::vector<SystemCapabilitiesComponent::CapabilityInformation> result;
	result.emplace_back("Operating System", _operatingSystem);
	result.emplace_back("CPU", _cpu);
	result.emplace_back("Cores", coresAsString());
	result.emplace_back("Cache line size", cacheLineSizeAsString());
	result.emplace_back("L2 Associativity", L2AssiciativityAsString());
	result.emplace_back("Cache size", cacheSizeAsString());
	result.emplace_back("Extensions", _extensions);
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

unsigned int CPUCapabilitiesComponent::cores() const {
	return _cores;
}

unsigned int CPUCapabilitiesComponent::cacheLineSize() const {
	return _cacheLineSize;
}

unsigned int CPUCapabilitiesComponent::L2Assiciativity() const {
	return _L2Associativity;
}

unsigned int CPUCapabilitiesComponent::cacheSize() const {
	return _cacheSize;
}

std::string CPUCapabilitiesComponent::coresAsString() const {
	std::stringstream s;
	s << _cores;
	return s.str();
}

std::string CPUCapabilitiesComponent::cacheLineSizeAsString() const {
	std::stringstream s;
	s << _cacheLineSize;
	return s.str();
}

std::string CPUCapabilitiesComponent::L2AssiciativityAsString() const {
	std::stringstream s;
	s << _L2Associativity;
	return s.str();
}

std::string CPUCapabilitiesComponent::cacheSizeAsString() const {
	std::stringstream s;
	s << _cacheSize << " K";
	return s.str();
}

std::string CPUCapabilitiesComponent::extensions() const {
	return _extensions;
}

std::string CPUCapabilitiesComponent::name() const {
    return "CPU";
}

} // namespace systemcapabilities
} // namespace ghoul

#ifdef WIN32
#pragma warning(pop)
#endif // WIN32
