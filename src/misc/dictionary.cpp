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

#include "ghoul/misc/dictionary.h"

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <array>

using std::string;

/**
 * The reason for the unified storage space is the easy conversion between similar types.
 * The Dictionary should not be used for high-performance code anyway, so the additional
 * storage requirement is a valid trade-off.
 */

namespace {
const std::string _loggerCat = "Dictionary";

typedef long long IntegralType;
typedef unsigned long long UnsignedIntegralType;
typedef double FloatingType;

template <typename InputType>
struct StorageTypeConverter {};

#define DEF_STORAGE_CONVERTER(T, U, S)                                                   \
    template <>                                                                          \
    struct StorageTypeConverter<T> {                                                     \
        typedef U type;                                                                  \
        static const size_t size = S;                                                    \
    };

DEF_STORAGE_CONVERTER(bool, IntegralType, 1)
DEF_STORAGE_CONVERTER(char, IntegralType, 1)
DEF_STORAGE_CONVERTER(signed char, IntegralType, 1)
DEF_STORAGE_CONVERTER(unsigned char, UnsignedIntegralType, 1)
DEF_STORAGE_CONVERTER(wchar_t, IntegralType, 1)
DEF_STORAGE_CONVERTER(short, IntegralType, 1)
DEF_STORAGE_CONVERTER(unsigned short, UnsignedIntegralType, 1)
DEF_STORAGE_CONVERTER(int, IntegralType, 1)
DEF_STORAGE_CONVERTER(unsigned int, UnsignedIntegralType, 1)
DEF_STORAGE_CONVERTER(long long, IntegralType, 1)
DEF_STORAGE_CONVERTER(unsigned long long, UnsignedIntegralType, 1)
DEF_STORAGE_CONVERTER(float, FloatingType, 1)
DEF_STORAGE_CONVERTER(double, FloatingType, 1)
DEF_STORAGE_CONVERTER(glm::vec2, FloatingType, 2)
DEF_STORAGE_CONVERTER(glm::dvec2, FloatingType, 2)
DEF_STORAGE_CONVERTER(glm::ivec2, IntegralType, 2)
DEF_STORAGE_CONVERTER(glm::uvec2, UnsignedIntegralType, 2)
DEF_STORAGE_CONVERTER(glm::bvec2, IntegralType, 2)
DEF_STORAGE_CONVERTER(glm::vec3, FloatingType, 3)
DEF_STORAGE_CONVERTER(glm::dvec3, FloatingType, 3)
DEF_STORAGE_CONVERTER(glm::ivec3, IntegralType, 3)
DEF_STORAGE_CONVERTER(glm::uvec3, UnsignedIntegralType, 3)
DEF_STORAGE_CONVERTER(glm::bvec3, IntegralType, 3)
DEF_STORAGE_CONVERTER(glm::vec4, FloatingType, 4)
DEF_STORAGE_CONVERTER(glm::dvec4, FloatingType, 4)
DEF_STORAGE_CONVERTER(glm::ivec4, IntegralType, 4)
DEF_STORAGE_CONVERTER(glm::uvec4, UnsignedIntegralType, 4)
DEF_STORAGE_CONVERTER(glm::bvec4, IntegralType, 4)
DEF_STORAGE_CONVERTER(glm::mat2x2, FloatingType, 4)
DEF_STORAGE_CONVERTER(glm::mat2x3, FloatingType, 6)
DEF_STORAGE_CONVERTER(glm::mat2x4, FloatingType, 8)
DEF_STORAGE_CONVERTER(glm::mat3x2, FloatingType, 6)
DEF_STORAGE_CONVERTER(glm::mat3x3, FloatingType, 9)
DEF_STORAGE_CONVERTER(glm::mat3x4, FloatingType, 12)
DEF_STORAGE_CONVERTER(glm::mat4x2, FloatingType, 8)
DEF_STORAGE_CONVERTER(glm::mat4x3, FloatingType, 12)
DEF_STORAGE_CONVERTER(glm::mat4x4, FloatingType, 16)
DEF_STORAGE_CONVERTER(glm::dmat2x2, FloatingType, 4)
DEF_STORAGE_CONVERTER(glm::dmat2x3, FloatingType, 6)
DEF_STORAGE_CONVERTER(glm::dmat2x4, FloatingType, 8)
DEF_STORAGE_CONVERTER(glm::dmat3x2, FloatingType, 6)
DEF_STORAGE_CONVERTER(glm::dmat3x3, FloatingType, 9)
DEF_STORAGE_CONVERTER(glm::dmat3x4, FloatingType, 12)
DEF_STORAGE_CONVERTER(glm::dmat4x2, FloatingType, 8)
DEF_STORAGE_CONVERTER(glm::dmat4x3, FloatingType, 12)
DEF_STORAGE_CONVERTER(glm::dmat4x4, FloatingType, 16)

#undef DEF_STORAGE_CONVERTER

}

namespace ghoul {

#ifdef WIN32
#pragma warning(disable : 4800)
#endif

template <typename SOURCE, typename TARGET, size_t SIZE>
std::array<TARGET, SIZE> createArray(const SOURCE* const src) {
    std::array<TARGET, SIZE> v;
    for (size_t i = 0; i < SIZE; ++i)
        v[i] = static_cast<TARGET>(src[i]);
    return v;
}

template <typename TargetType>
bool isConvertible(const Dictionary& dict) {
    const bool correctSize = dict.size() == StorageTypeConverter<TargetType>::size;
    if (!correctSize)
        return false;

    const std::vector<std::string>& keys = dict.keys();
    for (size_t i = 0; i < StorageTypeConverter<TargetType>::size; ++i) {
        const std::string& key = keys[i];
#ifdef WIN32
#pragma warning(push)
    // Suppress the warning C2684 (Redundant test) that occurs if
    // StorageTypeConverter<TargetType>::type == TargetType
#pragma warning(suppress: 6287)
#endif // WIN32
        const bool correctType = dict.hasValue<typename StorageTypeConverter<TargetType>::type>(
                                       key) || dict.hasValue<TargetType>(key);
#ifdef WIN32
#pragma warning(pop)
#endif // WIN32
        if (!correctType)
            return false;
    }
    return true;
}

template <typename TargetType>
void convertGLM(const Dictionary& dict, TargetType& target) {
    std::vector<std::string>&& keys = dict.keys();
    // sort numerically rather than by ASCII value
    std::sort(keys.begin(), keys.end(), [](const std::string& k1, const std::string& k2) {
        try {
            return std::stoi(k1) < std::stoi(k2);
        } catch (std::invalid_argument&) {
            return k1 < k2;
        }
    });
    for (size_t i = 0; i < StorageTypeConverter<TargetType>::size; ++i) {
        const std::string& key = keys[i];
        dict.getValue(key, glm::value_ptr(target)[i]);
    }
}

template <typename TargetType>
void convert(const Dictionary& dict, TargetType& target) {
    static_assert(StorageTypeConverter<TargetType>::size == 1,
                  "Wrong function call. StorageType::size > 1");
    const std::vector<std::string>& keys = dict.keys();
    for (size_t i = 0; i < StorageTypeConverter<TargetType>::size; ++i) {
        const std::string& key = keys[i];
        dict.getValue(key, target);
    }
}

// Yes, all those functions could be replaced by a macro (and they were), but they are
// easier to read (and debug!) this way ---abock
template <>
bool Dictionary::hasValue<double>(const std::string& key) const {
    const bool val = hasValueHelper<StorageTypeConverter<double>::type>(key);
    if (val)
    return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<double>(dict);
            if (canConvert)
            return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<double>(std::string key, double value, bool 
                                  createIntermediate)
{
    return setValueHelper(
        std::move(key),
        StorageTypeConverter<double>::type(value),
        createIntermediate
    );
}

template <>
bool Dictionary::getValue<double>(const std::string& key, double& value) const {
    StorageTypeConverter<double>::type v;
    const bool success = hasValueHelper<StorageTypeConverter<double>::type>(key);
    if (success) {
        getValueHelper(key, v);
        value = static_cast<double>(v);
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<double>(dict);
            if (canConvert) {
                convert(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<long long>(const std::string& key) const {
    const bool val = hasValueHelper<StorageTypeConverter<long long>::type>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<long long>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<long long>(std::string key, long long value,
                                     bool createIntermediate)
{
    return setValueHelper(
        std::move(key),
        StorageTypeConverter<long long>::type(value),
        createIntermediate
    );
}

template <>
bool Dictionary::getValue<long long>(const std::string& key, long long& value) const {
    StorageTypeConverter<long long>::type v;
    const bool success = hasValueHelper<StorageTypeConverter<long long>::type>(key);
    if (success) {
        getValueHelper(key, v);
        value = static_cast<long long>(v);
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<long long>(dict);
            if (canConvert) {
                convert(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<unsigned long long>(const std::string& key) const {
    const bool val = hasValueHelper<StorageTypeConverter<unsigned long long>::type>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<unsigned long long>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<unsigned long long>(std::string key, unsigned long long value,
                                bool createIntermediate)
{
    return setValueHelper(
        std::move(key),
        StorageTypeConverter<unsigned long long>::type(value),
        createIntermediate
    );
}

template <>
bool Dictionary::getValue<unsigned long long>(const std::string& key,
                            unsigned long long& value) const
{
    StorageTypeConverter<unsigned long long>::type v;
    const bool success =
        hasValueHelper<StorageTypeConverter<unsigned long long>::type>(key);
    if (success) {
        getValueHelper(key, v);
        value = static_cast<unsigned long long>(v);
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<unsigned long long>(dict);
            if (canConvert) {
                convert(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<bool>(const std::string& key) const {
    const bool val = hasValueHelper<StorageTypeConverter<bool>::type>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<bool>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<bool>(std::string key, bool value,
                                bool createIntermediate)
{
    return setValueHelper(
        std::move(key),
        StorageTypeConverter<bool>::type(value),
        createIntermediate
    );
}

template <>
bool Dictionary::getValue<bool>(const std::string& key, bool& value) const {
    StorageTypeConverter<bool>::type v;
    const bool success = hasValueHelper<StorageTypeConverter<bool>::type>(key);
    if (success) {
        getValueHelper(key, v);
        value = static_cast<bool>(v);
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<bool>(dict);
            if (canConvert) {
                convert(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<char>(const std::string& key) const {
    const bool val = hasValueHelper<StorageTypeConverter<char>::type>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<char>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<char>(std::string key, char value,
                                bool createIntermediate)
{
    return setValueHelper(
        std::move(key),
        StorageTypeConverter<char>::type(value),
        createIntermediate
    );
}

template <>
bool Dictionary::getValue<char>(const std::string& key, char& value) const {
    StorageTypeConverter<char>::type v;
    const bool success = hasValueHelper<StorageTypeConverter<char>::type>(key);
    if (success) {
        getValueHelper(key, v);
        value = static_cast<char>(v);
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<char>(dict);
            if (canConvert) {
                convert(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<signed char>(const std::string& key) const {
    const bool val = hasValueHelper<StorageTypeConverter<signed char>::type>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<signed char>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<signed char>(std::string key, signed char value,
                                bool createIntermediate)
{
    return setValueHelper(
        std::move(key),
        StorageTypeConverter<signed char>::type(value),
        createIntermediate
    );
}

template <>
bool Dictionary::getValue<signed char>(const std::string& key, signed char& value) const {
    StorageTypeConverter<signed char>::type v;
    const bool success = hasValueHelper<StorageTypeConverter<signed char>::type>(key);
    if (success) {
        getValueHelper(key, v);
        value = static_cast<signed char>(v);
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<signed char>(dict);
            if (canConvert) {
                convert(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<unsigned char>(const std::string& key) const {
    const bool val = hasValueHelper<StorageTypeConverter<unsigned char>::type>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<unsigned char>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<unsigned char>(std::string key, unsigned char value,
                                bool createIntermediate)
{
    return setValueHelper(
        std::move(key),
        StorageTypeConverter<unsigned char>::type(value),
        createIntermediate
    );
}

template <>
bool Dictionary::getValue<unsigned char>(const std::string& key,
                                         unsigned char& value) const
{
    StorageTypeConverter<unsigned char>::type v;
    const bool success = hasValueHelper<StorageTypeConverter<unsigned char>::type>(key);
    if (success) {
        getValueHelper(key, v);
        value = static_cast<unsigned char>(v);
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<unsigned char>(dict);
            if (canConvert) {
                convert(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<wchar_t>(const std::string& key) const {
    const bool val = hasValueHelper<StorageTypeConverter<wchar_t>::type>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<wchar_t>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<wchar_t>(std::string key, wchar_t value,
                                bool createIntermediate)
{
    return setValueHelper(
        std::move(key),
        StorageTypeConverter<wchar_t>::type(value),
        createIntermediate
    );
}

template <>
bool Dictionary::getValue<wchar_t>(const std::string& key, wchar_t& value) const {
    StorageTypeConverter<wchar_t>::type v;
    const bool success = hasValueHelper<StorageTypeConverter<wchar_t>::type>(key);
    if (success) {
        getValueHelper(key, v);
        value = static_cast<wchar_t>(v);
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<wchar_t>(dict);
            if (canConvert) {
                convert(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<short>(const std::string& key) const {
    const bool val = hasValueHelper<StorageTypeConverter<short>::type>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<short>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<short>(std::string key, short value,
                                bool createIntermediate)
{
    return setValueHelper(
        std::move(key),
        StorageTypeConverter<short>::type(value),
        createIntermediate
    );
}

template <>
bool Dictionary::getValue<short>(const std::string& key, short& value) const {
    StorageTypeConverter<short>::type v;
    const bool success = hasValueHelper<StorageTypeConverter<short>::type>(key);
    if (success) {
        getValueHelper(key, v);
        value = static_cast<short>(v);
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<short>(dict);
            if (canConvert) {
                convert(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<unsigned short>(const std::string& key) const {
    const bool val = hasValueHelper<StorageTypeConverter<unsigned short>::type>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<unsigned short>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<unsigned short>(std::string key, unsigned short value,
                                bool createIntermediate)
{
    return setValueHelper(
        std::move(key),
        StorageTypeConverter<unsigned short>::type(value),
        createIntermediate
    );
}

template <>
bool Dictionary::getValue<unsigned short>(const std::string& key, 
                                          unsigned short& value) const
{
    StorageTypeConverter<unsigned short>::type v;
    const bool success = hasValueHelper<StorageTypeConverter<unsigned short>::type>(key);
    if (success) {
        getValueHelper(key, v);
        value = static_cast<unsigned short>(v);
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<unsigned short>(dict);
            if (canConvert) {
                convert(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<int>(const std::string& key) const {
    const bool val = hasValueHelper<StorageTypeConverter<int>::type>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<int>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<int>(std::string key, int value, bool createIntermediate) {
    return setValueHelper(
        std::move(key),
        StorageTypeConverter<int>::type(value),
        createIntermediate
    );
}

template <>
bool Dictionary::getValue<int>(const std::string& key, int& value) const {
    StorageTypeConverter<int>::type v;
    const bool success = hasValueHelper<StorageTypeConverter<int>::type>(key);
    if (success) {
        getValueHelper(key, v);
        value = static_cast<int>(v);
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<int>(dict);
            if (canConvert) {
                convert(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<unsigned int>(const std::string& key) const {
    const bool val = hasValueHelper<StorageTypeConverter<unsigned int>::type>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<unsigned int>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<unsigned int>(std::string key, unsigned int value,
                                bool createIntermediate)
{
    return setValueHelper(
        std::move(key),
        StorageTypeConverter<unsigned int>::type(value),
        createIntermediate
    );
}

template <>
bool Dictionary::getValue<unsigned int>(const std::string& key,
                                        unsigned int& value) const
{
    StorageTypeConverter<unsigned int>::type v;
    const bool success = hasValueHelper<StorageTypeConverter<unsigned int>::type>(key);
    if (success) {
        getValueHelper(key, v);
        value = static_cast<unsigned int>(v);
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<unsigned int>(dict);
            if (canConvert) {
                convert(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<float>(const std::string& key) const {
    const bool val = hasValueHelper<StorageTypeConverter<float>::type>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<float>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<float>(std::string key, float value, bool createIntermediate) {
    return setValueHelper(
        std::move(key),
        StorageTypeConverter<float>::type(value),
        createIntermediate
    );
}

template <>
bool Dictionary::getValue<float>(const std::string& key, float& value) const {
    StorageTypeConverter<float>::type v;
    const bool success = hasValueHelper<StorageTypeConverter<float>::type>(key);
    if (success) {
        getValueHelper(key, v);
        value = static_cast<float>(v);
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<float>(dict);
            if (canConvert) {
                convert(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::vec2>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::vec2>::type,
                                        StorageTypeConverter<glm::vec2>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::vec2>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::vec2>(std::string key, glm::vec2 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::vec2::value_type,
        StorageTypeConverter<glm::vec2>::type,
        StorageTypeConverter<glm::vec2>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::vec2>(const std::string& key, glm::vec2& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::vec2>::type,
                                        StorageTypeConverter<glm::vec2>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::vec2>::type,
                   StorageTypeConverter<glm::vec2>::size> v;
        getValueHelper(key, v);
        value = glm::make_vec2(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::vec2>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::dvec2>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::dvec2>::type,
                                        StorageTypeConverter<glm::dvec2>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::dvec2>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::dvec2>(std::string key, glm::dvec2 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::dvec2::value_type,
        StorageTypeConverter<glm::dvec2>::type,
        StorageTypeConverter<glm::dvec2>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::dvec2>(const std::string& key, glm::dvec2& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::dvec2>::type,
                                        StorageTypeConverter<glm::dvec2>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::dvec2>::type,
                   StorageTypeConverter<glm::dvec2>::size> v;
        getValueHelper(key, v);
        value = glm::make_vec2(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::dvec2>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::ivec2>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::ivec2>::type,
                                        StorageTypeConverter<glm::ivec2>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::ivec2>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::ivec2>(std::string key, glm::ivec2 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::ivec2::value_type,
        StorageTypeConverter<glm::ivec2>::type,
        StorageTypeConverter<glm::ivec2>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::ivec2>(const std::string& key, glm::ivec2& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::ivec2>::type,
                                        StorageTypeConverter<glm::ivec2>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::ivec2>::type,
                   StorageTypeConverter<glm::ivec2>::size> v;
        getValueHelper(key, v);
        value = glm::make_vec2(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::ivec2>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::uvec2>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::uvec2>::type,
                                        StorageTypeConverter<glm::uvec2>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::uvec2>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::uvec2>(std::string key, glm::uvec2 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::uvec2::value_type,
        StorageTypeConverter<glm::uvec2>::type,
        StorageTypeConverter<glm::uvec2>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::uvec2>(const std::string& key, glm::uvec2& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::uvec2>::type,
                                        StorageTypeConverter<glm::uvec2>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::uvec2>::type,
                   StorageTypeConverter<glm::uvec2>::size> v;
        getValueHelper(key, v);
        value = glm::make_vec2(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::uvec2>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::bvec2>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::bvec2>::type,
                                        StorageTypeConverter<glm::bvec2>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::bvec2>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::bvec2>(std::string key, glm::bvec2 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::bvec2::value_type,
        StorageTypeConverter<glm::bvec2>::type,
        StorageTypeConverter<glm::bvec2>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::bvec2>(const std::string& key, glm::bvec2& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::bvec2>::type,
                                        StorageTypeConverter<glm::bvec2>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::bvec2>::type,
                   StorageTypeConverter<glm::bvec2>::size> v;
        getValueHelper(key, v);
        value = glm::make_vec2(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::bvec2>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::vec3>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::vec3>::type,
                                        StorageTypeConverter<glm::vec3>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::vec3>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::vec3>(std::string key, glm::vec3 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::vec3::value_type,
        StorageTypeConverter<glm::vec3>::type,
        StorageTypeConverter<glm::vec3>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::vec3>(const std::string& key, glm::vec3& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::vec3>::type,
                                        StorageTypeConverter<glm::vec3>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::vec3>::type,
                   StorageTypeConverter<glm::vec3>::size> v;
        getValueHelper(key, v);
        value = glm::make_vec3(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::vec3>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::dvec3>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::dvec3>::type,
                                        StorageTypeConverter<glm::dvec3>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::dvec3>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::dvec3>(std::string key, glm::dvec3 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::dvec3::value_type,
        StorageTypeConverter<glm::dvec3>::type,
        StorageTypeConverter<glm::dvec3>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::dvec3>(const std::string& key, glm::dvec3& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::dvec3>::type,
                                        StorageTypeConverter<glm::dvec3>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::dvec3>::type,
                   StorageTypeConverter<glm::dvec3>::size> v;
        getValueHelper(key, v);
        value = glm::make_vec3(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::dvec3>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::ivec3>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::ivec3>::type,
                                        StorageTypeConverter<glm::ivec3>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::ivec3>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::ivec3>(std::string key, glm::ivec3 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::ivec3::value_type,
        StorageTypeConverter<glm::ivec3>::type,
        StorageTypeConverter<glm::ivec3>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::ivec3>(const std::string& key, glm::ivec3& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::ivec3>::type,
                                        StorageTypeConverter<glm::ivec3>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::ivec3>::type,
                   StorageTypeConverter<glm::ivec3>::size> v;
        getValueHelper(key, v);
        value = glm::make_vec3(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::ivec3>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::uvec3>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::uvec3>::type,
                                        StorageTypeConverter<glm::uvec3>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::uvec3>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::uvec3>(std::string key, glm::uvec3 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::uvec3::value_type,
        StorageTypeConverter<glm::uvec3>::type,
        StorageTypeConverter<glm::uvec3>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::uvec3>(const std::string& key, glm::uvec3& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::uvec3>::type,
                                        StorageTypeConverter<glm::uvec3>::size>>(key);
    if (success) {
        std::array < StorageTypeConverter<glm::uvec3>::type,
                    StorageTypeConverter<glm::uvec3>::size > v;
        getValueHelper(key, v);
        value = glm::make_vec3(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::uvec3>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::bvec3>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::bvec3>::type,
                                        StorageTypeConverter<glm::bvec3>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::bvec3>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::bvec3>(std::string key, glm::bvec3 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::bvec3::value_type,
        StorageTypeConverter<glm::bvec3>::type,
        StorageTypeConverter<glm::bvec3>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::bvec3>(const std::string& key, glm::bvec3& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::bvec3>::type,
                                        StorageTypeConverter<glm::bvec3>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::bvec3>::type,
                   StorageTypeConverter<glm::bvec3>::size> v;
        getValueHelper(key, v);
        value = glm::make_vec3(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::bvec3>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::vec4>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::vec4>::type,
                                        StorageTypeConverter<glm::vec4>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::vec4>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::vec4>(std::string key, glm::vec4 value,
                                bool createIntermediate) {
    auto v = createArray<
        glm::vec4::value_type,
        StorageTypeConverter<glm::vec4>::type,
        StorageTypeConverter<glm::vec4>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::vec4>(const std::string& key, glm::vec4& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::vec4>::type,
                                        StorageTypeConverter<glm::vec4>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::vec4>::type,
                   StorageTypeConverter<glm::vec4>::size> v;
        getValueHelper(key, v);
        value = glm::make_vec4(v.data());
        return success;
    } else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::vec4>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::dvec4>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::dvec4>::type,
                                        StorageTypeConverter<glm::dvec4>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::dvec4>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::dvec4>(std::string key, glm::dvec4 value,
                                bool createIntermediate) {
    auto v = createArray<
        glm::dvec4::value_type,
        StorageTypeConverter<glm::dvec4>::type,
        StorageTypeConverter<glm::dvec4>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::dvec4>(const std::string& key, glm::dvec4& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::dvec4>::type,
                                        StorageTypeConverter<glm::dvec4>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::dvec4>::type,
                   StorageTypeConverter<glm::dvec4>::size> v;
        getValueHelper(key, v);
        value = glm::make_vec4(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::dvec4>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::ivec4>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::ivec4>::type,
                                        StorageTypeConverter<glm::ivec4>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::ivec4>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::ivec4>(std::string key, glm::ivec4 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::ivec4::value_type,
        StorageTypeConverter<glm::ivec4>::type,
        StorageTypeConverter<glm::ivec4>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::ivec4>(const std::string& key, glm::ivec4& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::ivec4>::type,
                                        StorageTypeConverter<glm::ivec4>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::ivec4>::type,
                   StorageTypeConverter<glm::ivec4>::size> v;
        getValueHelper(key, v);
        value = glm::make_vec4(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::ivec4>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::uvec4>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::uvec4>::type,
                                        StorageTypeConverter<glm::uvec4>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::uvec4>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::uvec4>(std::string key, glm::uvec4 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::uvec4::value_type,
        StorageTypeConverter<glm::uvec4>::type,
        StorageTypeConverter<glm::uvec4>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::uvec4>(const std::string& key, glm::uvec4& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::uvec4>::type,
                                        StorageTypeConverter<glm::uvec4>::size>>(key);
    if (success) {
        std::array < StorageTypeConverter<glm::uvec4>::type,
                    StorageTypeConverter<glm::uvec4>::size > v;
        getValueHelper(key, v);
        value = glm::make_vec4(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::uvec4>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::bvec4>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::bvec4>::type,
                                        StorageTypeConverter<glm::bvec4>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::bvec4>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::bvec4>(std::string key, glm::bvec4 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::bvec4::value_type,
        StorageTypeConverter<glm::bvec4>::type,
        StorageTypeConverter<glm::bvec4>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::bvec4>(const std::string& key, glm::bvec4& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::bvec4>::type,
                                        StorageTypeConverter<glm::bvec4>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::bvec4>::type,
                   StorageTypeConverter<glm::bvec4>::size> v;
        getValueHelper(key, v);
        value = glm::make_vec4(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::bvec4>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::mat2x2>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::mat2x2>::type,
                                        StorageTypeConverter<glm::mat2x2>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::mat2x2>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::mat2x2>(std::string key, glm::mat2x2 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::mat2x2::value_type,
        StorageTypeConverter<glm::mat2x2>::type,
        StorageTypeConverter<glm::mat2x2>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::mat2x2>(const std::string& key, glm::mat2x2& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::mat2x2>::type,
                                        StorageTypeConverter<glm::mat2x2>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::mat2x2>::type,
                    StorageTypeConverter<glm::mat2x2>::size> v;
        getValueHelper(key, v);
        value = glm::make_mat2x2(v.data());
        return success;
    } else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::mat2x2>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::mat2x3>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::mat2x3>::type,
                                        StorageTypeConverter<glm::mat2x3>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::mat2x3>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::mat2x3>(std::string key, glm::mat2x3 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::mat2x3::value_type,
        StorageTypeConverter<glm::mat2x3>::type,
        StorageTypeConverter<glm::mat2x3>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::mat2x3>(const std::string& key, glm::mat2x3& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::mat2x3>::type,
                                        StorageTypeConverter<glm::mat2x3>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::mat2x3>::type,
                   StorageTypeConverter<glm::mat2x3>::size> v;
        getValueHelper(key, v);
        value = glm::make_mat2x3(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::mat2x3>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::mat2x4>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::mat2x4>::type,
                                        StorageTypeConverter<glm::mat2x4>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::mat2x4>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::mat2x4>(std::string key, glm::mat2x4 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::mat2x4::value_type,
        StorageTypeConverter<glm::mat2x4>::type,
        StorageTypeConverter<glm::mat2x4>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::mat2x4>(const std::string& key, glm::mat2x4& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::mat2x4>::type,
                                        StorageTypeConverter<glm::mat2x4>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::mat2x4>::type,
                   StorageTypeConverter<glm::mat2x4>::size> v;
        getValueHelper(key, v);
        value = glm::make_mat2x4(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::mat2x4>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::mat3x2>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::mat3x2>::type,
                                        StorageTypeConverter<glm::mat3x2>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::mat3x2>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::mat3x2>(std::string key, glm::mat3x2 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::mat3x2::value_type,
        StorageTypeConverter<glm::mat3x2>::type,
        StorageTypeConverter<glm::mat3x2>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::mat3x2>(const std::string& key, glm::mat3x2& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::mat3x2>::type,
                                        StorageTypeConverter<glm::mat3x2>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::mat3x2>::type,
                   StorageTypeConverter<glm::mat3x2>::size> v;
        getValueHelper(key, v);
        value = glm::make_mat3x2(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::mat3x2>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::mat3x3>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::mat3x3>::type,
                                        StorageTypeConverter<glm::mat3x3>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::mat3x3>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::mat3x3>(std::string key, glm::mat3x3 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::mat3x3::value_type,
        StorageTypeConverter<glm::mat3x3>::type,
        StorageTypeConverter<glm::mat3x3>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::mat3x3>(const std::string& key, glm::mat3x3& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::mat3x3>::type,
                                        StorageTypeConverter<glm::mat3x3>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::mat3x3>::type,
                   StorageTypeConverter<glm::mat3x3>::size> v;
        getValueHelper(key, v);
        value = glm::make_mat3x3(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::mat3x3>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::mat3x4>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::mat3x4>::type,
                                        StorageTypeConverter<glm::mat3x4>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::mat3x4>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::mat3x4>(std::string key, glm::mat3x4 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::mat3x4::value_type,
        StorageTypeConverter<glm::mat3x4>::type,
        StorageTypeConverter<glm::mat3x4>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::mat3x4>(const std::string& key, glm::mat3x4& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::mat3x4>::type,
                                        StorageTypeConverter<glm::mat3x4>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::mat3x4>::type,
                   StorageTypeConverter<glm::mat3x4>::size> v;
        getValueHelper(key, v);
        value = glm::make_mat3x4(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::mat3x4>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::mat4x2>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::mat4x2>::type,
                                        StorageTypeConverter<glm::mat4x2>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::mat4x2>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::mat4x2>(std::string key, glm::mat4x2 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::mat4x2::value_type,
        StorageTypeConverter<glm::mat4x2>::type,
        StorageTypeConverter<glm::mat4x2>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::mat4x2>(const std::string& key, glm::mat4x2& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::mat4x2>::type,
                                        StorageTypeConverter<glm::mat4x2>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::mat4x2>::type,
                   StorageTypeConverter<glm::mat4x2>::size> v;
        getValueHelper(key, v);
        value = glm::make_mat4x2(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::mat4x2>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::mat4x3>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::mat4x3>::type,
                                        StorageTypeConverter<glm::mat4x3>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::mat4x3>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::mat4x3>(std::string key, glm::mat4x3 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::mat4x3::value_type,
        StorageTypeConverter<glm::mat4x3>::type,
        StorageTypeConverter<glm::mat4x3>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::mat4x3>(const std::string& key, glm::mat4x3& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::mat4x3>::type,
                                        StorageTypeConverter<glm::mat4x3>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::mat4x3>::type,
                   StorageTypeConverter<glm::mat4x3>::size> v;
        getValueHelper(key, v);
        value = glm::make_mat4x3(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::mat4x3>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::mat4x4>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::mat4x4>::type,
                                        StorageTypeConverter<glm::mat4x4>::size>> (key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::mat4x4>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::mat4x4>(std::string key, glm::mat4x4 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::mat4x4::value_type,
        StorageTypeConverter<glm::mat4x4>::type,
        StorageTypeConverter<glm::mat4x4>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::mat4x4>(const std::string& key, glm::mat4x4& value) const {
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::mat4x4>::type,
                                        StorageTypeConverter<glm::mat4x4>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::mat4x4>::type,
                   StorageTypeConverter<glm::mat4x4>::size> v;
        getValueHelper(key, v);
        value = glm::make_mat4x4(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::mat4x4>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::dmat2x2>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::dmat2x2>::type,
                                        StorageTypeConverter<glm::dmat2x2>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::dmat2x2>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::dmat2x2>(std::string key, glm::dmat2x2 value,
                                        bool createIntermediate)
{
    auto v = createArray<
        glm::dmat2x2::value_type,
        StorageTypeConverter<glm::dmat2x2>::type,
        StorageTypeConverter<glm::dmat2x2>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::dmat2x2>(const std::string& key,
                                        glm::dmat2x2& value) const
{
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::dmat2x2>::type,
                                        StorageTypeConverter<glm::dmat2x2>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::dmat2x2>::type,
                   StorageTypeConverter<glm::dmat2x2>::size> v;
        getValueHelper(key, v);
        value = glm::make_mat2x2(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::dmat2x2>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::dmat2x3>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::dmat2x3>::type,
                                        StorageTypeConverter<glm::dmat2x3>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::dmat2x3>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::dmat2x3>(std::string key, glm::dmat2x3 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::dmat2x3::value_type,
        StorageTypeConverter<glm::dmat2x3>::type,
        StorageTypeConverter<glm::dmat2x3>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::dmat2x3>(const std::string& key,
                                        glm::dmat2x3& value) const
{
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::dmat2x3>::type,
                                        StorageTypeConverter<glm::dmat2x3>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::dmat2x3>::type,
                   StorageTypeConverter<glm::dmat2x3>::size> v;
        getValueHelper(key, v);
        value = glm::make_mat2x3(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::dmat2x3>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::dmat2x4>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::dmat2x4>::type,
                                        StorageTypeConverter<glm::dmat2x4>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::dmat2x4>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::dmat2x4>(std::string key, glm::dmat2x4 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::dmat2x4::value_type,
        StorageTypeConverter<glm::dmat2x4>::type,
        StorageTypeConverter<glm::dmat2x4>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::dmat2x4>(const std::string& key,
                                        glm::dmat2x4& value) const
{
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::dmat2x4>::type,
                                        StorageTypeConverter<glm::dmat2x4>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::dmat2x4>::type,
                   StorageTypeConverter<glm::dmat2x4>::size> v;
        getValueHelper(key, v);
        value = glm::make_mat2x4(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::dmat2x4>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::dmat3x2>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::dmat3x2>::type,
                                        StorageTypeConverter<glm::dmat3x2>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::dmat3x2>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::dmat3x2>(std::string key, glm::dmat3x2 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::dmat3x2::value_type,
        StorageTypeConverter<glm::dmat3x2>::type,
        StorageTypeConverter<glm::dmat3x2>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::dmat3x2>(const std::string& key,
                                        glm::dmat3x2& value) const
{
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::dmat3x2>::type,
                                        StorageTypeConverter<glm::dmat3x2>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::dmat3x2>::type,
                   StorageTypeConverter<glm::dmat3x2>::size> v;
        getValueHelper(key, v);
        value = glm::make_mat3x2(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::dmat3x2>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::dmat3x3>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::dmat3x3>::type,
                                        StorageTypeConverter<glm::dmat3x3>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::dmat3x3>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::dmat3x3>(std::string key, glm::dmat3x3 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::dmat3x3::value_type,
        StorageTypeConverter<glm::dmat3x3>::type,
        StorageTypeConverter<glm::dmat3x3>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::dmat3x3>(const std::string& key,
                                        glm::dmat3x3& value) const
{
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::dmat3x3>::type,
                                        StorageTypeConverter<glm::dmat3x3>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::dmat3x3>::type,
                   StorageTypeConverter<glm::dmat3x3>::size> v;
        getValueHelper(key, v);
        value = glm::make_mat3x3(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::dmat3x3>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::dmat3x4>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::dmat3x4>::type,
                                        StorageTypeConverter<glm::dmat3x4>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::dmat3x4>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::dmat3x4>(std::string key, glm::dmat3x4 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::dmat3x4::value_type,
        StorageTypeConverter<glm::dmat3x4>::type,
        StorageTypeConverter<glm::dmat3x4>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::dmat3x4>(const std::string& key,
                                        glm::dmat3x4& value) const
{
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::dmat3x4>::type,
                                        StorageTypeConverter<glm::dmat3x4>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::dmat3x4>::type,
                   StorageTypeConverter<glm::dmat3x4>::size> v;
        getValueHelper(key, v);
        value = glm::make_mat3x4(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::dmat3x4>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::dmat4x2>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::dmat4x2>::type,
                                        StorageTypeConverter<glm::dmat4x2>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::dmat4x2>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::dmat4x2>(std::string key, glm::dmat4x2 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::dmat4x2::value_type,
        StorageTypeConverter<glm::dmat4x2>::type,
        StorageTypeConverter<glm::dmat4x2>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::dmat4x2>(const std::string& key,
                                        glm::dmat4x2& value) const
{
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::dmat4x2>::type,
                                        StorageTypeConverter<glm::dmat4x2>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::dmat4x2>::type,
                   StorageTypeConverter<glm::dmat4x2>::size> v;
        getValueHelper(key, v);
        value = glm::make_mat4x2(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::dmat4x2>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::dmat4x3>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::dmat4x3>::type,
                                        StorageTypeConverter<glm::dmat4x3>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::dmat4x3>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::dmat4x3>(std::string key, glm::dmat4x3 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::dmat4x3::value_type,
        StorageTypeConverter<glm::dmat4x3>::type,
        StorageTypeConverter<glm::dmat4x3>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::dmat4x3>(const std::string& key,
                                        glm::dmat4x3& value) const
{
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::dmat4x3>::type,
                                        StorageTypeConverter<glm::dmat4x3>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::dmat4x3>::type,
                   StorageTypeConverter<glm::dmat4x3>::size> v;
        getValueHelper(key, v);
        value = glm::make_mat4x3(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::dmat4x3>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::hasValue<glm::dmat4x4>(const std::string& key) const {
    const bool val
            = hasValueHelper<std::array<StorageTypeConverter<glm::dmat4x4>::type,
                                        StorageTypeConverter<glm::dmat4x4>::size>>(key);
    if (val)
        return true;
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper(key, dict);
            const bool canConvert = isConvertible<glm::dmat4x4>(dict);
            if (canConvert)
                return true;
        }
        return false;
    }
}

template <>
bool Dictionary::setValue<glm::dmat4x4>(std::string key, glm::dmat4x4 value,
                                bool createIntermediate)
{
    auto v = createArray<
        glm::dmat4x4::value_type,
        StorageTypeConverter<glm::dmat4x4>::type,
        StorageTypeConverter<glm::dmat4x4>::size>(glm::value_ptr(value));
    return setValueHelper(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<glm::dmat4x4>(const std::string& key,
                                        glm::dmat4x4& value) const
{
    bool success
            = hasValueHelper<std::array<StorageTypeConverter<glm::dmat4x4>::type,
                                        StorageTypeConverter<glm::dmat4x4>::size>>(key);
    if (success) {
        std::array<StorageTypeConverter<glm::dmat4x4>::type,
                   StorageTypeConverter<glm::dmat4x4>::size> v;
        getValueHelper(key, v);
        value = glm::make_mat4x4(v.data());
        return success;
    }
    else {
        const bool hasDictionary = hasValueHelper<Dictionary>(key);
        if (hasDictionary) {
            Dictionary dict;
            getValueHelper<Dictionary>(key, dict);
            const bool canConvert = isConvertible<glm::dmat4x4>(dict);
            if (canConvert) {
                convertGLM(dict, value);
                return true;
            }
        }
    }
    return false;
}

template <>
bool Dictionary::getValue<Dictionary>(const std::string& key, Dictionary& value) const {
    if (&value == this) {
        LERROR(
              "The argument in the 'getValue' methods cannot be the same Dictionary as "
              "the method is called on");
        return false;
    }
    return getValueHelper(key, value);
}

#ifdef WIN32
#pragma warning ( default : 4800 )
#endif

Dictionary::Dictionary(std::initializer_list<std::pair<std::string, boost::any>> l) {
    for (const auto& p : l)
        setValueAnyHelper(std::move(p.first), std::move(p.second));
}

std::vector<string> Dictionary::keys(const string& location) const {
    if (location.empty()) {
        std::vector<string> result;
        result.reserve(size());
        for (const auto& it : *this)
            result.push_back(it.first);
        return result;
    }

    std::string first;
    std::string rest;
    splitKey(location, first, rest);

    auto keyIt = find(first);
    if (keyIt == cend()) {
        LERROR("Key '" << first << "' was not found in dictionary");
        return std::vector<string>();
    }

    const Dictionary* const dict = boost::any_cast<Dictionary>(&(keyIt->second));
    if (!dict) {
        LERROR("Error converting key '" << first << "' to type 'Dictionary', was '"
                                        << keyIt->second.type().name() << "'");
        return std::vector<string>();
    }
    // proper tail-recursion
    return dict->keys(rest);
}

bool Dictionary::hasKey(const string& key) const {
    auto it = find(key);
    if (it != cend())
        return true;

    std::string first;
    std::string rest;
    splitKey(key, first, rest);

    auto keyIt = find(first);
    if (keyIt == cend())
        return false;

    const Dictionary* const dict = boost::any_cast<Dictionary>(&(keyIt->second));
    if (!dict)
        return false;
    return dict->hasKey(rest);
}

size_t Dictionary::size() const {
    return std::map<std::string, boost::any>::size();
}

void Dictionary::clear() {
    return std::map<std::string, boost::any>::clear();
}

bool Dictionary::empty() const {
    return std::map<std::string, boost::any>::empty();
}

bool Dictionary::removeKey(const std::string& key) {
    std::map<std::string, boost::any>::size_type res = erase(key);
    return (res == 1);
}

bool Dictionary::splitKey(const string& key, string& first, string& rest) const {
    const string::size_type l = key.find('.');

    if (l == string::npos) {
        first = key;
        return false;
    } else {
        first = key.substr(0, l);
        rest = key.substr(l + 1);
        return true;
    }
}

void Dictionary::setValueAnyHelper(std::string key, boost::any value) {
    // Ugly if-else statement is necessary as 'type' cannot be not constexpr
    const std::type_info& type = value.type();

    if (type == typeid(bool))
        setValue(std::move(key), std::move(boost::any_cast<bool>(value)));
    else if (type == typeid(char))
        setValue(std::move(key), std::move(boost::any_cast<char>(value)));
    else if (type == typeid(signed char))
        setValue(std::move(key), std::move(boost::any_cast<signed char>(value)));
    else if (type == typeid(unsigned char))
        setValue(std::move(key), std::move(boost::any_cast<unsigned char>(value)));
    else if (type == typeid(wchar_t))
        setValue(std::move(key), std::move(boost::any_cast<wchar_t>(value)));
    else if (type == typeid(short))
        setValue(std::move(key), std::move(boost::any_cast<short>(value)));
    else if (type == typeid(unsigned short))
        setValue(std::move(key), std::move(boost::any_cast<unsigned short>(value)));
    else if (type == typeid(int))
        setValue(std::move(key), std::move(boost::any_cast<int>(value)));
    else if (type == typeid(unsigned int))
        setValue(std::move(key), std::move(boost::any_cast<unsigned int>(value)));
    else if (type == typeid(long long))
        setValue(std::move(key), std::move(boost::any_cast<long long>(value)));
    else if (type == typeid(unsigned long long))
        setValue(std::move(key), std::move(boost::any_cast<unsigned long long>(value)));
    else if (type == typeid(float))
        setValue(std::move(key), std::move(boost::any_cast<float>(value)));
    else if (type == typeid(glm::vec2))
        setValue(std::move(key), std::move(boost::any_cast<glm::vec2>(value)));
    else if (type == typeid(glm::dvec2))
        setValue(std::move(key), std::move(boost::any_cast<glm::dvec2>(value)));
    else if (type == typeid(glm::ivec2))
        setValue(std::move(key), std::move(boost::any_cast<glm::ivec2>(value)));
    else if (type == typeid(glm::uvec2))
        setValue(std::move(key), std::move(boost::any_cast<glm::uvec2>(value)));
    else if (type == typeid(glm::bvec2))
        setValue(std::move(key), std::move(boost::any_cast<glm::bvec2>(value)));
    else if (type == typeid(glm::vec3))
        setValue(std::move(key), std::move(boost::any_cast<glm::vec3>(value)));
    else if (type == typeid(glm::dvec3))
        setValue(std::move(key), std::move(boost::any_cast<glm::dvec3>(value)));
    else if (type == typeid(glm::ivec3))
        setValue(std::move(key), std::move(boost::any_cast<glm::ivec3>(value)));
    else if (type == typeid(glm::uvec3))
        setValue(std::move(key), std::move(boost::any_cast<glm::uvec3>(value)));
    else if (type == typeid(glm::bvec3))
        setValue(std::move(key), std::move(boost::any_cast<glm::bvec3>(value)));
    else if (type == typeid(glm::vec4))
        setValue(std::move(key), std::move(boost::any_cast<glm::vec4>(value)));
    else if (type == typeid(glm::dvec4))
        setValue(std::move(key), std::move(boost::any_cast<glm::dvec4>(value)));
    else if (type == typeid(glm::ivec4))
        setValue(std::move(key), std::move(boost::any_cast<glm::ivec4>(value)));
    else if (type == typeid(glm::uvec4))
        setValue(std::move(key), std::move(boost::any_cast<glm::uvec4>(value)));
    else if (type == typeid(glm::bvec4))
        setValue(std::move(key), std::move(boost::any_cast<glm::bvec4>(value)));
    else if (type == typeid(glm::mat2x2))
        setValue(std::move(key), std::move(boost::any_cast<glm::mat2x2>(value)));
    else if (type == typeid(glm::mat2x3))
        setValue(std::move(key), std::move(boost::any_cast<glm::mat2x3>(value)));
    else if (type == typeid(glm::mat2x4))
        setValue(std::move(key), std::move(boost::any_cast<glm::mat2x4>(value)));
    else if (type == typeid(glm::mat3x2))
        setValue(std::move(key), std::move(boost::any_cast<glm::mat3x2>(value)));
    else if (type == typeid(glm::mat3x3))
        setValue(std::move(key), std::move(boost::any_cast<glm::mat3x3>(value)));
    else if (type == typeid(glm::mat3x4))
        setValue(std::move(key), std::move(boost::any_cast<glm::mat3x4>(value)));
    else if (type == typeid(glm::mat4x2))
        setValue(std::move(key), std::move(boost::any_cast<glm::mat4x2>(value)));
    else if (type == typeid(glm::mat4x3))
        setValue(std::move(key), std::move(boost::any_cast<glm::mat4x3>(value)));
    else if (type == typeid(glm::mat4x4))
        setValue(std::move(key), std::move(boost::any_cast<glm::mat4x4>(value)));
    else if (type == typeid(glm::dmat2x2))
        setValue(std::move(key), std::move(boost::any_cast<glm::dmat2x2>(value)));
    else if (type == typeid(glm::dmat2x3))
        setValue(std::move(key), std::move(boost::any_cast<glm::dmat2x3>(value)));
    else if (type == typeid(glm::dmat2x4))
        setValue(std::move(key), std::move(boost::any_cast<glm::dmat2x4>(value)));
    else if (type == typeid(glm::dmat3x2))
        setValue(std::move(key), std::move(boost::any_cast<glm::dmat3x2>(value)));
    else if (type == typeid(glm::dmat3x3))
        setValue(std::move(key), std::move(boost::any_cast<glm::dmat3x3>(value)));
    else if (type == typeid(glm::dmat3x4))
        setValue(std::move(key), std::move(boost::any_cast<glm::dmat3x4>(value)));
    else if (type == typeid(glm::dmat4x2))
        setValue(std::move(key), std::move(boost::any_cast<glm::dmat4x2>(value)));
    else if (type == typeid(glm::dmat4x3))
        setValue(std::move(key), std::move(boost::any_cast<glm::dmat4x3>(value)));
    else if (type == typeid(glm::dmat4x4))
        setValue(std::move(key), std::move(boost::any_cast<glm::dmat4x4>(value)));
    else
        setValue(key, value, false);
}

}  // namespace ghoul
