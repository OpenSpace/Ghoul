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

#ifndef __SYSTEMCAPABILITIES_H__
#define __SYSTEMCAPABILITIES_H__

#include "systemcapabilitiescomponent.h"
#include "cpucapabilitiescomponent.h"
#include "openglcapabilitiescomponent.h"
#include "openclcapabilitiescomponent.h"

#include <string>
#include <vector>

namespace ghoul {
namespace systemcapabilities {

/**
 * The SystemCapabilities class allows access to the functionality the system provides.
 * The class is composed of SystemCapabilitiesComponent%s with each component checking for
 * a specific kind of capabilities (for example OpenGLCapabilitiesComponent or
 * CPUCapabilitiesComponent) and provides access to the components with the templated
 * #component method. The values are not guaranteed to be constant over the lifetime of
 * the application, but most values can be considered to be static and cache-able. New
 * components can be added using the #addComponent and retrieved using the #component
 * method. Each type of component can only be added once to the SystemCapabilities. The
 * second insertion will log a warning if <code>GHL_DEBUG</code> is defined.
 */
class SystemCapabilities {
public:
    /**
     * Initializes the static member variable and makes the global SystemCapabilities
     * available via the #ref method. Calling this method twice will trigger an assertion.
     */
    static void initialize();

    /**
     * Destroys the static member variable and cleans up all the
     * SystemCapabilitiesComponent%s that have been added to this SystemCapabilities
     * (#addComponent). The
     * #ghoul::systemcapabilities::SystemCapabilitiesComponent::deinitialize methods will
     * be called in the process. If this method is called and the SystemCapabilities is 
     * already destroyed, an assertion will be triggered.
     */
    static void deinitialize();

    /**
     * Returns a reference to the global SystemCapabilities object. The system must have
     * been initialized previously, or an assertion will be triggered.
     * \return A reference to the global SystemCapabilities object
     */
    static SystemCapabilities& ref();

    /**
     * Returns the initialization state of the SystemCapabilities object. 
     * \return The initialization state of the SystemCapabilities object
     */
    static bool isInitialized();

    /**
     * Calling this method will trigger the
     * #ghoul::systemcapabilities::SystemCapabilitiesComponent::detectCapabilities of all
     * registered components (#addComponent). If the capabilities have been detected
     * previously and a new component is added, a following call will redetect the
     * capabilities of all components, thus, multiple calls to this function will perform
     * a full redetection.
     */
    void detectCapabilities();

    /**
     * Logs all of the detected capabilities of the log, group by the individual
     * SystemCapabilitiesComponent%s. The verbosity of the log is controlled by the
     * <code>verbosity</code>. This method will, in turn, call the
     * #ghoul::systemcapabilities::SystemCapabilitiesComponent::capabilities of all the
     * registered SystemCapabilitiesComponent%s.
     * \param verbosity The verbosity of the resulting log entries
     */
    void logCapabilities(SystemCapabilitiesComponent::Verbosity verbosity =
                                   SystemCapabilitiesComponent::Verbosity::Default) const;

    /**
     * Adds the passed <code>component</code> to this SystemCapabilities and assumes
     * ownership of this object. This method will not automatically initialize and/or
     * detect the capabilities in this component; this has to be done using the
     * #ghoul::systemcapabilities::SystemCapabilitiesComponent::initialize and
     * #ghoul::systemcapabilities::SystemCapabilitiesComponent::detectCapabilities
     * methods. A specific subclass of SystemCapabilitiesComponent can only be added once
     * to the SystemCapabilities. This requirement will only be tested if
     * <code>GHL_DEBUG</code> is defined.
     * \param component The component that will be added to this SystemCapabilities
     * object. This method call will transfer ownership of the component to this object
     * permanently.
     */
    void addComponent(SystemCapabilitiesComponent* component);

    /**
     * Returns the component of type T or <code>nullptr</code> if no such component
     * exists.
     * \tparam T The subclass of SystemCapabilitiesComponent that should be retrieved
     * \return The SystemCapabilitiesComponent that should be retrieved
     */
    template <class T>
    T* component();

private:
    /**
     * Creates an empty SystemCapabilities object. The constructor will be called by the
     * static #create method.
     */
    SystemCapabilities();
    /**
     * Destructor that will deinitialize and dispose of all the
     * SystemCapabilitiesComponent%s that have been added to this SystemCapabilities
     * object.
     */
    ~SystemCapabilities();

    SystemCapabilities(const SystemCapabilities& rhs) = delete;

    void clearCapabilities();

    std::vector<SystemCapabilitiesComponent*> _components;

    static SystemCapabilities* _systemCapabilities;  ///< singleton member
};

} // namespace systemcapabilities
} // namespace ghoul

#define SysCap (ghoul::systemcapabilities::SystemCapabilities::ref())
#define CpuCap (*(ghoul::systemcapabilities::SystemCapabilities::ref().component<ghoul::systemcapabilities::CPUCapabilitiesComponent>()))
#define OpenGLCap (*(ghoul::systemcapabilities::SystemCapabilities::ref().component<ghoul::systemcapabilities::OpenGLCapabilitiesComponent>()))
#define OpenCLCap (*(ghoul::systemcapabilities::SystemCapabilities::ref().component<ghoul::systemcapabilities::OpenCLCapabilitiesComponent>()))

#include "systemcapabilities.inl"

#endif // __SYSTEMCAPABILITIES_H__
