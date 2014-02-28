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

#include <type_traits>

namespace ghoul {

namespace {
    /// This should not be used outside, as it is fairly useless on its own
    template <typename T, typename U>
    U* createType() {
        return new T;
    }
}

template <typename BaseClass>
BaseClass* TemplateFactory<BaseClass>::create(const std::string& className) const {
    std::map<std::string, FactoryFuncPtr>::const_iterator it = _map.find(className);
    if (it == _map.end()) {
        LERRORC("TemplateFactory", "Factory did not a class named '" << className << "'");
        return nullptr;
    }
    else
        // If 'className' is a valid name, we can use the stored functionpointer to create
        // the class using the 'createType' method
        return it->second();
}

template <typename BaseClass>
template <typename Class>
void TemplateFactory<BaseClass>::registerClass(const std::string& className) {
    static_assert(std::is_base_of<BaseClass, Class>::value,
        "BaseClass must be the base class of Class");
    FactoryFuncPtr function = &createType<Class>;
    _map.insert({ className, function });
}

template <typename BaseClass>
bool TemplateFactory<BaseClass>::hasClass(const std::string& className) const {
    return (_map.find(className) != _map.end());
}

}
