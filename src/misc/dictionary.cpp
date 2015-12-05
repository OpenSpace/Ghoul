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

#include <algorithm>
#include <array>

using std::string;

using namespace ghoul::internal;

/**
 * The reason for the unified storage space is the easy conversion between similar types.
 * The Dictionary should not be used for high-performance code anyway, so the additional
 * storage requirement is a valid trade-off.
 */

namespace {
const std::string _loggerCat = "Dictionary";

}

namespace ghoul {

#ifdef WIN32
#pragma warning(disable : 4800)
#endif


// Yes, all those functions could be replaced by a macro (and they were), but they are
// easier to read (and debug!) this way ---abock
    
template bool Dictionary::setValue<bool>(std::string key, bool value, bool createIntermediate);
template bool Dictionary::getValue<bool>(const std::string& key, bool& value) const;
template bool Dictionary::hasValue<bool>(const std::string& key) const;

template bool Dictionary::setValue<char>(std::string key, char value, bool createIntermediate);
template bool Dictionary::getValue<char>(const std::string& key, char& value) const;
template bool Dictionary::hasValue<char>(const std::string& key) const;

template bool Dictionary::setValue<signed char>(std::string key, signed char value, bool createIntermediate);
template bool Dictionary::getValue<signed char>(const std::string& key, signed char& value) const;
template bool Dictionary::hasValue<signed char>(const std::string& key) const;

template bool Dictionary::setValue<unsigned char>(std::string key, unsigned char value, bool createIntermediate);
template bool Dictionary::getValue<unsigned char>(const std::string& key, unsigned char& value) const;
template bool Dictionary::hasValue<unsigned char>(const std::string& key) const;

template bool Dictionary::setValue<wchar_t>(std::string key, wchar_t value, bool createIntermediate);
template bool Dictionary::getValue<wchar_t>(const std::string& key, wchar_t& value) const;
template bool Dictionary::hasValue<wchar_t>(const std::string& key) const;

template bool Dictionary::setValue<short>(std::string key, short value, bool createIntermediate);
template bool Dictionary::getValue<short>(const std::string& key, short& value) const;
template bool Dictionary::hasValue<short>(const std::string& key) const;

template bool Dictionary::setValue<unsigned short>(std::string key, unsigned short value, bool createIntermediate);
template bool Dictionary::getValue<unsigned short>(const std::string& key, unsigned short& value) const;
template bool Dictionary::hasValue<unsigned short>(const std::string& key) const;

template bool Dictionary::setValue<int>(std::string key, int value, bool createIntermediate);
template bool Dictionary::getValue<int>(const std::string& key, int& value) const;
template bool Dictionary::hasValue<int>(const std::string& key) const;

template bool Dictionary::setValue<unsigned int>(std::string key, unsigned int value, bool createIntermediate);
template bool Dictionary::getValue<unsigned int>(const std::string& key, unsigned int& value) const;
template bool Dictionary::hasValue<unsigned int>(const std::string& key) const;

template bool Dictionary::setValue<long long>(std::string key, long long value, bool createIntermediate);
template bool Dictionary::getValue<long long>(const std::string& key, long long& value) const;
template bool Dictionary::hasValue<long long>(const std::string& key) const;

template bool Dictionary::setValue<unsigned long long>(std::string key, unsigned long long value, bool createIntermediate);
template bool Dictionary::getValue<unsigned long long>(const std::string& key, unsigned long long& value) const;
template bool Dictionary::hasValue<unsigned long long>(const std::string& key) const;

template bool Dictionary::setValue<float>(std::string key, float value, bool createIntermediate);
template bool Dictionary::getValue<float>(const std::string& key, float& value) const;
template bool Dictionary::hasValue<float>(const std::string& key) const;

template bool Dictionary::setValue<double>(std::string key, double value, bool createIntermediate);
template bool Dictionary::getValue<double>(const std::string& key, double& value) const;
template bool Dictionary::hasValue<double>(const std::string& key) const;

template bool Dictionary::setValue<glm::vec2>(std::string key, glm::vec2 value, bool createIntermediate);
template bool Dictionary::getValue<glm::vec2>(const std::string& key, glm::vec2& value) const;
template bool Dictionary::hasValue<glm::vec2>(const std::string& key) const;

template bool Dictionary::setValue<glm::dvec2>(std::string key, glm::dvec2 value, bool createIntermediate);
template bool Dictionary::getValue<glm::dvec2>(const std::string& key, glm::dvec2& value) const;
template bool Dictionary::hasValue<glm::dvec2>(const std::string& key) const;

template bool Dictionary::setValue<glm::ivec2>(std::string key, glm::ivec2 value, bool createIntermediate);
template bool Dictionary::getValue<glm::ivec2>(const std::string& key, glm::ivec2& value) const;
template bool Dictionary::hasValue<glm::ivec2>(const std::string& key) const;

template bool Dictionary::setValue<glm::uvec2>(std::string key, glm::uvec2 value, bool createIntermediate);
template bool Dictionary::getValue<glm::uvec2>(const std::string& key, glm::uvec2& value) const;
template bool Dictionary::hasValue<glm::uvec2>(const std::string& key) const;

template bool Dictionary::setValue<glm::bvec2>(std::string key, glm::bvec2 value, bool createIntermediate);
template bool Dictionary::getValue<glm::bvec2>(const std::string& key, glm::bvec2& value) const;
template bool Dictionary::hasValue<glm::bvec2>(const std::string& key) const;

template bool Dictionary::setValue<glm::vec3>(std::string key, glm::vec3 value, bool createIntermediate);
template bool Dictionary::getValue<glm::vec3>(const std::string& key, glm::vec3& value) const;
template bool Dictionary::hasValue<glm::vec3>(const std::string& key) const;

template bool Dictionary::setValue<glm::dvec3>(std::string key, glm::dvec3 value, bool createIntermediate);
template bool Dictionary::getValue<glm::dvec3>(const std::string& key, glm::dvec3& value) const;
template bool Dictionary::hasValue<glm::dvec3>(const std::string& key) const;

template bool Dictionary::setValue<glm::ivec3>(std::string key, glm::ivec3 value, bool createIntermediate);
template bool Dictionary::getValue<glm::ivec3>(const std::string& key, glm::ivec3& value) const;
template bool Dictionary::hasValue<glm::ivec3>(const std::string& key) const;

template bool Dictionary::setValue<glm::uvec3>(std::string key, glm::uvec3 value, bool createIntermediate);
template bool Dictionary::getValue<glm::uvec3>(const std::string& key, glm::uvec3& value) const;
template bool Dictionary::hasValue<glm::uvec3>(const std::string& key) const;

template bool Dictionary::setValue<glm::bvec3>(std::string key, glm::bvec3 value, bool createIntermediate);
template bool Dictionary::getValue<glm::bvec3>(const std::string& key, glm::bvec3& value) const;
template bool Dictionary::hasValue<glm::bvec3>(const std::string& key) const;

template bool Dictionary::setValue<glm::vec4>(std::string key, glm::vec4 value, bool createIntermediate);
template bool Dictionary::getValue<glm::vec4>(const std::string& key, glm::vec4& value) const;
template bool Dictionary::hasValue<glm::vec4>(const std::string& key) const;

template bool Dictionary::setValue<glm::dvec4>(std::string key, glm::dvec4 value, bool createIntermediate);
template bool Dictionary::getValue<glm::dvec4>(const std::string& key, glm::dvec4& value) const;
template bool Dictionary::hasValue<glm::dvec4>(const std::string& key) const;

template bool Dictionary::setValue<glm::ivec4>(std::string key, glm::ivec4 value, bool createIntermediate);
template bool Dictionary::getValue<glm::ivec4>(const std::string& key, glm::ivec4& value) const;
template bool Dictionary::hasValue<glm::ivec4>(const std::string& key) const;

template bool Dictionary::setValue<glm::uvec4>(std::string key, glm::uvec4 value, bool createIntermediate);
template bool Dictionary::getValue<glm::uvec4>(const std::string& key, glm::uvec4& value) const;
template bool Dictionary::hasValue<glm::uvec4>(const std::string& key) const;

template bool Dictionary::setValue<glm::bvec4>(std::string key, glm::bvec4 value, bool createIntermediate);
template bool Dictionary::getValue<glm::bvec4>(const std::string& key, glm::bvec4& value) const;
template bool Dictionary::hasValue<glm::bvec4>(const std::string& key) const;

template bool Dictionary::setValue<glm::mat2x2>(std::string key, glm::mat2x2 value, bool createIntermediate);
template bool Dictionary::getValue<glm::mat2x2>(const std::string& key, glm::mat2x2& value) const;
template bool Dictionary::hasValue<glm::mat2x2>(const std::string& key) const;

template bool Dictionary::setValue<glm::mat2x3>(std::string key, glm::mat2x3 value, bool createIntermediate);
template bool Dictionary::getValue<glm::mat2x3>(const std::string& key, glm::mat2x3& value) const;
template bool Dictionary::hasValue<glm::mat2x3>(const std::string& key) const;

template bool Dictionary::setValue<glm::mat2x4>(std::string key, glm::mat2x4 value, bool createIntermediate);
template bool Dictionary::getValue<glm::mat2x4>(const std::string& key, glm::mat2x4& value) const;
template bool Dictionary::hasValue<glm::mat2x4>(const std::string& key) const;

template bool Dictionary::setValue<glm::mat3x2>(std::string key, glm::mat3x2 value, bool createIntermediate);
template bool Dictionary::getValue<glm::mat3x2>(const std::string& key, glm::mat3x2& value) const;
template bool Dictionary::hasValue<glm::mat3x2>(const std::string& key) const;

template bool Dictionary::setValue<glm::mat3x3>(std::string key, glm::mat3x3 value, bool createIntermediate);
template bool Dictionary::getValue<glm::mat3x3>(const std::string& key, glm::mat3x3& value) const;
template bool Dictionary::hasValue<glm::mat3x3>(const std::string& key) const;

template bool Dictionary::setValue<glm::mat3x4>(std::string key, glm::mat3x4 value, bool createIntermediate);
template bool Dictionary::getValue<glm::mat3x4>(const std::string& key, glm::mat3x4& value) const;
template bool Dictionary::hasValue<glm::mat3x4>(const std::string& key) const;

template bool Dictionary::setValue<glm::mat4x2>(std::string key, glm::mat4x2 value, bool createIntermediate);
template bool Dictionary::getValue<glm::mat4x2>(const std::string& key, glm::mat4x2& value) const;
template bool Dictionary::hasValue<glm::mat4x2>(const std::string& key) const;

template bool Dictionary::setValue<glm::mat4x3>(std::string key, glm::mat4x3 value, bool createIntermediate);
template bool Dictionary::getValue<glm::mat4x3>(const std::string& key, glm::mat4x3& value) const;
template bool Dictionary::hasValue<glm::mat4x3>(const std::string& key) const;

template bool Dictionary::setValue<glm::mat4x4>(std::string key, glm::mat4x4 value, bool createIntermediate);
template bool Dictionary::getValue<glm::mat4x4>(const std::string& key, glm::mat4x4& value) const;
template bool Dictionary::hasValue<glm::mat4x4>(const std::string& key) const;

template bool Dictionary::setValue<glm::dmat2x2>(std::string key, glm::dmat2x2 value, bool createIntermediate);
template bool Dictionary::getValue<glm::dmat2x2>(const std::string& key, glm::dmat2x2& value) const;
template bool Dictionary::hasValue<glm::dmat2x2>(const std::string& key) const;

template bool Dictionary::setValue<glm::dmat2x3>(std::string key, glm::dmat2x3 value, bool createIntermediate);
template bool Dictionary::getValue<glm::dmat2x3>(const std::string& key, glm::dmat2x3& value) const;
template bool Dictionary::hasValue<glm::dmat2x3>(const std::string& key) const;

template bool Dictionary::setValue<glm::dmat2x4>(std::string key, glm::dmat2x4 value, bool createIntermediate);
template bool Dictionary::getValue<glm::dmat2x4>(const std::string& key, glm::dmat2x4& value) const;
template bool Dictionary::hasValue<glm::dmat2x4>(const std::string& key) const;

template bool Dictionary::setValue<glm::dmat3x2>(std::string key, glm::dmat3x2 value, bool createIntermediate);
template bool Dictionary::getValue<glm::dmat3x2>(const std::string& key, glm::dmat3x2& value) const;
template bool Dictionary::hasValue<glm::dmat3x2>(const std::string& key) const;

template bool Dictionary::setValue<glm::dmat3x3>(std::string key, glm::dmat3x3 value, bool createIntermediate);
template bool Dictionary::getValue<glm::dmat3x3>(const std::string& key, glm::dmat3x3& value) const;
template bool Dictionary::hasValue<glm::dmat3x3>(const std::string& key) const;

template bool Dictionary::setValue<glm::dmat3x4>(std::string key, glm::dmat3x4 value, bool createIntermediate);
template bool Dictionary::getValue<glm::dmat3x4>(const std::string& key, glm::dmat3x4& value) const;
template bool Dictionary::hasValue<glm::dmat3x4>(const std::string& key) const;

template bool Dictionary::setValue<glm::dmat4x2>(std::string key, glm::dmat4x2 value, bool createIntermediate);
template bool Dictionary::getValue<glm::dmat4x2>(const std::string& key, glm::dmat4x2& value) const;
template bool Dictionary::hasValue<glm::dmat4x2>(const std::string& key) const;

template bool Dictionary::setValue<glm::dmat4x3>(std::string key, glm::dmat4x3 value, bool createIntermediate);
template bool Dictionary::getValue<glm::dmat4x3>(const std::string& key, glm::dmat4x3& value) const;
template bool Dictionary::hasValue<glm::dmat4x3>(const std::string& key) const;

template bool Dictionary::setValue<glm::dmat4x4>(std::string key, glm::dmat4x4 value, bool createIntermediate);
template bool Dictionary::getValue<glm::dmat4x4>(const std::string& key, glm::dmat4x4& value) const;
template bool Dictionary::hasValue<glm::dmat4x4>(const std::string& key) const;

template <>
bool Dictionary::getValue<Dictionary>(const string& key, Dictionary& value) const {
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

Dictionary::Dictionary(std::initializer_list<std::pair<string, boost::any>> l) {
    for (auto p : l)
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
