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

#ifndef __CPUCAPABILITIESCOMPONENT_H__
#define __CPUCAPABILITIESCOMPONENT_H__

#include "systemcapabilities/systemcapabilitiescomponent.h"

namespace ghoul {
namespace systemcapabilities {

class CPUCapabilitiesComponent : public SystemCapabilitiesComponent {
public:
    CPUCapabilitiesComponent();
    ~CPUCapabilitiesComponent();

    std::string createCapabilitiesString(
        const SystemCapabilitiesComponent::Verbosity& verbosity) const override;

protected:
    void detectCapabilities() override;
    void clearCapabilities() override;


    void detectOS();
    void detectMemory();

    std::string _operatingSystem; ///< Information about the Operating system
    unsigned int _installedMainMemory; ///< The amount of RAM that is installed on this machine

};

} // namespace systemcapabilities
} // namespace ghoul

#endif // __CPUCAPABILITIESCOMPONENT_H__
