/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2015                                                               *
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

#ifndef __GENERALCAPABILITIESCOMPONENT_H__
#define __GENERALCAPABILITIESCOMPONENT_H__

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
class GeneralCapabilitiesComponent : public SystemCapabilitiesComponent {
public:
    GeneralCapabilitiesComponent();
    ~GeneralCapabilitiesComponent();

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
	 * Returns the number of cores
	 * \return The number of cores
	 */
	unsigned int cores() const;

	/**
	 * Returns the cache line size
	 * \return The cache line size
	 */
	unsigned int cacheLineSize() const;

	/**
	 * Returns the L2 associativity
	 * \return The L2 associativity
	 */
	unsigned int L2Associativity() const;

	/**
	 * Returns the cache size
	 * \return The cache size
	 */
	unsigned int cacheSize() const;

	/**
	 * Returns the number of cores as a string
	 * \return The number of cores
	 */
	std::string coresAsString() const;

	/**
	 * Returns the cache line size as a string
	 * \return The cache line size
	 */
	std::string cacheLineSizeAsString() const;

	/**
	 * Returns the L2 associativity as a string
	 * \return The L2 associativity
	 */
	std::string L2AssiciativityAsString() const;

	/**
	 * Returns the cache size in K as a string
	 * \return The cache size
	 */
	std::string cacheSizeAsString() const;

	/**
	 * Returns all supported exteions as commaseparated string
	 * \return The extension
	 */
	std::string extensions() const;

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
	void detectCPU();

    std::string _operatingSystem; ///< Information about the operating system
    unsigned int _installedMainMemory; ///< The amount of RAM that is installed
	std::string _cpu; ///< Information about the CPU
	unsigned int _cores;
	unsigned int _cacheLineSize;
	unsigned int _L2Associativity;
	unsigned int _cacheSize;
	std::string _extensions;

};

} // namespace systemcapabilities
} // namespace ghoul

#endif // __GeneralCapabilitiesComponent_H__
