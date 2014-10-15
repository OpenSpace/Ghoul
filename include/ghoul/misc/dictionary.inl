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

// Removes a warning that occurs when trying to convert int directly to bool values in the
// glm::bvecX template specializations
#ifdef WIN32
#pragma warning(disable : 4800)
#endif

#include <ghoul/glm.h>

#ifdef WIN32
#pragma warning(default : 4800)
#endif

#include <ghoul/logging/logmanager.h>

namespace ghoul {

template <typename T>
bool ghoul::Dictionary::setValueHelper(std::string key, T value,
                                       bool createIntermediate) {
    std::string first;
    std::string rest;
    const bool hasRestPath = splitKey(key, first, rest);
    if (!hasRestPath) {
        // if no rest exists, key == first and we can just insert the value
        (*this)[key] = value;
        return true;
    }

    // if we get to this point, the 'key' did contain a nested key
    // so we have to find the correct Dictionary (or create it if it doesn't exist)
    std::map<std::string, boost::any>::iterator keyIt = find(first);
    if (keyIt == cend()) {
        // didn't find the Dictionary
        if (createIntermediate) {
            ghoul::Dictionary intermediate;
            (*this)[first] = intermediate;
            keyIt = find(first);
        }
        else {
            LERRORC("Dictionary", "Key '" << first << "' was not found in dictionary");
            return false;
        }
    }

    // See if it is actually a Dictionary at this location
    Dictionary* const dict = boost::any_cast<Dictionary>(&(keyIt->second));
    if (dict == nullptr) {
        LERRORC("Dictionary", "Error converting key '"
            << first << "' to type 'Dictionary', was '"
            << keyIt->second.type().name() << "'");
        return false;
    }
    // Proper tail-recursion
    return dict->setValue(rest, value, createIntermediate);
}

template <typename T>
bool Dictionary::setValue(std::string key, T value, bool createIntermediate) {
    return setValueHelper(std::move(key), std::move(value), createIntermediate);
}


template <typename T>
bool ghoul::Dictionary::getValueHelper(const std::string& key, T& value) const {
    // If we can find the key directly, we can return it immediately
    const std::map<std::string, boost::any>::const_iterator it = find(key);
    if (it != cend()) {
        const T* const v = boost::any_cast<T>(&(it->second));
        // See if it has the correct type
        if (v == nullptr) {
            LERRORC("Dictionary", "Wrong type of key '"
                << key << "': Expected '" << typeid(T).name()
                << "', got '" << it->second.type().name() << "'");
            return false;
        }
        value = *v;
        return true;
    }

    // if we get to this point, the 'key' did contain a nested key
    // so we have to find the correct Dictionary (or create it if it doesn't exist)
    std::string first;
    std::string rest;
    splitKey(key, first, rest);

    const std::map<std::string, boost::any>::const_iterator keyIt = find(first);
    if (keyIt == cend()) {
#ifdef GHL_DEBUG
		LERRORC("Dictionary", "Could not find key '" << first << "' in Dictionary");
#endif
        return false;
	}

    const Dictionary* const dict = boost::any_cast<Dictionary>(&(keyIt->second));
    // See if it is actually a Dictionary at this location
    if (dict == nullptr) {
        LERRORC("Dictionary", "Error converting key '"
            << first << "' to type 'Dictionary', was '"
            << keyIt->second.type().name() << "'");
        return false;
    }
    // Proper tail-recursion
    return dict->getValue<T>(rest, value);
}

template <typename T>
bool Dictionary::getValue(const std::string& key, T& value) const {
    return getValueHelper(key, value);
}

template <typename T>
bool ghoul::Dictionary::hasValueHelper(const std::string& key) const {
    const std::map<std::string, boost::any>::const_iterator it = find(key);
    if (it != cend()) {
        // If we can find the key directly, we can check the types and return
        const bool typeCorrect = (it->second.type() == typeid(T));
        return typeCorrect;
    }

    std::string first;
    std::string rest;
    splitKey(key, first, rest);

    const std::map<std::string, boost::any>::const_iterator keyIt = find(first);
    if (keyIt == cend())
        // If we can't find the first part of nested key, there is no need to continue
        return false;

    const Dictionary* const dict = boost::any_cast<Dictionary>(&(keyIt->second));
    if (dict == nullptr)
        // If it is not a Dictionary, the value can't be found and no recursion necessary
        return false;

    // Proper tail-recursion
    return dict->hasValue<T>(rest);
}

template <typename T>
bool Dictionary::hasValue(const std::string& key) const {
    return hasValueHelper<T>(key);
}
    
template <typename T>
bool Dictionary::hasKeyAndValue(const std::string& key) const {
    // Short-circuit evaluation is used to guard the 'hasValue' function from non-existing
    // keys
    return (hasKey(key) && hasValue<T>(key));
}

// Make template definitions so that the compiler won't try to instantiate each
// member function individually whenever it is encountered. This way, we promise the
// compiler that they will be instantiated somewhere else. This is done in the
// dictionary.cpp compilation unit; if a new template specialization is added, it must be
// included here and in the dictionary.cpp file

#define DEF_EXT_TEMPLATES(TYPE)                                                          \
    template<> bool Dictionary::setValue<TYPE>(std::string key, TYPE value,         \
                                                    bool createIntermediate);            \
    template<> bool Dictionary::getValue<TYPE>(const std::string& key, TYPE& value) \
          const;                                                                         \
    template<> bool Dictionary::hasValue<TYPE>(const std::string& key) const;

DEF_EXT_TEMPLATES(bool)
DEF_EXT_TEMPLATES(char)
DEF_EXT_TEMPLATES(signed char)
DEF_EXT_TEMPLATES(unsigned char)
DEF_EXT_TEMPLATES(wchar_t)
DEF_EXT_TEMPLATES(short)
DEF_EXT_TEMPLATES(unsigned short)
DEF_EXT_TEMPLATES(int)
DEF_EXT_TEMPLATES(unsigned int)
DEF_EXT_TEMPLATES(long long)
DEF_EXT_TEMPLATES(unsigned long long)
DEF_EXT_TEMPLATES(float)
DEF_EXT_TEMPLATES(double)
DEF_EXT_TEMPLATES(glm::vec2)
DEF_EXT_TEMPLATES(glm::dvec2)
DEF_EXT_TEMPLATES(glm::ivec2)
DEF_EXT_TEMPLATES(glm::uvec2)
DEF_EXT_TEMPLATES(glm::bvec2)
DEF_EXT_TEMPLATES(glm::vec3)
DEF_EXT_TEMPLATES(glm::dvec3)
DEF_EXT_TEMPLATES(glm::ivec3)
DEF_EXT_TEMPLATES(glm::uvec3)
DEF_EXT_TEMPLATES(glm::bvec3)
DEF_EXT_TEMPLATES(glm::vec4)
DEF_EXT_TEMPLATES(glm::dvec4)
DEF_EXT_TEMPLATES(glm::ivec4)
DEF_EXT_TEMPLATES(glm::uvec4)
DEF_EXT_TEMPLATES(glm::bvec4)
DEF_EXT_TEMPLATES(glm::mat2x2)
DEF_EXT_TEMPLATES(glm::mat2x3)
DEF_EXT_TEMPLATES(glm::mat2x4)
DEF_EXT_TEMPLATES(glm::mat3x2)
DEF_EXT_TEMPLATES(glm::mat3x3)
DEF_EXT_TEMPLATES(glm::mat3x4)
DEF_EXT_TEMPLATES(glm::mat4x2)
DEF_EXT_TEMPLATES(glm::mat4x3)
DEF_EXT_TEMPLATES(glm::mat4x4)
DEF_EXT_TEMPLATES(glm::dmat2x2)
DEF_EXT_TEMPLATES(glm::dmat2x3)
DEF_EXT_TEMPLATES(glm::dmat2x4)
DEF_EXT_TEMPLATES(glm::dmat3x2)
DEF_EXT_TEMPLATES(glm::dmat3x3)
DEF_EXT_TEMPLATES(glm::dmat3x4)
DEF_EXT_TEMPLATES(glm::dmat4x2)
DEF_EXT_TEMPLATES(glm::dmat4x3)
DEF_EXT_TEMPLATES(glm::dmat4x4)

template<> bool Dictionary::getValue<Dictionary>(const std::string& key,
                                                      Dictionary& value) const;

#undef DEF_EXT_TEMPLATES

} // namespace ghoul
