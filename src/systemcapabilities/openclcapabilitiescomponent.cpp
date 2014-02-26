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

#include "systemcapabilities/openclcapabilitiescomponent.h"

#include <ghoul/opencl/ghoul_cl.h>
#include <algorithm>
#include <cassert>
#include <sstream>
#include "logging/logmanager.h"

#define MAX_NAME_LEN 1000

namespace {
    const std::string _loggerCat = "OpenCLCapabilities";

    template <class T>
    std::string toString(T i) {
        std::stringstream s;
        s << i;
        return s.str();
    }
}

namespace ghoul {
namespace systemcapabilities {

OpenCLCapabilitiesComponent::OpenCLCapabilitiesComponent()
    : SystemCapabilitiesComponent()
{
    clearCapabilities();
}

OpenCLCapabilitiesComponent::~OpenCLCapabilitiesComponent() {
    deinitialize();
}

void OpenCLCapabilitiesComponent::detectCapabilities() {
    clearCapabilities();
    
    // get number of platforms
    cl_uint plat_count;
    cl_int status_code;
    
    status_code = clGetPlatformIDs(0, NULL, &plat_count);
    
    // allocate memory, get list of platforms
    cl_platform_id *platforms = new cl_platform_id[plat_count];
    
    status_code =clGetPlatformIDs(plat_count, platforms, NULL);
    
    // iterate over platforms
    for (cl_uint i = 0; i < plat_count; ++i) {
        // get platform vendor name
        char buf[MAX_NAME_LEN];
        status_code =clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(buf), buf, NULL);
        LDEBUG("platform " << i << ": vendor '" << buf << "'");
        
        // get number of devices in platform
        cl_uint dev_count;
        status_code = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &dev_count);
        cl_device_id *devices = new cl_device_id[dev_count];
        
        // get list of devices in platform
        status_code =clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL,
                                    dev_count, devices, NULL);
        // iterate over devices
        for (cl_uint j = 0; j < dev_count; ++j) {
            char buf[MAX_NAME_LEN];
            status_code = clGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(buf), buf, NULL);
            LDEBUG("  device " << j << ": '" << buf << "'");
        }
        
        delete[] devices;
    }
    
    delete[] platforms;
}

void OpenCLCapabilitiesComponent::clearCapabilities() {
    _clVersion._major = 0;
    _clVersion._minor = 0;
    _clVersion._release =0;
}
    
std::vector<SystemCapabilitiesComponent::CapabilityInformation>
    OpenCLCapabilitiesComponent::capabilities(
        const SystemCapabilitiesComponent::Verbosity& verbosity) const
{
    std::vector<SystemCapabilitiesComponent::CapabilityInformation> result;
    return result;
}
    
const std::string OpenCLCapabilitiesComponent::name() const {
    return "OpenGL";
}

    
/////////////////////////////
/// OpenCLVersion
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
    
OpenCLCapabilitiesComponent::Version::Version(int major, int minor, int release)
    : _major(static_cast<unsigned char>(major))
    , _minor(static_cast<unsigned char>(minor))
    , _release(static_cast<unsigned char>(release))
{}

bool OpenCLCapabilitiesComponent::Version::operator==(const Version& rhs) const {
    return (_major == rhs._major) && (_minor == rhs._minor) && (_release == rhs._release);
}

bool OpenCLCapabilitiesComponent::Version::operator!=(const Version& rhs) const {
    return !(*this == rhs);
}

bool OpenCLCapabilitiesComponent::Version::operator<(const Version& rhs) const {
    const unsigned int numThis = packVersion(_major, _minor, _release);
    const unsigned int numRhs = packVersion(rhs._major, rhs._minor, rhs._release);

    return numThis < numRhs;
}

bool OpenCLCapabilitiesComponent::Version::operator<=(const Version& rhs) const {
    const unsigned int numThis = packVersion(_major, _minor, _release);
    const unsigned int numRhs = packVersion(rhs._major, rhs._minor, rhs._release);

    return numThis <= numRhs;
}

bool OpenCLCapabilitiesComponent::Version::operator>(const Version& rhs) const {
    const unsigned int numThis = packVersion(_major, _minor, _release);
    const unsigned int numRhs = packVersion(rhs._major, rhs._minor, rhs._release);

    return numThis > numRhs;
}

bool OpenCLCapabilitiesComponent::Version::operator>=(const Version& rhs) const {
    const unsigned int numThis = packVersion(_major, _minor, _release);
    const unsigned int numRhs = packVersion(rhs._major, rhs._minor, rhs._release);

    return numThis >= numRhs;
}

std::string OpenCLCapabilitiesComponent::Version::toString() const {
    std::stringstream stream;
    stream << static_cast<int>(_major) << "." << static_cast<int>(_minor);
    if (_release != 0)
        stream << "." << static_cast<int>(_release);
    return stream.str();
}

} // namespace ghoul
} // namespace systemcapabilities
