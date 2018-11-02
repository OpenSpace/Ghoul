/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2018                                                               *
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

#include <ghoul/misc/dictionary.h>

#include <ghoul/misc/assert.h>
#include <algorithm>
#include <array>

using std::string;

using namespace ghoul::internal;

/**
 * The reason for the unified storage space is the easy conversion between similar types.
 * The Dictionary should not be used for high-performance code anyway, so the additional
 * storage requirement is a valid trade-off.
 *
 * The calling hierachy for the #ghoul::Dictionary::setValue,
 * #ghoul::Dictionary::getValue, and #ghoul::Dictionary::hasValue methods are as follows:

 * #ghoul::Dictionary::value <code>-></code> #ghoul::Dictionary::getValue <code>-></code>
 * #ghoul::Dictionary::getValueInternal <code>-></code> #ghoul::Dictionary::getValueHelper
 *
 * #ghoul::Dictionary::hasValue <code>-></code> #ghoul::Dictionary::hasValueInternal
 * <code>-></code> #ghoul::Dictionary::hasValueHelper
 *
 * #ghoul::Dictionary::setValue <code>-></code> #ghoul::Dictionary::setValueInternal
 * <code>-></code> #ghoul::Dictionary::setValueHelper
 */

namespace ghoul {

Dictionary::DictionaryError::DictionaryError(std::string msg)
    : RuntimeError(std::move(msg), "Dictionary")
{}

Dictionary::KeyError::KeyError(std::string msg)
    : DictionaryError(std::move(msg))
{}

Dictionary::ConversionError::ConversionError(std::string msg)
    : DictionaryError(std::move(msg))
{}

#define EXTERN_TEMPLATE_DEFINITION(__TYPE__) \
template void Dictionary::setValue<__TYPE__>(std::string, __TYPE__, CreateIntermediate); \
template bool Dictionary::getValue<__TYPE__>(const std::string&, __TYPE__&) const;       \
template __TYPE__ Dictionary::value<__TYPE__>(const std::string&) const;                 \
template bool Dictionary::hasValue<__TYPE__>(const std::string&) const;                  \
template bool isConvertible<__TYPE__>(const Dictionary&);                                \
template void Dictionary::setValueHelper<__TYPE__>(std::string, __TYPE__,                \
    CreateIntermediate);                                                                 \
template void Dictionary::getValueHelper<__TYPE__>(const std::string&, __TYPE__&) const; \
template bool Dictionary::hasValueHelper<__TYPE__>(const std::string&) const

EXTERN_TEMPLATE_DEFINITION(char);
EXTERN_TEMPLATE_DEFINITION(signed char);
EXTERN_TEMPLATE_DEFINITION(unsigned char);
EXTERN_TEMPLATE_DEFINITION(wchar_t);
EXTERN_TEMPLATE_DEFINITION(short);
EXTERN_TEMPLATE_DEFINITION(unsigned short);
EXTERN_TEMPLATE_DEFINITION(int);
EXTERN_TEMPLATE_DEFINITION(unsigned int);
EXTERN_TEMPLATE_DEFINITION(long long);
EXTERN_TEMPLATE_DEFINITION(unsigned long long);
EXTERN_TEMPLATE_DEFINITION(float);
EXTERN_TEMPLATE_DEFINITION(double);
EXTERN_TEMPLATE_DEFINITION(glm::vec2);
EXTERN_TEMPLATE_DEFINITION(glm::dvec2);
EXTERN_TEMPLATE_DEFINITION(glm::ivec2);
EXTERN_TEMPLATE_DEFINITION(glm::uvec2);
EXTERN_TEMPLATE_DEFINITION(glm::bvec2);
EXTERN_TEMPLATE_DEFINITION(glm::vec3);
EXTERN_TEMPLATE_DEFINITION(glm::dvec3);
EXTERN_TEMPLATE_DEFINITION(glm::ivec3);
EXTERN_TEMPLATE_DEFINITION(glm::uvec3);
EXTERN_TEMPLATE_DEFINITION(glm::bvec3);
EXTERN_TEMPLATE_DEFINITION(glm::vec4);
EXTERN_TEMPLATE_DEFINITION(glm::dvec4);
EXTERN_TEMPLATE_DEFINITION(glm::ivec4);
EXTERN_TEMPLATE_DEFINITION(glm::uvec4);
EXTERN_TEMPLATE_DEFINITION(glm::bvec4);
EXTERN_TEMPLATE_DEFINITION(glm::mat2x2);
EXTERN_TEMPLATE_DEFINITION(glm::mat2x3);
EXTERN_TEMPLATE_DEFINITION(glm::mat2x4);
EXTERN_TEMPLATE_DEFINITION(glm::mat3x2);
EXTERN_TEMPLATE_DEFINITION(glm::mat3x3);
EXTERN_TEMPLATE_DEFINITION(glm::mat3x4);
EXTERN_TEMPLATE_DEFINITION(glm::mat4x2);
EXTERN_TEMPLATE_DEFINITION(glm::mat4x3);
EXTERN_TEMPLATE_DEFINITION(glm::mat4x4);
EXTERN_TEMPLATE_DEFINITION(glm::dmat2x2);
EXTERN_TEMPLATE_DEFINITION(glm::dmat2x3);
EXTERN_TEMPLATE_DEFINITION(glm::dmat2x4);
EXTERN_TEMPLATE_DEFINITION(glm::dmat3x2);
EXTERN_TEMPLATE_DEFINITION(glm::dmat3x3);
EXTERN_TEMPLATE_DEFINITION(glm::dmat3x4);
EXTERN_TEMPLATE_DEFINITION(glm::dmat4x2);
EXTERN_TEMPLATE_DEFINITION(glm::dmat4x3);
EXTERN_TEMPLATE_DEFINITION(glm::dmat4x4);

#undef EXTERN_TEMPLATE_DEFINITION
#undef EXTERN_TEMPLATE_DEFINITION_SCALAR

template <>
bool Dictionary::getValue<Dictionary>(const string& key, Dictionary& val) const {
    ghoul_assert(&val != this, "Value argument must not be 'this' object");
    const bool dict = hasKeyAndValue<Dictionary>(key);
    if (dict) {
        getValueHelper(key, val);
        return true;
    }
    else {
        return false;
    }
}

template <>
bool Dictionary::getValue<std::string>(const std::string& key, std::string& val) const {
    const bool str = hasValue<std::string>(key);
    if (str) {
        getValueHelper(key, val);
        return true;
    }
    const bool c = hasValue<const char*>(key);
    if (c) {
        const char* data;
        getValueHelper<const char*>(key, data);
        val = std::string(data);
        return true;
    }
    return false;
}

template <>
std::string Dictionary::value<std::string>(const std::string& key) const {
    std::string tmp;
    const bool s = getValue(key, tmp);

    if (s) {
        return tmp;
    }
    else {
        throw ConversionError(
            "Error converting key '" + key + "' from type '" +
            find(key)->second.type().name() + "' to type '" +
            typeid(std::string).name() + "'"
        );
    }
}

Dictionary::Dictionary(std::initializer_list<std::pair<std::string, ghoul::any>> l) {
    for (const std::pair<std::string, ghoul::any>& p : l) {
        if (!p.first.empty()) {
            setValueAnyHelper(std::move(p.first), std::move(p.second));
        }
    }
}

std::vector<string> Dictionary::keys(const string& location) const {
    if (location.empty()) {
        std::vector<string> result;
        result.reserve(size());
        for (const std::pair<std::string, ghoul::any>& it : *this) {
            result.push_back(it.first);
        }
        return result;
    }

    std::string first;
    std::string rest;
    splitKey(location, first, rest);

    auto keyIt = find(first);
    if (keyIt == cend()) {
        throw KeyError("Key '" + first + "' was not found");
    }

    const Dictionary* const dict = ghoul::any_cast<Dictionary>(&(keyIt->second));
    if (!dict) {
        throw ConversionError("Error converting key '" + first + "' from type '" +
            keyIt->second.type().name() + "' to type 'Dictionary"
        );
    }
    // proper tail-recursion
    return dict->keys(rest);
}

bool Dictionary::hasKey(const string& key) const {
    ghoul_assert(!key.empty(), "Key must not be empty");

    auto it = find(key);
    if (it != cend()) {
        return true;
    }

    std::string first;
    std::string rest;
    splitKey(key, first, rest);

    auto keyIt = find(first);
    if (keyIt == cend()) {
        return false;
    }

    const Dictionary* const dict = ghoul::any_cast<Dictionary>(&(keyIt->second));
    if (!dict) {
        return false;
    }
    return dict->hasKey(rest);
}

size_t Dictionary::size() const {
    return std::map<std::string, ghoul::any>::size();
}

void Dictionary::clear() {
    std::map<std::string, ghoul::any>::clear();
}

bool Dictionary::empty() const {
    return std::map<std::string, ghoul::any>::empty();
}

bool Dictionary::removeKey(const std::string& key) {
    ghoul_assert(!key.empty(), "Key must not be empty");

    std::map<std::string, ghoul::any>::size_type res = erase(key);
    return (res == 1);
}

bool Dictionary::splitKey(const string& key, string& first, string& rest) const {
    string::size_type l = key.find('.');

    if (l == string::npos) {
        first = key;
        return false;
    } else {
        first = key.substr(0, l);
        rest = key.substr(l + 1);
        return true;
    }
}

void Dictionary::setValueAnyHelper(std::string key, ghoul::any val) {
    // Ugly if-else statement is necessary as 'type' cannot be not constexpr
    const std::type_info& type = val.type();

    if (type == typeid(bool)) {
        setValue(std::move(key), std::move(ghoul::any_cast<bool>(val)));
    }
    else if (type == typeid(char)) {
        setValue(std::move(key), std::move(ghoul::any_cast<char>(val)));
    }
    else if (type == typeid(signed char)) {
        setValue(std::move(key), std::move(ghoul::any_cast<signed char>(val)));
    }
    else if (type == typeid(unsigned char)) {
        setValue(std::move(key), std::move(ghoul::any_cast<unsigned char>(val)));
    }
    else if (type == typeid(wchar_t)) {
        setValue(std::move(key), std::move(ghoul::any_cast<wchar_t>(val)));
    }
    else if (type == typeid(short)) {
        setValue(std::move(key), std::move(ghoul::any_cast<short>(val)));
    }
    else if (type == typeid(unsigned short)) {
        setValue(std::move(key), std::move(ghoul::any_cast<unsigned short>(val)));
    }
    else if (type == typeid(int)) {
        setValue(std::move(key), std::move(ghoul::any_cast<int>(val)));
    }
    else if (type == typeid(unsigned int)) {
        setValue(std::move(key), std::move(ghoul::any_cast<unsigned int>(val)));
    }
    else if (type == typeid(long long)) {
        setValue(std::move(key), std::move(ghoul::any_cast<long long>(val)));
    }
    else if (type == typeid(unsigned long long)) {
        setValue(std::move(key), std::move(ghoul::any_cast<unsigned long long>(val)));
    }
    else if (type == typeid(float)) {
        setValue(std::move(key), std::move(ghoul::any_cast<float>(val)));
    }
    else if (type == typeid(glm::vec2)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::vec2>(val)));
    }
    else if (type == typeid(glm::dvec2)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::dvec2>(val)));
    }
    else if (type == typeid(glm::ivec2)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::ivec2>(val)));
    }
    else if (type == typeid(glm::uvec2)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::uvec2>(val)));
    }
    else if (type == typeid(glm::bvec2)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::bvec2>(val)));
    }
    else if (type == typeid(glm::vec3)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::vec3>(val)));
    }
    else if (type == typeid(glm::dvec3)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::dvec3>(val)));
    }
    else if (type == typeid(glm::ivec3)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::ivec3>(val)));
    }
    else if (type == typeid(glm::uvec3)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::uvec3>(val)));
    }
    else if (type == typeid(glm::bvec3)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::bvec3>(val)));
    }
    else if (type == typeid(glm::vec4)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::vec4>(val)));
    }
    else if (type == typeid(glm::dvec4)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::dvec4>(val)));
    }
    else if (type == typeid(glm::ivec4)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::ivec4>(val)));
    }
    else if (type == typeid(glm::uvec4)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::uvec4>(val)));
    }
    else if (type == typeid(glm::bvec4)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::bvec4>(val)));
    }
    else if (type == typeid(glm::mat2x2)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::mat2x2>(val)));
    }
    else if (type == typeid(glm::mat2x3)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::mat2x3>(val)));
    }
    else if (type == typeid(glm::mat2x4)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::mat2x4>(val)));
    }
    else if (type == typeid(glm::mat3x2)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::mat3x2>(val)));
    }
    else if (type == typeid(glm::mat3x3)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::mat3x3>(val)));
    }
    else if (type == typeid(glm::mat3x4)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::mat3x4>(val)));
    }
    else if (type == typeid(glm::mat4x2)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::mat4x2>(val)));
    }
    else if (type == typeid(glm::mat4x3)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::mat4x3>(val)));
    }
    else if (type == typeid(glm::mat4x4)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::mat4x4>(val)));
    }
    else if (type == typeid(glm::dmat2x2)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::dmat2x2>(val)));
    }
    else if (type == typeid(glm::dmat2x3)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::dmat2x3>(val)));
    }
    else if (type == typeid(glm::dmat2x4)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::dmat2x4>(val)));
    }
    else if (type == typeid(glm::dmat3x2)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::dmat3x2>(val)));
    }
    else if (type == typeid(glm::dmat3x3)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::dmat3x3>(val)));
    }
    else if (type == typeid(glm::dmat3x4)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::dmat3x4>(val)));
    }
    else if (type == typeid(glm::dmat4x2)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::dmat4x2>(val)));
    }
    else if (type == typeid(glm::dmat4x3)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::dmat4x3>(val)));
    }
    else if (type == typeid(glm::dmat4x4)) {
        setValue(std::move(key), std::move(ghoul::any_cast<glm::dmat4x4>(val)));
    }
    else {
        setValue(key, val, CreateIntermediate::No);
    }
}

}  // namespace ghoul
