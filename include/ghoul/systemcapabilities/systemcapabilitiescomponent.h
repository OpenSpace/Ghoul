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

#ifndef __SYSTEMCAPABILITIESCOMPONENT_H__
#define __SYSTEMCAPABILITIESCOMPONENT_H__

#include <string>

#ifdef GHOUL_USE_WMI
#include <wbemidl.h>
#include <comdef.h>
#endif

namespace ghoul {
namespace systemcapabilities {

class SystemCapabilitiesComponent {
public:
    enum class Verbosity {
        Minimal,
        Default,
        Full
    };

    SystemCapabilitiesComponent(const std::string& componentName);
    virtual ~SystemCapabilitiesComponent();

    virtual void initialize();
    virtual void deinitialize();

    virtual void detectCapabilities();
    virtual void clearCapabilities();

    virtual std::string createCapabilitiesString(
        const SystemCapabilitiesComponent::Verbosity& verbosity) const;

    const std::string& name() const;

protected:
#ifdef GHOUL_USE_WMI
    void initializeWMI();
    void deinitializeWMI();
    bool isWMIinitialized() const;

    VARIANT* queryWMI(const std::string& wmiClass, const std::string& attribute) const;

    bool queryWMI(const std::string& wmiClass,
        const std::string& attribute, std::string& value) const;
    bool queryWMI(const std::string& wmiClass, const std::string& attribute, int& value) const;
    bool queryWMI(const std::string& wmiClass, const std::string& attribute,
        unsigned int& value) const;
    bool queryWMI(const std::string& wmiClass, const std::string& attribute,
        unsigned long long& value) const;

    IWbemLocator* _iwbemLocator;
    IWbemServices* _iwbemServices;
#endif


private:
    std::string _componentName;
};

} // namespace systemcapabilities
} // namespace ghoul

#endif // __SYSTEMCAPABILITIESCOMPONENT_H__
