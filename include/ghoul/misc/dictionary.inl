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

#include <ghoul/logging/logmanager.h>

namespace ghoul {
    
//extern template bool Dictionary::setValue<double>(std::string key, double value, bool createIntermediate);
//extern template bool Dictionary::getValue<double>(const std::string& key, double& value) const;
//extern template bool Dictionary::hasValue<double>(const std::string& key) const;
    
template <typename T>
bool ghoul::Dictionary::setValueHelper(std::string key, T value,
                                       bool createIntermediate) {
    std::string first;
    std::string rest;
    const bool hasRestPath = splitKey(key, first, rest);
    if (!hasRestPath) {
        // if no rest exists, key == first and we can just insert the value
        (*this)[key] = std::move(value);
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
    return dict->setValue(rest, std::move(value), createIntermediate);
}

//template <typename T>
//bool Dictionary::setValueInternal(std::string key, T value, bool createIntermediate) {
//    return setValueHelper(std::move(key), std::move(value), createIntermediate);
//}
//
//template <typename T, typename U>
//bool Dictionary::setValueInternal(std::string key, T value, bool createIntermediate) {
//    return setValueHelper(
//                          std::move(key),
//                          static_cast<typename internal::StorageTypeConverter<T>::type>(value),
//                          createIntermediate
//                          );
//}
    
    
template <typename T>
bool Dictionary::setValue(std::string key, T value, bool createIntermediate) {
    return setValueInternal(std::move(key), std::move(value), createIntermediate);
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

//template <typename T, bool HasStorageConverter>
//bool Dictionary::getValueInternal(const std::string& key, T& value) const {
//    if (HasStorageConverter) {
//        typename internal::StorageTypeConverter<T>::type v;
//        bool success = hasValueHelper<internal::StorageTypeConverter<T>::type>(key);
//        if (success) {
//            getValueHelper(key, v);
//            value = static_cast<T>(v);
//            return success;
//        }
//        else {
//            bool hasDictionary = hasValueHelper<Dictionary>(key);
//            if (hasDictionary) {
//                Dictionary dict;
//                getValueHelper(key, dict);
//                bool canConvert = isConvertible<T>(dict);
//                if (canConvert) {
//                    convert(dict, value);
//                    return true;
//                }
//            }
//        }
//        return false;
//    }
//    else
//        return getValueHelper(key, value);
//}

template <typename T>
bool Dictionary::getValue(const std::string& key, T& value) const {
    return getValueInternal(key, value);
}

template <typename T>
T ghoul::Dictionary::value(const std::string& key) const {
	T value;
	getValue(key, value);
	return value;
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

//template <typename T, bool HasStorageConverter>
//bool Dictionary::hasValueInternal(const std::string& key) const {
//    if (HasStorageConverter) {
//        bool val = hasValueHelper<internal::StorageTypeConverter<T>::type>(key);
//        if (val)
//            return true;
//        else {
//            bool hasDictionary = hasValueHelper<Dictionary>(key);
//            if (hasDictionary) {
//                Dictionary dict;
//                getValueHelper(key, dict);
//                bool canConvert = isConvertible<T>(dict);
//                if (canConvert)
//                    return true;
//            }
//            return false;
//        }
//    }
//    else
//        return hasValueHelper<T>(key);
//}

template <typename T>
bool Dictionary::hasValue(const std::string& key) const {
    return hasValueInternal<T>(key);
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
    
extern template bool Dictionary::setValue<bool>(std::string key, bool value, bool createIntermediate);
extern template bool Dictionary::getValue<bool>(const std::string& key, bool& value) const;
extern template bool Dictionary::hasValue<bool>(const std::string& key) const;

extern template bool Dictionary::setValue<char>(std::string key, char value, bool createIntermediate);
extern template bool Dictionary::getValue<char>(const std::string& key, char& value) const;
extern template bool Dictionary::hasValue<char>(const std::string& key) const;

extern template bool Dictionary::setValue<signed char>(std::string key, signed char value, bool createIntermediate);
extern template bool Dictionary::getValue<signed char>(const std::string& key, signed char& value) const;
extern template bool Dictionary::hasValue<signed char>(const std::string& key) const;

extern template bool Dictionary::setValue<unsigned char>(std::string key, unsigned char value, bool createIntermediate);
extern template bool Dictionary::getValue<unsigned char>(const std::string& key, unsigned char& value) const;
extern template bool Dictionary::hasValue<unsigned char>(const std::string& key) const;

extern template bool Dictionary::setValue<wchar_t>(std::string key, wchar_t value, bool createIntermediate);
extern template bool Dictionary::getValue<wchar_t>(const std::string& key, wchar_t& value) const;
extern template bool Dictionary::hasValue<wchar_t>(const std::string& key) const;

extern template bool Dictionary::setValue<short>(std::string key, short value, bool createIntermediate);
extern template bool Dictionary::getValue<short>(const std::string& key, short& value) const;
extern template bool Dictionary::hasValue<short>(const std::string& key) const;

extern template bool Dictionary::setValue<unsigned short>(std::string key, unsigned short value, bool createIntermediate);
extern template bool Dictionary::getValue<unsigned short>(const std::string& key, unsigned short& value) const;
extern template bool Dictionary::hasValue<unsigned short>(const std::string& key) const;

extern template bool Dictionary::setValue<int>(std::string key, int value, bool createIntermediate);
extern template bool Dictionary::getValue<int>(const std::string& key, int& value) const;
extern template bool Dictionary::hasValue<int>(const std::string& key) const;

extern template bool Dictionary::setValue<unsigned int>(std::string key, unsigned int value, bool createIntermediate);
extern template bool Dictionary::getValue<unsigned int>(const std::string& key, unsigned int& value) const;
extern template bool Dictionary::hasValue<unsigned int>(const std::string& key) const;

extern template bool Dictionary::setValue<long long>(std::string key, long long value, bool createIntermediate);
extern template bool Dictionary::getValue<long long>(const std::string& key, long long& value) const;
extern template bool Dictionary::hasValue<long long>(const std::string& key) const;

extern template bool Dictionary::setValue<unsigned long long>(std::string key, unsigned long long value, bool createIntermediate);
extern template bool Dictionary::getValue<unsigned long long>(const std::string& key, unsigned long long& value) const;
extern template bool Dictionary::hasValue<unsigned long long>(const std::string& key) const;

extern template bool Dictionary::setValue<float>(std::string key, float value, bool createIntermediate);
extern template bool Dictionary::getValue<float>(const std::string& key, float& value) const;
extern template bool Dictionary::hasValue<float>(const std::string& key) const;

extern template bool Dictionary::setValue<double>(std::string key, double value, bool createIntermediate);
extern template bool Dictionary::getValue<double>(const std::string& key, double& value) const;
extern template bool Dictionary::hasValue<double>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::vec2>(std::string key, glm::vec2 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::vec2>(const std::string& key, glm::vec2& value) const;
extern template bool Dictionary::hasValue<glm::vec2>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::dvec2>(std::string key, glm::dvec2 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::dvec2>(const std::string& key, glm::dvec2& value) const;
extern template bool Dictionary::hasValue<glm::dvec2>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::ivec2>(std::string key, glm::ivec2 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::ivec2>(const std::string& key, glm::ivec2& value) const;
extern template bool Dictionary::hasValue<glm::ivec2>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::uvec2>(std::string key, glm::uvec2 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::uvec2>(const std::string& key, glm::uvec2& value) const;
extern template bool Dictionary::hasValue<glm::uvec2>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::bvec2>(std::string key, glm::bvec2 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::bvec2>(const std::string& key, glm::bvec2& value) const;
extern template bool Dictionary::hasValue<glm::bvec2>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::vec3>(std::string key, glm::vec3 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::vec3>(const std::string& key, glm::vec3& value) const;
extern template bool Dictionary::hasValue<glm::vec3>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::dvec3>(std::string key, glm::dvec3 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::dvec3>(const std::string& key, glm::dvec3& value) const;
extern template bool Dictionary::hasValue<glm::dvec3>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::ivec3>(std::string key, glm::ivec3 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::ivec3>(const std::string& key, glm::ivec3& value) const;
extern template bool Dictionary::hasValue<glm::ivec3>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::uvec3>(std::string key, glm::uvec3 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::uvec3>(const std::string& key, glm::uvec3& value) const;
extern template bool Dictionary::hasValue<glm::uvec3>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::bvec3>(std::string key, glm::bvec3 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::bvec3>(const std::string& key, glm::bvec3& value) const;
extern template bool Dictionary::hasValue<glm::bvec3>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::vec4>(std::string key, glm::vec4 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::vec4>(const std::string& key, glm::vec4& value) const;
extern template bool Dictionary::hasValue<glm::vec4>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::dvec4>(std::string key, glm::dvec4 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::dvec4>(const std::string& key, glm::dvec4& value) const;
extern template bool Dictionary::hasValue<glm::dvec4>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::ivec4>(std::string key, glm::ivec4 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::ivec4>(const std::string& key, glm::ivec4& value) const;
extern template bool Dictionary::hasValue<glm::ivec4>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::uvec4>(std::string key, glm::uvec4 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::uvec4>(const std::string& key, glm::uvec4& value) const;
extern template bool Dictionary::hasValue<glm::uvec4>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::bvec4>(std::string key, glm::bvec4 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::bvec4>(const std::string& key, glm::bvec4& value) const;
extern template bool Dictionary::hasValue<glm::bvec4>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::mat2x2>(std::string key, glm::mat2x2 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::mat2x2>(const std::string& key, glm::mat2x2& value) const;
extern template bool Dictionary::hasValue<glm::mat2x2>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::mat2x3>(std::string key, glm::mat2x3 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::mat2x3>(const std::string& key, glm::mat2x3& value) const;
extern template bool Dictionary::hasValue<glm::mat2x3>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::mat2x4>(std::string key, glm::mat2x4 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::mat2x4>(const std::string& key, glm::mat2x4& value) const;
extern template bool Dictionary::hasValue<glm::mat2x4>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::mat3x2>(std::string key, glm::mat3x2 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::mat3x2>(const std::string& key, glm::mat3x2& value) const;
extern template bool Dictionary::hasValue<glm::mat3x2>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::mat3x3>(std::string key, glm::mat3x3 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::mat3x3>(const std::string& key, glm::mat3x3& value) const;
extern template bool Dictionary::hasValue<glm::mat3x3>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::mat3x4>(std::string key, glm::mat3x4 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::mat3x4>(const std::string& key, glm::mat3x4& value) const;
extern template bool Dictionary::hasValue<glm::mat3x4>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::mat4x2>(std::string key, glm::mat4x2 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::mat4x2>(const std::string& key, glm::mat4x2& value) const;
extern template bool Dictionary::hasValue<glm::mat4x2>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::mat4x3>(std::string key, glm::mat4x3 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::mat4x3>(const std::string& key, glm::mat4x3& value) const;
extern template bool Dictionary::hasValue<glm::mat4x3>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::mat4x4>(std::string key, glm::mat4x4 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::mat4x4>(const std::string& key, glm::mat4x4& value) const;
extern template bool Dictionary::hasValue<glm::mat4x4>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::dmat2x2>(std::string key, glm::dmat2x2 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::dmat2x2>(const std::string& key, glm::dmat2x2& value) const;
extern template bool Dictionary::hasValue<glm::dmat2x2>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::dmat2x3>(std::string key, glm::dmat2x3 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::dmat2x3>(const std::string& key, glm::dmat2x3& value) const;
extern template bool Dictionary::hasValue<glm::dmat2x3>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::dmat2x4>(std::string key, glm::dmat2x4 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::dmat2x4>(const std::string& key, glm::dmat2x4& value) const;
extern template bool Dictionary::hasValue<glm::dmat2x4>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::dmat3x2>(std::string key, glm::dmat3x2 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::dmat3x2>(const std::string& key, glm::dmat3x2& value) const;
extern template bool Dictionary::hasValue<glm::dmat3x2>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::dmat3x3>(std::string key, glm::dmat3x3 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::dmat3x3>(const std::string& key, glm::dmat3x3& value) const;
extern template bool Dictionary::hasValue<glm::dmat3x3>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::dmat3x4>(std::string key, glm::dmat3x4 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::dmat3x4>(const std::string& key, glm::dmat3x4& value) const;
extern template bool Dictionary::hasValue<glm::dmat3x4>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::dmat4x2>(std::string key, glm::dmat4x2 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::dmat4x2>(const std::string& key, glm::dmat4x2& value) const;
extern template bool Dictionary::hasValue<glm::dmat4x2>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::dmat4x3>(std::string key, glm::dmat4x3 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::dmat4x3>(const std::string& key, glm::dmat4x3& value) const;
extern template bool Dictionary::hasValue<glm::dmat4x3>(const std::string& key) const;

extern template bool Dictionary::setValue<glm::dmat4x4>(std::string key, glm::dmat4x4 value, bool createIntermediate);
extern template bool Dictionary::getValue<glm::dmat4x4>(const std::string& key, glm::dmat4x4& value) const;
extern template bool Dictionary::hasValue<glm::dmat4x4>(const std::string& key) const;


template<>
bool Dictionary::getValue<Dictionary>(const std::string& key, Dictionary& value) const;

} // namespace ghoul
