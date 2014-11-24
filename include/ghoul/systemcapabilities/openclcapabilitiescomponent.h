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

#ifndef __OPENCLCAPABILITIESCOMPONENT_H__
#define __OPENCLCAPABILITIESCOMPONENT_H__

#include "systemcapabilitiescomponent.h"

#include <string>
#include <vector>

namespace ghoul {

// Forward declare to minimize dependencies
namespace opencl {
class Platform;
class Device;
} // namespace opencl

namespace systemcapabilities {

/**
 * This subclass of SystemCapabilitiesComponent detects OpenCL-related capabilities, like available
 * platforms and devices. The most important device variables can be logged.
 */
class OpenCLCapabilitiesComponent : public SystemCapabilitiesComponent {
public:

    OpenCLCapabilitiesComponent();
    ~OpenCLCapabilitiesComponent();

    std::vector<CapabilityInformation> capabilities(
        const SystemCapabilitiesComponent::Verbosity& verbosity) const override;
    
    /**
     * Returns the <code>OpenCL</code> string.
     * \return The <code>OpenCL</code> string
     */
    std::string name() const override;
    
protected:
    void detectCapabilities() override;
    void clearCapabilities() override;
    
    // Internal struct used for detecting capabilities
    struct PlatformAndDevices {
        ghoul::opencl::Platform* platform;
        std::vector<ghoul::opencl::Device*> devices;
    };
    
    std::vector<PlatformAndDevices> _data;
    
};

} // namespace systemcapabilities
} // namespace ghoul

#endif // __OPENCLCAPABILITIESCOMPONENT_H__
