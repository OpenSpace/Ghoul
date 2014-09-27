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

#include "misc/dictionary.h"

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
        const bool correctType = dict.hasValue<typename StorageTypeConverter<TargetType>::type>(
                                       key) || dict.hasValue<TargetType>(key);
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

#define DEF_SPEC_TEMPLATE(TYPE)                                                          \
    template <>                                                                          \
    bool Dictionary::hasValue<TYPE>(const std::string& key) const {                      \
        const bool val = hasValueHelper<StorageTypeConverter<TYPE>::type>(key);          \
        if (val)                                                                         \
            return true;                                                                 \
        else {                                                                           \
            const bool hasDictionary = hasValueHelper<Dictionary>(key);                  \
            if (hasDictionary) {                                                         \
                Dictionary dict;                                                         \
                getValueHelper(key, dict);                                               \
                const bool canConvert = isConvertible<TYPE>(dict);                       \
                if (canConvert)                                                          \
                    return true;                                                         \
            }                                                                            \
            return false;                                                                \
        }                                                                                \
    }                                                                                    \
    template <>                                                                          \
    bool Dictionary::setValue<TYPE>(std::string key, TYPE value,                         \
                                    bool createIntermediate) {                           \
        return setValueHelper(std::move(key), StorageTypeConverter<TYPE>::type(value),   \
                              createIntermediate);                                       \
    }                                                                                    \
    template <>                                                                          \
    bool Dictionary::getValue<TYPE>(const std::string& key, TYPE& value) const {         \
        StorageTypeConverter<TYPE>::type v;                                              \
        const bool success = hasValueHelper<StorageTypeConverter<TYPE>::type>(key);      \
        if (success) {                                                                   \
            getValueHelper(key, v);                                                      \
            value = static_cast<TYPE>(v);                                                \
            return success;                                                              \
        } else {                                                                         \
            const bool hasDictionary = hasValueHelper<Dictionary>(key);                  \
            if (hasDictionary) {                                                         \
                Dictionary dict;                                                         \
                getValueHelper(key, dict);                                               \
                const bool canConvert = isConvertible<TYPE>(dict);                       \
                if (canConvert) {                                                        \
                    convert(dict, value);                                                \
                    return true;                                                         \
                }                                                                        \
            }                                                                            \
        }                                                                                \
        return false;                                                                    \
    }

#define DEF_SPEC_TEMPLATE_GLM(TYPE, CREATE)                                              \
    template <>                                                                          \
    bool Dictionary::hasValue<TYPE>(const std::string& key) const {                      \
        const bool val                                                                   \
              = hasValueHelper<std::array<StorageTypeConverter<TYPE>::type,              \
                                          StorageTypeConverter<TYPE>::size>>(key);       \
        if (val)                                                                         \
            return true;                                                                 \
        else {                                                                           \
            const bool hasDictionary = hasValueHelper<Dictionary>(key);                  \
            if (hasDictionary) {                                                         \
                Dictionary dict;                                                         \
                getValueHelper(key, dict);                                               \
                const bool canConvert = isConvertible<TYPE>(dict);                       \
                if (canConvert)                                                          \
                    return true;                                                         \
            }                                                                            \
            return false;                                                                \
        }                                                                                \
    }                                                                                    \
    template <>                                                                          \
    bool Dictionary::setValue<TYPE>(std::string key, TYPE value,                         \
                                    bool createIntermediate) {                           \
        auto v = createArray<TYPE::value_type, StorageTypeConverter<TYPE>::type,         \
                             StorageTypeConverter<TYPE>::size>(glm::value_ptr(value));   \
        return setValueHelper(std::move(key), std::move(v), createIntermediate);         \
    }                                                                                    \
    template <>                                                                          \
    bool Dictionary::getValue<TYPE>(const std::string& key, TYPE& value) const {         \
        bool success                                                                     \
              = hasValueHelper<std::array<StorageTypeConverter<TYPE>::type,              \
                                          StorageTypeConverter<TYPE>::size>>(key);       \
        if (success) {                                                                   \
            std::array<StorageTypeConverter<TYPE>::type,                                 \
                       StorageTypeConverter<TYPE>::size> v;                              \
            getValueHelper(key, v);                                                      \
            value = CREATE(v.data());                                                    \
            return success;                                                              \
        } else {                                                                         \
            const bool hasDictionary = hasValueHelper<Dictionary>(key);                  \
            if (hasDictionary) {                                                         \
                Dictionary dict;                                                         \
                getValueHelper<Dictionary>(key, dict);                                   \
                const bool canConvert = isConvertible<TYPE>(dict);                       \
                if (canConvert) {                                                        \
                    convertGLM(dict, value);                                             \
                    return true;                                                         \
                }                                                                        \
            }                                                                            \
        }                                                                                \
        return false;                                                                    \
    }

// Storage types
DEF_SPEC_TEMPLATE(double)
DEF_SPEC_TEMPLATE(long long)
DEF_SPEC_TEMPLATE(unsigned long long)

DEF_SPEC_TEMPLATE(bool)
DEF_SPEC_TEMPLATE(char)
DEF_SPEC_TEMPLATE(signed char)
DEF_SPEC_TEMPLATE(unsigned char)
DEF_SPEC_TEMPLATE(wchar_t)
DEF_SPEC_TEMPLATE(short)
DEF_SPEC_TEMPLATE(unsigned short)
DEF_SPEC_TEMPLATE(int)
DEF_SPEC_TEMPLATE(unsigned int)
DEF_SPEC_TEMPLATE(float)
DEF_SPEC_TEMPLATE_GLM(glm::vec2, glm::make_vec2)
DEF_SPEC_TEMPLATE_GLM(glm::dvec2, glm::make_vec2)
DEF_SPEC_TEMPLATE_GLM(glm::ivec2, glm::make_vec2)
DEF_SPEC_TEMPLATE_GLM(glm::uvec2, glm::make_vec2)
DEF_SPEC_TEMPLATE_GLM(glm::bvec2, glm::make_vec2)
DEF_SPEC_TEMPLATE_GLM(glm::vec3, glm::make_vec3)
DEF_SPEC_TEMPLATE_GLM(glm::dvec3, glm::make_vec3)
DEF_SPEC_TEMPLATE_GLM(glm::ivec3, glm::make_vec3)
DEF_SPEC_TEMPLATE_GLM(glm::uvec3, glm::make_vec3)
DEF_SPEC_TEMPLATE_GLM(glm::bvec3, glm::make_vec3)
DEF_SPEC_TEMPLATE_GLM(glm::vec4, glm::make_vec4)
DEF_SPEC_TEMPLATE_GLM(glm::dvec4, glm::make_vec4)
DEF_SPEC_TEMPLATE_GLM(glm::ivec4,  glm::make_vec4)
DEF_SPEC_TEMPLATE_GLM(glm::uvec4, glm::make_vec4)
DEF_SPEC_TEMPLATE_GLM(glm::bvec4, glm::make_vec4)
DEF_SPEC_TEMPLATE_GLM(glm::mat2x2, glm::make_mat2x2)
DEF_SPEC_TEMPLATE_GLM(glm::mat2x3, glm::make_mat2x3)
DEF_SPEC_TEMPLATE_GLM(glm::mat2x4, glm::make_mat2x4)
DEF_SPEC_TEMPLATE_GLM(glm::mat3x2, glm::make_mat3x2)
DEF_SPEC_TEMPLATE_GLM(glm::mat3x3, glm::make_mat3x3)
DEF_SPEC_TEMPLATE_GLM(glm::mat3x4, glm::make_mat3x4)
DEF_SPEC_TEMPLATE_GLM(glm::mat4x2, glm::make_mat4x2)
DEF_SPEC_TEMPLATE_GLM(glm::mat4x3, glm::make_mat4x3)
DEF_SPEC_TEMPLATE_GLM(glm::mat4x4, glm::make_mat4x4)
DEF_SPEC_TEMPLATE_GLM(glm::dmat2x2, glm::make_mat2x2)
DEF_SPEC_TEMPLATE_GLM(glm::dmat2x3, glm::make_mat2x3)
DEF_SPEC_TEMPLATE_GLM(glm::dmat2x4, glm::make_mat2x4)
DEF_SPEC_TEMPLATE_GLM(glm::dmat3x2, glm::make_mat3x2)
DEF_SPEC_TEMPLATE_GLM(glm::dmat3x3, glm::make_mat3x3)
DEF_SPEC_TEMPLATE_GLM(glm::dmat3x4, glm::make_mat3x4)
DEF_SPEC_TEMPLATE_GLM(glm::dmat4x2, glm::make_mat4x2)
DEF_SPEC_TEMPLATE_GLM(glm::dmat4x3, glm::make_mat4x3)
DEF_SPEC_TEMPLATE_GLM(glm::dmat4x4, glm::make_mat4x4)

#undef DEF_SPEC_TEMPLATE
#undef DEF_SPEC_TEMPLATE_GLM

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
