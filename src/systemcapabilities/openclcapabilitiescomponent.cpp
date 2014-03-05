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
#include <ghoul/opencl/device.h>
#include <ghoul/opencl/platform.h>
#include <ghoul/opencl/cl.hpp>


#include <algorithm>
#include <cassert>
#include <sstream>
#include <vector>
#include <type_traits>
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
    
template<class T>
std::string datatostring(T data) {
    std::stringstream ss;
    ss << data;
    return ss.str();
}
template<bool>
std::string datatostring(bool data) {
    if (data) {
        return "true";
    }
    return "false";
}
template<cl_ulong>
std::string datatostring(cl_ulong data) {
    std::stringstream ss;
    ss << data;
    return ss.str();
}
template<cl_uint>
std::string datatostring(cl_uint data) {
    std::stringstream ss;
    ss << data;
    return ss.str();
}






template<cl_device_id>
std::string datatostring(cl_device_id data) {
    std::stringstream ss;
    ss << data;
    return ss.str();
}
template<cl_platform_id>
std::string datatostring(cl_platform_id data) {
    std::stringstream ss;
    ss << data;
    return ss.str();
}



void OpenCLCapabilitiesComponent::detectCapabilities() {
    clearCapabilities();
    
    std::vector<cl::Platform> platforms;
    if(cl::Platform::get(&platforms) != CL_SUCCESS)
        return;
    
    for (auto platform: platforms) {
        ghoul::opencl::Platform* gPlatform = new ghoul::opencl::Platform(&platform);
        std::vector<ghoul::opencl::Device*> gDevices;
        
        gPlatform->fetchInformation();
        
        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
        
        for(auto device: devices) {
            ghoul::opencl::Device* gDevice = new ghoul::opencl::Device(&device);
            gDevice->fetchInformation();
            gDevices.push_back(gDevice);
            
        }
        _data.push_back(PlatformAndDevices{gPlatform, gDevices});
    }
}

void OpenCLCapabilitiesComponent::clearCapabilities() {
    
    _data.erase(_data.begin(), _data.end());
}
    
std::vector<SystemCapabilitiesComponent::CapabilityInformation>
    OpenCLCapabilitiesComponent::capabilities(
        const SystemCapabilitiesComponent::Verbosity& verbosity) const
{
    std::vector<SystemCapabilitiesComponent::CapabilityInformation> result;
    std::stringstream ss;
    
    if (verbosity >= Verbosity::Default) {
        for (size_t i = 0; i < _data.size(); ++i) {
            ss <<"Platform[" << i << "] Name";
            ghoul::opencl::Platform* p  = _data.at(i).platform;
            
            result.push_back(std::make_pair(ss.str() , p->name()));
            for (size_t j = 0; j < _data.at(i).devices.size(); ++j) {
                ghoul::opencl::Device* d  = _data.at(i).devices.at(j);
                ss.str("");
                ss <<"    Device[" << j << "] Name";
                result.push_back(std::make_pair(ss.str() , d->name()));
                result.push_back(std::make_pair("        Vendor" , d->vendor()));
                result.push_back(std::make_pair("        Version" , d->version()));
                result.push_back(std::make_pair("        Max Compute Units" , datatostring(d->maxComputeUnits())));
                result.push_back(std::make_pair("        Max Samplers" , datatostring(d->maxSamplers())));
                result.push_back(std::make_pair("        Max Work Group Size" , datatostring(d->maxWorkGroupSize())));
                result.push_back(std::make_pair("        Global Memory Size" , datatostring(d->globalMemSize())));
                result.push_back(std::make_pair("        Local Memory Type" , datatostring(d->localMemType())));
                result.push_back(std::make_pair("        Local Memory Size" , datatostring(d->localMemSize())));
                if (verbosity >= Verbosity::Full) {
                    result.push_back(std::make_pair("        Build in kernels" , d->builtInKernels()));
                    result.push_back(std::make_pair("        Extensions" , d->extensions()));
                }
            }
        }
    }
    return result;
}
    
const std::string OpenCLCapabilitiesComponent::name() const {
    return "OpenCL";
}

} // namespace ghoul
} // namespace systemcapabilities
