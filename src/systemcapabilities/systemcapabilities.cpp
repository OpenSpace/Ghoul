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

#include "systemcapabilities/systemcapabilities.h"

#include "logging/logmanager.h"
#include "systemcapabilities/systemcapabilitiescomponent.h"
#include "systemcapabilities/cpucapabilitiescomponent.h"
#include "systemcapabilities/openglcapabilitiescomponent.h"

#include <assert.h>
#include <sstream>
#include <typeinfo>
#include <algorithm>

namespace {
    const std::string _loggerCat = "SystemCapabilities";
}

namespace ghoul {
namespace systemcapabilities {

SystemCapabilities* SystemCapabilities::_systemCapabilities = nullptr;

SystemCapabilities::SystemCapabilities() {}

SystemCapabilities::~SystemCapabilities() {
    for (SystemCapabilitiesComponent* component : _components)
        delete component;
}

void SystemCapabilities::initialize() {
    assert(_systemCapabilities == nullptr);
    if (_systemCapabilities == nullptr)
        _systemCapabilities = new SystemCapabilities;
}

void SystemCapabilities::deinitialize() {
    assert(_systemCapabilities != nullptr);
    delete _systemCapabilities;
    _systemCapabilities = nullptr;
}

bool SystemCapabilities::isInitialized() {
    return (_systemCapabilities != nullptr);
}

SystemCapabilities& SystemCapabilities::ref() {
    assert(_systemCapabilities != nullptr);
    return *_systemCapabilities;
}

void SystemCapabilities::detectCapabilities() {
    for (SystemCapabilitiesComponent* c : _components) {
        if (!c->isInitialized())
            c->initialize();
    }
    clearCapabilities();
    for (SystemCapabilitiesComponent* component : _components)
        component->detectCapabilities();
}

void SystemCapabilities::clearCapabilities() {
    for (SystemCapabilitiesComponent* component : _components)
        component->clearCapabilities();
}

void SystemCapabilities::logCapabilities(
                                  SystemCapabilitiesComponent::Verbosity verbosity) const
{
    for (SystemCapabilitiesComponent* c : _components) {
        const std::string _loggerCat = ::_loggerCat + "." + c->name();
        std::vector<SystemCapabilitiesComponent::CapabilityInformation> cap = c->capabilities(verbosity);
        for (SystemCapabilitiesComponent::CapabilityInformation c : cap) {
            LINFO(c.first << ": " << c.second);
        }
    }
}

void SystemCapabilities::addComponent(SystemCapabilitiesComponent* component) {
#ifdef GHL_DEBUG
    //std::type_info i = typeid(*component);
    // TODO check typeid?
    if (std::find(
        _components.begin(), _components.end(), component) != _components.end())
    {
        LWARNING("Component with name '" << component->name() << "' was already added");
        return;
    }
#endif
    _components.push_back(component);
}

} // namespace systemcapabilities
} // namespace ghoul
