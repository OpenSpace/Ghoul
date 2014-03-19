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

namespace {
	const std::string _loggerCat = "Dictionary";
}

namespace ghoul {

Dictionary::Dictionary() {}

Dictionary::Dictionary(const std::initializer_list<std::pair<string, boost::any>>& l) {
	for (const auto& p : l)
        setValueHelper(std::move(p.first), std::move(p.second));
}
    
const std::vector<string> Dictionary::keys(const string& location) const {
	if (location.empty()) {
		std::vector<string> result;
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
		LERROR("Error converting key '" << first << "' to type 'Dictionary', was '" <<
			keyIt->second.type().name() << "'");
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

bool Dictionary::splitKey(const string& key, string& first, string& rest) const {
	const string::size_type l = key.find('.');

	if (l == string::npos) {
		first = key;
		return false;
	}
	else {
		first = key.substr(0, l);
		rest = key.substr(l + 1);
		return true;
	}
}

#ifdef WIN32
#pragma warning ( disable : 4800 )
#endif

template <typename SOURCE, typename TARGET, size_t SIZE>
std::array<TARGET, SIZE> createArray(const SOURCE* const src) {
    std::array<TARGET, SIZE> v;
    for (size_t i = 0; i < SIZE; ++i)
        v[i] = static_cast<TARGET>(src[i]);
    return v;
}

#define DEF_SPEC_TEMPLATE(TYPE, STORAGETYPE)                                             \
    template <>                                                                          \
    bool Dictionary::setValue<TYPE>(std::string key, TYPE value,                         \
                                    bool createIntermediate) {                           \
        return setValue(std::move(key), STORAGETYPE(value), createIntermediate);         \
    }                                                                                    \
    template <>                                                                          \
    bool Dictionary::getValue<TYPE>(const std::string& key, TYPE& value) const {         \
        STORAGETYPE v;                                                                   \
        const bool success = getValue(key, v);                                           \
        value = static_cast<TYPE>(v);                                                    \
        return success;                                                                  \
    }                                                                                    \
    template <>                                                                          \
    bool Dictionary::hasValue<TYPE>(const std::string& key) const {                      \
        return hasValue<STORAGETYPE>(key);                                               \
    }

#define DEF_SPEC_TEMPLATE_GLM(TYPE, STORAGETYPE, SIZE, CREATE)                           \
    template <>                                                                          \
    bool Dictionary::setValue<TYPE>(std::string key, TYPE value,                         \
                                    bool createIntermediate) {                           \
        auto v                                                                           \
              = createArray<TYPE::value_type, STORAGETYPE, SIZE>(glm::value_ptr(value)); \
        return setValue(std::move(key), std::move(v), createIntermediate);               \
    }                                                                                    \
    template <>                                                                          \
    bool Dictionary::getValue<TYPE>(const std::string& key, TYPE& value) const {         \
        std::array<STORAGETYPE, SIZE> v;                                                 \
        const bool success = getValue(key, v);                                           \
        value = CREATE(v.data());                                                               \
        return success;                                                                  \
    }                                                                                    \
    template <>                                                                          \
    bool Dictionary::hasValue<TYPE>(const std::string& key) const {                      \
        return hasValue<std::array<STORAGETYPE, SIZE>>(key);                             \
    }

DEF_SPEC_TEMPLATE(bool, IntegralType)
DEF_SPEC_TEMPLATE(char, IntegralType)
DEF_SPEC_TEMPLATE(signed char, IntegralType)
DEF_SPEC_TEMPLATE(unsigned char, UnsignedIntegralType)
DEF_SPEC_TEMPLATE(wchar_t, IntegralType)
DEF_SPEC_TEMPLATE(short, IntegralType)
DEF_SPEC_TEMPLATE(unsigned short, UnsignedIntegralType)
DEF_SPEC_TEMPLATE(int, IntegralType)
DEF_SPEC_TEMPLATE(unsigned int, UnsignedIntegralType)
DEF_SPEC_TEMPLATE(float, FloatingType)
DEF_SPEC_TEMPLATE_GLM(glm::vec2, FloatingType, 2, glm::make_vec2)
DEF_SPEC_TEMPLATE_GLM(glm::dvec2, FloatingType, 2, glm::make_vec2)
DEF_SPEC_TEMPLATE_GLM(glm::ivec2, IntegralType, 2, glm::make_vec2)
DEF_SPEC_TEMPLATE_GLM(glm::uvec2, UnsignedIntegralType, 2, glm::make_vec2)
DEF_SPEC_TEMPLATE_GLM(glm::bvec2, IntegralType, 2, glm::make_vec2)
DEF_SPEC_TEMPLATE_GLM(glm::vec3, FloatingType, 3, glm::make_vec3)
DEF_SPEC_TEMPLATE_GLM(glm::dvec3, FloatingType, 3, glm::make_vec3)
DEF_SPEC_TEMPLATE_GLM(glm::ivec3, IntegralType, 3, glm::make_vec3)
DEF_SPEC_TEMPLATE_GLM(glm::uvec3, UnsignedIntegralType, 3, glm::make_vec3)
DEF_SPEC_TEMPLATE_GLM(glm::bvec3, IntegralType, 3, glm::make_vec3)
DEF_SPEC_TEMPLATE_GLM(glm::vec4, FloatingType, 4, glm::make_vec4)
DEF_SPEC_TEMPLATE_GLM(glm::dvec4, FloatingType, 4, glm::make_vec4)
DEF_SPEC_TEMPLATE_GLM(glm::ivec4, IntegralType, 4, glm::make_vec4)
DEF_SPEC_TEMPLATE_GLM(glm::uvec4, UnsignedIntegralType, 4, glm::make_vec4)
DEF_SPEC_TEMPLATE_GLM(glm::bvec4, IntegralType, 4, glm::make_vec4)
DEF_SPEC_TEMPLATE_GLM(glm::mat2x2, FloatingType, 4, glm::make_mat2x2)
DEF_SPEC_TEMPLATE_GLM(glm::mat2x3, FloatingType, 6, glm::make_mat2x3)
DEF_SPEC_TEMPLATE_GLM(glm::mat2x4, FloatingType, 8, glm::make_mat2x4)
DEF_SPEC_TEMPLATE_GLM(glm::mat3x2, FloatingType, 6, glm::make_mat3x2)
DEF_SPEC_TEMPLATE_GLM(glm::mat3x3, FloatingType, 9, glm::make_mat3x3)
DEF_SPEC_TEMPLATE_GLM(glm::mat3x4, FloatingType, 12, glm::make_mat3x4)
DEF_SPEC_TEMPLATE_GLM(glm::mat4x2, FloatingType, 8, glm::make_mat4x2)
DEF_SPEC_TEMPLATE_GLM(glm::mat4x3, FloatingType, 12, glm::make_mat4x3)
DEF_SPEC_TEMPLATE_GLM(glm::mat4x4, FloatingType, 16, glm::make_mat4x4)
DEF_SPEC_TEMPLATE_GLM(glm::dmat2x2, FloatingType, 4, glm::make_mat2x2)
DEF_SPEC_TEMPLATE_GLM(glm::dmat2x3, FloatingType, 6, glm::make_mat2x3)
DEF_SPEC_TEMPLATE_GLM(glm::dmat2x4, FloatingType, 8, glm::make_mat2x4)
DEF_SPEC_TEMPLATE_GLM(glm::dmat3x2, FloatingType, 6, glm::make_mat3x2)
DEF_SPEC_TEMPLATE_GLM(glm::dmat3x3, FloatingType, 9, glm::make_mat3x3)
DEF_SPEC_TEMPLATE_GLM(glm::dmat3x4, FloatingType, 12, glm::make_mat3x4)
DEF_SPEC_TEMPLATE_GLM(glm::dmat4x2, FloatingType, 8, glm::make_mat4x2)
DEF_SPEC_TEMPLATE_GLM(glm::dmat4x3, FloatingType, 12, glm::make_mat4x3)
DEF_SPEC_TEMPLATE_GLM(glm::dmat4x4, FloatingType, 16, glm::make_mat4x4)

#undef DEF_SPEC_TEMPLATE
#undef DEF_SPEC_TEMPLATE_GLM


#ifdef WIN32
#pragma warning ( default : 4800 )
#endif


void Dictionary::setValueHelper(std::string key, boost::any value) {
///////
#define ELSE(TYPE)                                                                       \
    else if (type == typeid(TYPE)) {                                                     \
        setValue(std::move(key), std::move(boost::any_cast<TYPE>(value)));               \
    \
}
    ///////

    // Ugly if-else statement is necessary as 'hash_code' is not constexpr, therefore a
    // switch statement won't work
    const std::type_info& type = value.type();

    if (type == typeid(bool)) {
        setValue(std::move(key), std::move(boost::any_cast<bool>(value)));
    }
    ELSE(char)
    ELSE(signed char)
    ELSE(unsigned char)
    ELSE(wchar_t)
    ELSE(short)
    ELSE(unsigned short)
    ELSE(int)
    ELSE(unsigned int)
    ELSE(long long)
    ELSE(unsigned long long)
    ELSE(float)
    ELSE(double)
    ELSE(glm::vec2)
    ELSE(glm::dvec2)
    ELSE(glm::ivec2)
    ELSE(glm::uvec2)
    ELSE(glm::bvec2)
    ELSE(glm::vec3)
    ELSE(glm::dvec3)
    ELSE(glm::ivec3)
    ELSE(glm::uvec3)
    ELSE(glm::bvec3)
    ELSE(glm::vec4)
    ELSE(glm::dvec4)
    ELSE(glm::ivec4)
    ELSE(glm::uvec4)
    ELSE(glm::bvec4)
    ELSE(glm::mat2x2)
    ELSE(glm::mat2x3)
    ELSE(glm::mat2x4)
    ELSE(glm::mat3x2)
    ELSE(glm::mat3x3)
    ELSE(glm::mat3x4)
    ELSE(glm::mat4x2)
    ELSE(glm::mat4x3)
    ELSE(glm::mat4x4)
    ELSE(glm::dmat2x2)
    ELSE(glm::dmat2x3)
    ELSE(glm::dmat2x4)
    ELSE(glm::dmat3x2)
    ELSE(glm::dmat3x3)
    ELSE(glm::dmat3x4)
    ELSE(glm::dmat4x2)
    ELSE(glm::dmat4x3)
    ELSE(glm::dmat4x4)
    else {
        setValue(key, value, false);
    }
#undef ELSE
}


//
// Helpers
//
//std::string dictionaryhelper::getValueAsString(Dictionary* D, std::string key) {
//    if(D->type(key) == typeid(std::string)) {
//        std::string value;
//        if (D->getValue(key, value)) {
//            return "\"" + value + "\"";
//        }
//    }
//    std::stringstream stringvalue;
//    if(D->type(key) == typeid(double)) {
//        double value;
//        if (D->getValue(key, value)) {
//            stringvalue << value;
//        }
//    } else if(D->type(key) == typeid(int)) {
//        int value;
//        if (D->getValue(key, value)) {
//            stringvalue << value;
//        }
//    } else if(D->type(key) == typeid(bool)) {
//        bool value;
//        if (D->getValue(key, value)) {
//            stringvalue << std::boolalpha << value;
//        }
//    }
//    return stringvalue.str();
//}
//    
//std::string dictionaryhelper::lua_serializer(Dictionary* D, const std::string indentation) {
//    
//    std::string content;
//    const std::string indentationLevel = "    ";
//    std::string internalIndentation = indentation + indentationLevel;
//    
//    content += "{\n";
//    
//    auto keys = D->keys();
//    for (auto name: keys) {
//        if(D->type(name) == typeid(Dictionary*)) {
//            Dictionary *next;
//            D->getValue<Dictionary*>(name, next);
//            content += internalIndentation + name +" = "+lua_serializer(next, internalIndentation) + ",\n";
//        } else {
//            content += internalIndentation + name +" = " + getValueAsString(D, name) + ",\n";
//        }
//    }
//    content += indentation + "}";
//    return content;
//}

} // namespace ghoul
