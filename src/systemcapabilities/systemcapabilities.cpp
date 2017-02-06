/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2017                                                               *
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

#include <ghoul/systemcapabilities/systemcapabilities.h>

#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/assert.h>

#include <algorithm>
#include <typeinfo>

namespace ghoul {
namespace systemcapabilities {

SystemCapabilities SystemCapabilities::_systemCapabilities;
bool SystemCapabilities::_isInitialized = false;

SystemCapabilities::CapabilitiesComponentNotFoundError::
CapabilitiesComponentNotFoundError()
    : ghoul::RuntimeError("SystemCapabilities not found", "SystemCapabilities")
{}

SystemCapabilities& SystemCapabilities::ref() {
    return _systemCapabilities;
}

bool SystemCapabilities::isInitialized() {
    return _isInitialized;
}

void SystemCapabilities::detectCapabilities() {
    clearCapabilities();
    for (const auto& component : _components) {
        component->detectCapabilities();
    }
    _isInitialized = true;
}

void SystemCapabilities::clearCapabilities() {
    for (const auto& component : _components) {
        component->clearCapabilities();
    }
    _isInitialized = false;
}

void SystemCapabilities::logCapabilities(
                                  SystemCapabilitiesComponent::Verbosity verbosity) const
{
    for (const auto& c : _components) {
        const auto& capabilities = c->capabilities();
        for (const auto& cap : capabilities) {
            if (verbosity >= cap.verbosity) {
                LINFOC(
                    "SystemCapabilitiesComponent." + c->name(),
                    cap.description << ": " << cap.value
                );
            }
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
