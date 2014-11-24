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

#ifdef GHL_OPENCL
#include <ghoul/opencl/platform.h>
#include <ghoul/opencl/device.h>
#include <ghoul/opencl/ghoul_cl.h>
#include <ghoul/opencl/ghoul_cl.hpp>
#include <ghoul/opencl/device.h>
#include <ghoul/opencl/platform.h>
#endif

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
    
void OpenCLCapabilitiesComponent::detectCapabilities() {
    clearCapabilities();

#ifdef GHL_OPENCL
    
    std::vector<cl::Platform> platforms;
    if (cl::Platform::get(&platforms) != CL_SUCCESS)
        return;
    
    for (cl::Platform platform : platforms) {
        ghoul::opencl::Platform* gPlatform = new ghoul::opencl::Platform(&platform);
        std::vector<ghoul::opencl::Device*> gDevices;
        
        gPlatform->fetchInformation();
        
        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
        
        for (cl::Device device : devices) {
            ghoul::opencl::Device* gDevice = new ghoul::opencl::Device(&device);
            gDevice->fetchInformation();
            gDevices.push_back(gDevice);
            
        }
        _data.push_back(PlatformAndDevices{gPlatform, gDevices});
    }
#endif
}

void OpenCLCapabilitiesComponent::clearCapabilities() {
#ifdef GHL_OPENCL
	for (PlatformAndDevices& data : _data) {
		delete data.platform;
		for (ghoul::opencl::Device* d : data.devices)
			delete d;
	}
#endif
	_data.clear();
}
    
std::vector<SystemCapabilitiesComponent::CapabilityInformation>
    OpenCLCapabilitiesComponent::capabilities(
        const SystemCapabilitiesComponent::Verbosity& verbosity) const
{
    std::vector<SystemCapabilitiesComponent::CapabilityInformation> result;
    std::stringstream ss;

#ifdef GHL_OPENCL
    if (verbosity >= Verbosity::Default) {
        for (size_t i = 0; i < _data.size(); ++i) {
            ss << "Platform[" << i << "] ";
            ghoul::opencl::Platform* p  = _data.at(i).platform;
            
            result.emplace_back(ss.str() + "Name" , p->name());
            result.emplace_back(ss.str() + "Vendor", p->vendor());
            result.emplace_back(ss.str() + "Profile", p->profile());
            result.emplace_back(ss.str() + "Version", p->version());
            for (size_t j = 0; j < _data.at(i).devices.size(); ++j) {
                ghoul::opencl::Device* d  = _data.at(i).devices.at(j);
                ss.str("");
                ss << "    Device[" << j << "] Name";
                result.emplace_back(ss.str() , d->name());
                result.emplace_back("        Vendor" , d->vendor());
                result.emplace_back("        Version" , d->version());
                result.emplace_back("        Max Compute Units" , toString(d->maxComputeUnits()));
                result.emplace_back("        Max Samplers" , toString(d->maxSamplers()));
                result.emplace_back("        Max Work Group Size" , toString(d->maxWorkGroupSize()));
                result.emplace_back("        Global Memory Size" , toString(d->globalMemSize()));
                result.emplace_back("        Local Memory Type" , toString(d->localMemType()));
                result.emplace_back("        Local Memory Size" , toString(d->localMemSize()));
                if (verbosity >= Verbosity::Full) {
                    result.emplace_back("        Build in kernels" , d->builtInKernels());
                    result.emplace_back("        Extensions" , d->extensions());
                }
            }
        }
    }
#endif
    return result;
}
    
std::string OpenCLCapabilitiesComponent::name() const {
    return "OpenCL";
}

} // namespace ghoul
} // namespace systemcapabilities