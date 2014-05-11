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

#ifndef __CPUCAPABILITIESCOMPONENT_H__
#define __CPUCAPABILITIESCOMPONENT_H__

#include "systemcapabilitiescomponent.h"

namespace ghoul {
namespace systemcapabilities {

/**
 * This subclass of SystemCapabilitiesComponent detects CPU-related capabilities, like CPU
 * information, main memory availability and other local, general hardware features. At
 * the current time, only the Operating System and the main memory are implemented.
 * \todo Implement CPU detection, feature detection
 * \todo More features
 */
class CPUCapabilitiesComponent : public SystemCapabilitiesComponent {
public:
    CPUCapabilitiesComponent();
    ~CPUCapabilitiesComponent();

    std::vector<CapabilityInformation> capabilities(
        const SystemCapabilitiesComponent::Verbosity& verbosity) const override;

    /**
     * Returns the operating system as a parsed string. The exact format of the returned
     * string is implementation and operating system-dependent but it should contain the
     * manufacturer and the version. The default value is <code>""</code>.
     * \return The operating system as a parsed string
     */
    const std::string& operatingSystem() const;

    /**
     * Returns the amount of available, installed main memory (RAM) on the system in MB.
     * This value is retrieved using the Windows Management Instrumentation (if
     * available). The default value is <code>-1</code>.
     * \return The amount of available main memory
     */
    unsigned int installedMainMemory() const;

    /**
     * Returns the amount of available, installed main memory (as reported by
     * #installedMainMemory) in a <code>string</code> with an <code>"MB"</code> suffix;
     * \return The amount of available, installed main memory
     */
    std::string installedMainMemoryAsString() const;

    /**
     * Returns the <code>CPU</code> string.
     * \return The <code>CPU</code> string
     */
    std::string name() const override;

protected:
    void detectCapabilities() override;
    void clearCapabilities() override;

    void detectOS();
    void detectMemory();

    std::string _operatingSystem; ///< Information about the operating system
    unsigned int _installedMainMemory; ///< The amount of RAM that is installed

};

} // namespace systemcapabilities
} // namespace ghoul

#endif // __CPUCAPABILITIESCOMPONENT_H__
