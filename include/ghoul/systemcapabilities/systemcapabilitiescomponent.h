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

#ifndef __SYSTEMCAPABILITIESCOMPONENT_H__
#define __SYSTEMCAPABILITIESCOMPONENT_H__

#include <string>
#include <vector>

#ifdef GHOUL_USE_WMI
#include <wbemidl.h>
#include <comdef.h>
#endif

namespace ghoul {
namespace systemcapabilities {

/**
 * This class is the base class of all components that can detect a specific set of
 * features. Each subclass should focus on a specific module of features (like 
 * CPUCapabilitiesComponent or OpenGLCapabilitiesComponent). The action flow for a
 * SystemCapabilitiesComponent subclass is as follows: It gets created with the
 * constructor and should provide a descriptive name to this SystemCapabilitiesComponent`s
 * constructor, then, the #initialize function will be called, #detectCapabilities follows
 * and finally a call to #capabilities, which should return an <code>std::vector</code> of
 * <code>std::pair</code>'s with <code>{</code>description<code>,</code>value
 * <code>}</code>. Each SystemCapabilitiesComponent%'s #deinitialize function might be
 * called with a call to #initialize directly following and it should rescan all the
 * values and capabilities on the following #initialize function again to check for
 * changes.
 */
class SystemCapabilitiesComponent {
public:
    /// The verbosity that is used in the #capabilities method
    enum class Verbosity {
        Minimal, ///< The minimal verbosity presenting the absolute minimum information
        Default, ///< The default verbosity
        Full ///< Show all the available information
    };

    /// Each piece of capability information is represented by an information and a value
    typedef std::pair<std::string, std::string> CapabilityInformation;

    /// The constructor will initialize all necessary values
    SystemCapabilitiesComponent();

    /// The virtual destructor that will deinitialize all necessary values
    virtual ~SystemCapabilitiesComponent();

    /**
     * The base initialize method will initialize the Windows Management Instrumentation
     * (if the application is running on Windows and it was compiled with
     * <code>GHOUL_USE_WMI</code>). If the application is compiled not on Window or the
     * WMI is disabled in the compilation, no error will occur. If WMI should be used and
     * an error occurs, an error will be logged. This method needs to be called from each
     * derived subclass, even if WMI is not used.
     * \param initializeWMI If this parameter is <code>true</code>, the Windows Management
     * Instrumentation will be initialized.
     */
    virtual void initialize(bool initializeWMI = true);

    /**
     * The base deinitialize method will deinitialize the Windows Management
     * Instrumentation if it has been #initialize%d before. It will also automatically
     * call #clearCapabilities. This method has to be called from each derived subclass,
     * even if WMI is not used.
     */
    virtual void deinitialize();

    /**
     * Returns <code>true</code> if this SystemCapabilitiesComponent has been initialized
     * before; <code>false</code> otherwise.
     * \return <code>true</code> if this SystemCapabilitiesComponent has been initialized
     */
    bool isInitialized() const;

    /**
     * This method will need to detect all the capabilities or values the subclass is
     * responsible for. It is useful for the subclass to cache the values in member
     * variables and make them available though accessor functions as well as subsequent
     * calls to #capabilities.
     */
    virtual void detectCapabilities() = 0;

    /**
     * This method will clear all the capabilities of the subclass. Sensible default
     * values should be used that indicate clearly that it is an invalid value.
     */
    virtual void clearCapabilities() = 0;

    /**
     * This method returns pairs of <code>{</code>description<code>,</code> value
     * <code>}</code>, describing the features and capabilities the subclass is
     * responsible for. As a best-practice, the subclass should allow individual query for
     * each of the elements as well. The <code>verbosity</code> parameter should determine
     * how many of the available values should be added to the result.
     * \param verbosity The requested verbosity
     * \return Pairs of description+values for all of the capabilities
     */
    virtual std::vector<CapabilityInformation> capabilities(
        const SystemCapabilitiesComponent::Verbosity& verbosity) const = 0;

    /**
     * The implementation in the subclass should returns a descriptive name of the
     * component.
     * \return A descriptive name of the component
     */
    virtual std::string name() const = 0;

protected:
#ifdef GHOUL_USE_WMI
    /**
     * This method initializes the Windows Management Instrumentation. If the WMI is not
     * enabled, or the application is compiled on a non-Windows architecture, this method
     * is a no-op.
     */
    static void initializeWMI();

    /**
     * This method deinitializes the Windows Management Instrumentation. If the WMI is not
     * enabled, or the application is compiled on a non-Windows architecture, this method
     * is a no-op.
     */
    static void deinitializeWMI();

    /**
    * Returns if the Windows Management Instrumentation has been initialized before. If
    * the WMI is not enabled, or the application is compiled on a non-Windows
    * architecture, this method is a no-op.
    * \return <code>true</code> if the WMI has been initialized
    */
    static bool isWMIinitialized();

    /**
     * Queries the Windows Management Instrumentation for the <code>attribute</code>
     * contained in the <code>wmiClass</code> and returns the value of it. It will execute
     * a query on the Wbem services.
     * \param wmiClass The class in which the required attribute resides
     * \param attribute The attribute which we are interested in
     * \return The value of the requested attributes, or <code>nullptr</code> if an error
     * occurred
     */

    static  VARIANT* queryWMI(const std::string& wmiClass, const std::string& attribute);

    /**
     * Helper function that queries the Windows Management Instrumentation for the
     * <code>attribute</code> within the <code>wmiClass</code> and it expects the
     * attribute to be of type <code>std::string</code>. If the value could be retrieved,
     * the return value will be <code>true</code>. Otherwise <code>false</code> is
     * returned and the value remains unchanged.
     * \param wmiClass The class in which the required attribute resides
     * \param attribute The attribute which we are interested in
     * \param value A reference to the value, where the attribute will be stored in (if
     * the attribute could be found and it is of type <code>string</code>)
     * \return <code>true</code> if the attribute was retrieved
     */
    static bool queryWMI(const std::string& wmiClass,
        const std::string& attribute, std::string& value);

    /**
     * Helper function that queries the Windows Management Instrumentation for the
     * <code>attribute</code> within the <code>wmiClass</code> and it expects the
     * attribute to be of type <code>int</code>. If the value could be retrieved, the
     * return value will be <code>true</code>. Otherwise <code>false</code> is returned
     * and the value remains unchanged.
     * \param wmiClass The class in which the required attribute resides
     * \param attribute The attribute which we are interested in
     * \param value A reference to the value, where the attribute will be stored in (if
     * the attribute could be found and it is of type <code>int</code>)
     * \return <code>true</code> if the attribute was retrieved
     */
    static bool queryWMI(const std::string& wmiClass, const std::string& attribute, int& value);

    /**
     * Helper function that queries the Windows Management Instrumentation for the
     * <code>attribute</code> within the <code>wmiClass</code> and it expects the
     * attribute to be of type <code>unsigned int</code>. If the value could be retrieved,
     * the return value will be <code>true</code>. Otherwise <code>false</code> is
     * returned and the value remains unchanged.
     * \param wmiClass The class in which the required attribute resides
     * \param attribute The attribute which we are interested in
     * \param value A reference to the value, where the attribute will be stored in (if
     * the attribute could be found and it is of type <code>unsigned int</code>)
     * \return <code>true</code> if the attribute was retrieved
     */
    static bool queryWMI(const std::string& wmiClass, const std::string& attribute,
        unsigned int& value);

    /**
     * Helper function that queries the Windows Management Instrumentation for the
     * <code>attribute</code> within the <code>wmiClass</code> and it expects the
     * attribute to be of type <code>unsigned long long</code>. If the value could be
     * retrieved, the return value will be <code>true</code>. Otherwise <code>false</code>
     * is returned and the value remains unchanged.
     * \param wmiClass The class in which the required attribute resides
     * \param attribute The attribute which we are interested in
     * \param value A reference to the value, where the attribute will be stored in (if
     * the attribute could be found and it is of type <code>unsigned long long</code>)
     * \return <code>true</code> if the attribute was retrieved
     */
    static bool queryWMI(const std::string& wmiClass, const std::string& attribute,
        unsigned long long& value);

    /**
     * The locator object that was used in the <code>CoCreateInstance</code> call in the
     * #initializeWMI call
     */
    static IWbemLocator* _iwbemLocator;

    /**
     * The service object that was used in the <code>_iwbemLocator->ConnectServer</code>
     * call in the #initializeWMI call
     */
    static IWbemServices* _iwbemServices;
#endif

private:
    bool _isInitialized;
};

} // namespace systemcapabilities
} // namespace ghoul

#endif // __SYSTEMCAPABILITIESCOMPONENT_H__
