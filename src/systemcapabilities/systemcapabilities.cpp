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

#include "ghoul/systemcapabilities/systemcapabilities.h"

#include "ghoul/logging/logmanager.h"
#include "ghoul/systemcapabilities/systemcapabilitiescomponent.h"
#include "ghoul/systemcapabilities/generalcapabilitiescomponent.h"
#include "ghoul/systemcapabilities/openglcapabilitiescomponent.h"

#include <assert.h>
#include <sstream>
#include <typeinfo>
#include <algorithm>
#include <ghoul/misc/assert.h>

namespace {
    const std::string _loggerCat = "SystemCapabilities";
}

namespace ghoul {
namespace systemcapabilities {

SystemCapabilities* SystemCapabilities::_systemCapabilities = nullptr;

void SystemCapabilities::initialize() {
    ghoul_assert(
        _systemCapabilities == nullptr,
        "Static SystemCapabilities must not have been initialized"
    );
    if (_systemCapabilities == nullptr)
        _systemCapabilities = new SystemCapabilities;
}

void SystemCapabilities::deinitialize() {
    ghoul_assert(
        _systemCapabilities, "Static SystemCapabilities must have been initialized"
    );
    delete _systemCapabilities;
    _systemCapabilities = nullptr;
}

bool SystemCapabilities::isInitialized() {
    return (_systemCapabilities != nullptr);
}

SystemCapabilities& SystemCapabilities::ref() {
    ghoul_assert(
        _systemCapabilities, "Static SystemCapabilities must have been initialized"
    );
    return *_systemCapabilities;
}

void SystemCapabilities::detectCapabilities() {
    clearCapabilities();
    for (auto& component : _components)
        component->detectCapabilities();
}

void SystemCapabilities::clearCapabilities() {
    for (auto& component : _components)
        component->clearCapabilities();
}

void SystemCapabilities::logCapabilities(
                                  SystemCapabilitiesComponent::Verbosity verbosity) const
{
    for (auto& c : _components) {
        const std::string _loggerCat = ::_loggerCat + "." + c->name();
        auto capabilities = c->capabilities();
        for (SystemCapabilitiesComponent::CapabilityInformation cap : capabilities) {
            if (verbosity >= cap.verbosity)
                LINFO(cap.description << ": " << cap.value);
        }
    }
}

void SystemCapabilities::addComponent(
                                  std::unique_ptr<SystemCapabilitiesComponent> component)
{
    ghoul_assert(component != nullptr, "Component must not be nullptr");
    
    auto it = std::find_if(
        _components.begin(),
        _components.end(),
        [&component](const std::unique_ptr<SystemCapabilitiesComponent>& rhs) {
            auto& c = *component;
            auto& r = *rhs;
            return typeid(c) == typeid(r);
        }
    );
    
    ghoul_assert(it == _components.end(), "Component must not have been added before");
    _components.push_back(std::move(component));
}

} // namespace systemcapabilities
} // namespace ghoul
