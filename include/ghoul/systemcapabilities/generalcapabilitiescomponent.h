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

#include <ghoul/systemcapabilities/systemcapabilitiescomponent.h>

#include <ghoul/misc/exception.h>
#include <ghoul/systemcapabilities/systemcapabilities.h>

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
    /// Main exception that is thrown if an error occured in the detection of general
    /// capabilities
    struct GeneralCapabilitiesComponentError : public RuntimeError {
        explicit GeneralCapabilitiesComponentError(std::string message);
    };

    /// Exception that is thrown if there was an error detecting the operating system
    struct OperatingSystemError : public GeneralCapabilitiesComponentError {
        explicit OperatingSystemError(std::string description, std::string errorMessage);
        
        /// The general description of the error
        std::string description;
        
        /// Additional information about the error message
        std::string errorMessage;
    };
    
    /// Exception that is thrown if there was an error detecting the main memory
    struct MainMemoryError : public GeneralCapabilitiesComponentError {
        explicit MainMemoryError(std::string message);
    };
    
    /**
     * Returns the list of all capabilities that were detected by this
     * SystemCapabilitiesComponent.
     * \return The list of all detected capabilities
     */
    std::vector<CapabilityInformation> capabilities() const override;

    /**
     * Returns the operating system as a parsed string. The exact format of the returned
     * string is implementation and operating system-dependent but it should contain the
     * manufacturer and the version.
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
    /**
     * Method that detects all of the capabilities.
     * \throw OperatingSystemError If the detection of the operating system failed
     * \throw MainMemoryError If the detection of the main memory failed
     */
    void detectCapabilities() override;
    void clearCapabilities() override;

    /**
     * Detects the operating system.
     * \throw OperatingSystemError If the detection of the operating system failed
     */
    void detectOS();
    
    /**
     * Detects the amount of the computer's main memory.
     * \throw MainMemoryError If the detection of the main memory failed
     * \throw WMIError If there was an error accessing the Windows Management
     */
    void detectMemory();
    
    /**
     * Detects detailed information about the CPU on this computer.
     */
	void detectCPU();

    /// Information about the operating system
    std::string _operatingSystem = "";
    
    /// The amount of RAM that is installed
    unsigned int _installedMainMemory = 0;
    
    /// Information about the CPU
	std::string _cpu = "";
    
    /// Number of CPU cores
	unsigned int _cores = 0;
    
    /// The size of a cache line
	unsigned int _cacheLineSize = 0;
    
    /// The associativity of the L2 cache
	unsigned int _L2Associativity = 0;
    
    /// The size of the cache
	unsigned int _cacheSize = 0;
    
    /// Available CPU extensions
	std::string _extensions = "";

};

} // namespace systemcapabilities
} // namespace ghoul

#define CpuCap (*(ghoul::systemcapabilities::SystemCapabilities::ref().component<ghoul::systemcapabilities::GeneralCapabilitiesComponent>()))

#endif // __GeneralCapabilitiesComponent_H__
