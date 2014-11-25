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
#include <ghoul/logging/logmanager.h>

namespace ghoul {

namespace {
    
/*
 * The working principle is as follows: There are two methods which can create subclasses,
 * 'create' and 'createWithDictionary'. The first one will create the subclass using the
 * default constructor while the second can use the default constructor or a constructor
 * using a ghoul::Dictionary as an input. Since both methods adhere to the same function
 * prototype 'BaseClass* (*FactoryFuncPtr)(bool, const Dictionary&)', they can be stored
 * in the same TemplateFactory's '_map' without the TemplateFactory knowing which of the
 * two functions it is. If C++ would support partial template specialization, two separate
 * methods wouldn't be necessary and could instead be split up by a single template
 * argument. Instead, a helper class 'CreateHelper' has to be created as partial template
 * specialization is allowed for classes. 'CreateHelper' has a single method that returns
 * either a 'create' or a 'createWithDictionary' function pointer, depending on the third
 * parameter Constructor; this parameter is determined at compile-time in the
 * registerClass by using the constant values DEFAULT_CONSTRUCTOR with
 * std::has_default_constructor and DICTIONARY_CONSTRUCTOR with std::is_convertible
 */

const int DEFAULT_CONSTRUCTOR = 1;
const int DICTIONARY_CONSTRUCTOR = 2;

/// Create Class using only the default constructor
template <typename BaseClass, typename Class>
BaseClass* createDefault(bool useDictionary, const Dictionary& dict) {
#ifdef GHL_DEBUG
    // We don't have a dictionary constructor, but the user tried to create it with a
    // Dictionary
    if (useDictionary || dict.size() != 0)
        LERRORC("TemplateFactory", "Class '" << typeid(Class).name() << "' does not " <<
        "provide a constructor receiving a Dictionary; the provided dictionary is " <<
        "silently ignored");
#endif
    return new Class;
}

// Create Class using the default constructor or the Dictionary
template <typename BaseClass, typename Class>
BaseClass* createDefaultAndDictionary(bool useDictionary, const Dictionary& dict) {
    if (useDictionary)
        return new Class(dict);
    else
        return new Class;
}

// Create Class using only the Dictionary constructor
template <typename BaseClass, typename Class>
BaseClass* createDictionary(bool useDictionary, const Dictionary& dict) {
    if (!useDictionary)
        // We don't have a default constructor, but the user tried to use it
        LERRORC("TemplateFactory", "Class '" << typeid(Class).name() << "' does only " <<
        "provide a Dictionary constructor but was called requesting the default " <<
        "constructor");
    return new Class(dict);
}

template <typename BaseClass, typename Class, int Constructor>
struct CreateHelper {
    typedef BaseClass* (*FactoryFuncPtr)(bool, const Dictionary&);
    FactoryFuncPtr createFunction();
};

template <typename BaseClass, typename Class>
struct CreateHelper<BaseClass, Class, DEFAULT_CONSTRUCTOR | DICTIONARY_CONSTRUCTOR> {
    typedef BaseClass* (*FactoryFuncPtr)(bool, const Dictionary&);
    FactoryFuncPtr createFunction() {
        return &createDefaultAndDictionary<BaseClass, Class>;
    }
};

template <typename BaseClass, typename Class>
struct CreateHelper<BaseClass, Class, DEFAULT_CONSTRUCTOR> {
    typedef BaseClass* (*FactoryFuncPtr)(bool, const Dictionary&);
    FactoryFuncPtr createFunction() {
        return &createDefault<BaseClass, Class>;
    }
};

template <typename BaseClass, typename Class>
struct CreateHelper<BaseClass, Class, DICTIONARY_CONSTRUCTOR> {
    typedef BaseClass* (*FactoryFuncPtr)(bool, const Dictionary&);
    FactoryFuncPtr createFunction() {
        return &createDictionary<BaseClass, Class>;
    }
};

}

template <typename BaseClass>
BaseClass* TemplateFactory<BaseClass>::create(const std::string& className) const {
    auto it = _map.find(className);
    if (it == _map.end()) {
        LERRORC("TemplateFactory", "Factory did not have a class '" << className << "'");
        return nullptr;
    }
    else
        // If 'className' is a valid name, we can use the stored function pointer to
		// create the class using the 'createType' method
        return it->second(false, {});
}

template <typename BaseClass>
BaseClass* TemplateFactory<BaseClass>::create(const std::string& className,
                                              const Dictionary& dictionary) const
{
    auto it = _map.find(className);
    if (it == _map.end()) {
        LERRORC("TemplateFactory", "Factory did not have a class '" << className << "'");
        return nullptr;
    }
    else
        // If 'className' is a valid name, we can use the stored function pointer to
		// create the class using the 'createType' method
        return it->second(true, dictionary);
}

template <typename BaseClass>
template <typename Class>
void TemplateFactory<BaseClass>::registerClass(std::string className) {
    static_assert(std::is_base_of<BaseClass, Class>::value,
        "BaseClass must be the base class of Class");
    static_assert(
        std::is_default_constructible<Class>::value |
        std::is_convertible<Dictionary, Class>::value,
        "Class needs a public default or Dictionary constructor");

    // Use the correct CreateHelper struct to create a function pointer that we can store
    // for later usage. std::is_convertible<>::value returns a boolean that checks at
    // run-time if it is possible to convert a Dictionary into a Class (thereby implicitly
    // checking if there is a proper constructor for it)
    FactoryFuncPtr&& function = CreateHelper<BaseClass, Class,
        (std::is_default_constructible<Class>::value * DEFAULT_CONSTRUCTOR) |
        (std::is_convertible<Dictionary, Class>::value * DICTIONARY_CONSTRUCTOR)
    >().createFunction();

    registerClass(std::move(className), function);
}


template <typename BaseClass>
void TemplateFactory<BaseClass>::registerClass(std::string className,
                                               FactoryFuncPtr factoryFunction)
{
    registerClass(std::move(className), FactoryFunction(std::move(factoryFunction)));
}

template <typename BaseClass>
void TemplateFactory<BaseClass>::registerClass(std::string className,
            std::function<BaseClass*(bool, const ghoul::Dictionary&)> factoryFunction)
{
	_map.emplace(std::move(className), std::move(factoryFunction));
}

template <typename BaseClass>
bool TemplateFactory<BaseClass>::hasClass(const std::string& className) const {
    return (_map.find(className) != _map.end());
}

}

template <typename BaseClass>
const std::type_info& ghoul::TemplateFactory<BaseClass>::baseClassType() const
{
    return typeid(BaseClass);
}

