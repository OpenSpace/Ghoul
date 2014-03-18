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
#include <fstream>
#include <iostream>
#include <array>

using std::string;

namespace {
	const std::string _loggerCat = "Dictionary";
}

namespace ghoul {

template <>
bool Dictionary::setValue<bool>(std::string key, bool value, bool createIntermediate);
template <>
bool Dictionary::setValue<char>(std::string key, char value, bool createIntermediate);
template <>
bool Dictionary::setValue<signed char>(std::string key, signed char value,
                                       bool createIntermediate);
template <>
bool Dictionary::setValue<unsigned char>(std::string key, unsigned char value,
                                         bool createIntermediate);
template <>
bool Dictionary::setValue<wchar_t>(std::string key, wchar_t value,
                                   bool createIntermediate);
template <>
bool Dictionary::setValue<short>(std::string key, short value, bool createIntermediate);
template <>
bool Dictionary::setValue<unsigned short>(std::string key, unsigned short value,
                                          bool createIntermediate);
template <>
bool Dictionary::setValue<int>(std::string key, int value, bool createIntermediate);
template <>
bool Dictionary::setValue<unsigned int>(std::string key, unsigned int value,
                                        bool createIntermediate);
template <>
bool Dictionary::setValue<float>(std::string key, float value, bool createIntermediate);
template <>
bool Dictionary::setValue<glm::vec2>(std::string key, glm::vec2 value,
                                     bool createIntermediate);
template <>
bool Dictionary::setValue<glm::dvec2>(std::string key, glm::dvec2 value,
                                      bool createIntermediate);
template <>
bool Dictionary::setValue<glm::ivec2>(std::string key, glm::ivec2 value,
                                      bool createIntermediate);
template <>
bool Dictionary::setValue<glm::uvec2>(std::string key, glm::uvec2 value,
                                      bool createIntermediate);
template <>
bool Dictionary::setValue<glm::bvec2>(std::string key, glm::bvec2 value,
                                      bool createIntermediate);
template <>
bool Dictionary::setValue<glm::vec3>(std::string key, glm::vec3 value,
                                     bool createIntermediate);
template <>
bool Dictionary::setValue<glm::dvec3>(std::string key, glm::dvec3 value,
                                      bool createIntermediate);
template <>
bool Dictionary::setValue<glm::ivec3>(std::string key, glm::ivec3 value,
                                      bool createIntermediate);
template <>
bool Dictionary::setValue<glm::uvec3>(std::string key, glm::uvec3 value,
                                      bool createIntermediate);
template <>
bool Dictionary::setValue<glm::bvec3>(std::string key, glm::bvec3 value,
                                      bool createIntermediate);
template <>
bool Dictionary::setValue<glm::vec4>(std::string key, glm::vec4 value,
                                     bool createIntermediate);
template <>
bool Dictionary::setValue<glm::dvec4>(std::string key, glm::dvec4 value,
                                      bool createIntermediate);
template <>
bool Dictionary::setValue<glm::ivec4>(std::string key, glm::ivec4 value,
                                      bool createIntermediate);
template <>
bool Dictionary::setValue<glm::uvec4>(std::string key, glm::uvec4 value,
                                      bool createIntermediate);
template <>
bool Dictionary::setValue<glm::bvec4>(std::string key, glm::bvec4 value,
                                      bool createIntermediate);
template <>
bool Dictionary::setValue<glm::mat2x2>(std::string key, glm::mat2x2 value,
                                       bool createIntermediate);
template <>
bool Dictionary::setValue<glm::mat2x3>(std::string key, glm::mat2x3 value,
                                       bool createIntermediate);
template <>
bool Dictionary::setValue<glm::mat2x4>(std::string key, glm::mat2x4 value,
                                       bool createIntermediate);
template <>
bool Dictionary::setValue<glm::mat3x2>(std::string key, glm::mat3x2 value,
                                       bool createIntermediate);
template <>
bool Dictionary::setValue<glm::mat3x3>(std::string key, glm::mat3x3 value,
                                       bool createIntermediate);
template <>
bool Dictionary::setValue<glm::mat3x4>(std::string key, glm::mat3x4 value,
                                       bool createIntermediate);
template <>
bool Dictionary::setValue<glm::mat4x2>(std::string key, glm::mat4x2 value,
                                       bool createIntermediate);
template <>
bool Dictionary::setValue<glm::mat4x3>(std::string key, glm::mat4x3 value,
                                       bool createIntermediate);
template <>
bool Dictionary::setValue<glm::mat4x4>(std::string key, glm::mat4x4 value,
                                       bool createIntermediate);
template <>
bool Dictionary::setValue<glm::dmat2x2>(std::string key, glm::dmat2x2 value,
                                        bool createIntermediate);
template <>
bool Dictionary::setValue<glm::dmat2x3>(std::string key, glm::dmat2x3 value,
                                        bool createIntermediate);
template <>
bool Dictionary::setValue<glm::dmat2x4>(std::string key, glm::dmat2x4 value,
                                        bool createIntermediate);
template <>
bool Dictionary::setValue<glm::dmat3x2>(std::string key, glm::dmat3x2 value,
                                        bool createIntermediate);
template <>
bool Dictionary::setValue<glm::dmat3x3>(std::string key, glm::dmat3x3 value,
                                        bool createIntermediate);
template <>
bool Dictionary::setValue<glm::dmat3x4>(std::string key, glm::dmat3x4 value,
                                        bool createIntermediate);
template <>
bool Dictionary::setValue<glm::dmat4x2>(std::string key, glm::dmat4x2 value,
                                        bool createIntermediate);
template <>
bool Dictionary::setValue<glm::dmat4x3>(std::string key, glm::dmat4x3 value,
                                        bool createIntermediate);
template <>
bool Dictionary::setValue<glm::dmat4x4>(std::string key, glm::dmat4x4 value,
                                        bool createIntermediate);

template <>
bool Dictionary::getValue<bool>(const std::string& key, bool& value) const;

template <>
bool Dictionary::getValue<char>(const std::string& key, char& value) const;

template <>
bool Dictionary::getValue<signed char>(const std::string& key, signed char& value) const;

template <>
bool Dictionary::getValue<unsigned char>(const std::string& key,
                                         unsigned char& value) const;
template <>
bool Dictionary::getValue<wchar_t>(const std::string& key, wchar_t& value) const;

template <>
bool Dictionary::getValue<short>(const std::string& key, short& value) const;

template <>
bool Dictionary::getValue<unsigned short>(const std::string& key,
                                          unsigned short& value) const;
template <>
bool Dictionary::getValue<int>(const std::string& key, int& value) const;

template <>
bool Dictionary::getValue<unsigned int>(const std::string& key,
                                        unsigned int& value) const;
template <>
bool Dictionary::getValue<float>(const std::string& key, float& value) const;

template <>
bool Dictionary::getValue<glm::vec2>(const std::string& key, glm::vec2& value) const;

template <>
bool Dictionary::getValue<glm::dvec2>(const std::string& key, glm::dvec2& value) const;

template <>
bool Dictionary::getValue<glm::ivec2>(const std::string& key, glm::ivec2& value) const;

template <>
bool Dictionary::getValue<glm::uvec2>(const std::string& key, glm::uvec2& value) const;

template <>
bool Dictionary::getValue<glm::bvec2>(const std::string& key, glm::bvec2& value) const;

template <>
bool Dictionary::getValue<glm::vec3>(const std::string& key, glm::vec3& value) const;

template <>
bool Dictionary::getValue<glm::dvec3>(const std::string& key, glm::dvec3& value) const;

template <>
bool Dictionary::getValue<glm::ivec3>(const std::string& key, glm::ivec3& value) const;

template <>
bool Dictionary::getValue<glm::uvec3>(const std::string& key, glm::uvec3& value) const;

template <>
bool Dictionary::getValue<glm::bvec3>(const std::string& key, glm::bvec3& value) const;

template <>
bool Dictionary::getValue<glm::vec4>(const std::string& key, glm::vec4& value) const;

template <>
bool Dictionary::getValue<glm::dvec4>(const std::string& key, glm::dvec4& value) const;

template <>
bool Dictionary::getValue<glm::ivec4>(const std::string& key, glm::ivec4& value) const;

template <>
bool Dictionary::getValue<glm::uvec4>(const std::string& key, glm::uvec4& value) const;

template <>
bool Dictionary::getValue<glm::bvec4>(const std::string& key, glm::bvec4& value) const;

template <>
bool Dictionary::getValue<glm::mat2x2>(const std::string& key, glm::mat2x2& value) const;

template <>
bool Dictionary::getValue<glm::mat2x3>(const std::string& key, glm::mat2x3& value) const;

template <>
bool Dictionary::getValue<glm::mat2x4>(const std::string& key, glm::mat2x4& value) const;

template <>
bool Dictionary::getValue<glm::mat3x2>(const std::string& key, glm::mat3x2& value) const;

template <>
bool Dictionary::getValue<glm::mat3x3>(const std::string& key, glm::mat3x3& value) const;

template <>
bool Dictionary::getValue<glm::mat3x4>(const std::string& key, glm::mat3x4& value) const;

template <>
bool Dictionary::getValue<glm::mat4x2>(const std::string& key, glm::mat4x2& value) const;

template <>
bool Dictionary::getValue<glm::mat4x3>(const std::string& key, glm::mat4x3& value) const;

template <>
bool Dictionary::getValue<glm::mat4x4>(const std::string& key, glm::mat4x4& value) const;

template <>
bool Dictionary::getValue<glm::dmat2x2>(const std::string& key,
                                        glm::dmat2x2& value) const;
template <>
bool Dictionary::getValue<glm::dmat2x3>(const std::string& key,
                                        glm::dmat2x3& value) const;
template <>
bool Dictionary::getValue<glm::dmat2x4>(const std::string& key,
                                        glm::dmat2x4& value) const;
template <>
bool Dictionary::getValue<glm::dmat3x2>(const std::string& key,
                                        glm::dmat3x2& value) const;
template <>
bool Dictionary::getValue<glm::dmat3x3>(const std::string& key,
                                        glm::dmat3x3& value) const;
template <>
bool Dictionary::getValue<glm::dmat3x4>(const std::string& key,
                                        glm::dmat3x4& value) const;
template <>
bool Dictionary::getValue<glm::dmat4x2>(const std::string& key,
                                        glm::dmat4x2& value) const;
template <>
bool Dictionary::getValue<glm::dmat4x3>(const std::string& key,
                                        glm::dmat4x3& value) const;
template <>
bool Dictionary::getValue<glm::dmat4x4>(const std::string& key,
                                        glm::dmat4x4& value) const;

template <>
bool Dictionary::hasValue<bool>(const std::string& key) const;

template <>
bool Dictionary::hasValue<char>(const std::string& key) const;

template <>
bool Dictionary::hasValue<signed char>(const std::string& key) const;

template <>
bool Dictionary::hasValue<unsigned char>(const std::string& key) const;

template <>
bool Dictionary::hasValue<wchar_t>(const std::string& key) const;

template <>
bool Dictionary::hasValue<short>(const std::string& key) const;

template <>
bool Dictionary::hasValue<unsigned short>(const std::string& key) const;

template <>
bool Dictionary::hasValue<int>(const std::string& key) const;

template <>
bool Dictionary::hasValue<unsigned int>(const std::string& key) const;

template <>
bool Dictionary::hasValue<float>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::vec2>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::dvec2>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::ivec2>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::uvec2>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::bvec2>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::vec3>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::dvec3>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::ivec3>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::uvec3>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::bvec3>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::vec4>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::dvec4>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::ivec4>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::uvec4>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::bvec4>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::mat2x2>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::mat2x3>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::mat2x4>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::mat3x2>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::mat3x3>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::mat3x4>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::mat4x2>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::mat4x3>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::mat4x4>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::dmat2x2>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::dmat2x3>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::dmat2x4>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::dmat3x2>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::dmat3x3>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::dmat3x4>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::dmat4x2>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::dmat4x3>(const std::string& key) const;

template <>
bool Dictionary::hasValue<glm::dmat4x4>(const std::string& key) const;

//namespace dictionaryhelper {
//    std::string getValueAsString(Dictionary* D, std::string key);
//    std::string lua_serializer(Dictionary* D, const std::string indentation = "");
//} // namespace helper
    
Dictionary::Dictionary() {}

Dictionary::Dictionary(const std::initializer_list<std::pair<string, boost::any>>& l) {
	for (const auto& p : l)
        setValueHelper(std::move(p.first), std::move(p.second));
		//(*this)[p.first] = p.second;
}
    
//std::string Dictionary::serializeToLuaString(){
//    return dictionaryhelper::lua_serializer(this, "");
//}
//
//bool Dictionary::serializeToFile(const std::string& filename) {
//    
//    // Serialize to lua configuration string
//    std::string luaConfig = serializeToLuaString();
//    
//    std::ofstream out(filename.c_str());
//    
//    if(! out.is_open())
//        return false;
//    
//    out << luaConfig;
//    out.close();
//    return true;
//}

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

//const std::type_info& Dictionary::type(const std::string& key) const {
//	auto it = find(key);
//	if (it != cend())
//		return it->second.type();
//
//	std::string first;
//	std::string rest;
//	splitKey(key, first, rest);
//
//	auto keyIt = find(first);
//	if (keyIt == cend())
//		return typeid(void);
//
//	const Dictionary* const dict = boost::any_cast<Dictionary>(&(keyIt->second));
//	if (!dict)
//		return typeid(void);
//	
//	// Proper tail-recursion
//	return dict->type(rest);
//}

void Dictionary::setValueHelper(std::string key, boost::any value) {
///////
#define ELSE(TYPE)                                                                       \
    else if (type == typeid(TYPE)) {                                                     \
        setValue(std::move(key), std::move(boost::any_cast<TYPE>(value)));                                     \
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

template <>
bool Dictionary::setValue<bool>(std::string key, bool value, bool createIntermediate) {
    return setValue(std::move(key), IntegralType(value), createIntermediate);
}

template <>
bool Dictionary::setValue<char>(std::string key, char value, bool createIntermediate) {
    return setValue(std::move(key), IntegralType(value), createIntermediate);
}

template <>
bool Dictionary::setValue<signed char>(std::string key, signed char value,
                                       bool createIntermediate) {
    return setValue(std::move(key), IntegralType(value), createIntermediate);
}

template <>
bool Dictionary::setValue<unsigned char>(std::string key, unsigned char value,
                                         bool createIntermediate) {
    return setValue(std::move(key), UnsignedIntegralType(value), createIntermediate);
}

template <>
bool Dictionary::setValue<wchar_t>(std::string key, wchar_t value,
                                   bool createIntermediate) {
    return setValue(std::move(key), IntegralType(value), createIntermediate);
}

template <>
bool Dictionary::setValue<short>(std::string key, short value, bool createIntermediate) {
    return setValue(std::move(key), IntegralType(value), createIntermediate);
}

template <>
bool Dictionary::setValue<unsigned short>(std::string key, unsigned short value,
                                          bool createIntermediate) {
    return setValue(std::move(key), UnsignedIntegralType(value), createIntermediate);
}

template <>
bool Dictionary::setValue<int>(std::string key, int value, bool createIntermediate) {
    return setValue(std::move(key), IntegralType(value), createIntermediate);
}

template <>
bool Dictionary::setValue<unsigned int>(std::string key, unsigned int value,
                                        bool createIntermediate) {
    return setValue(std::move(key), UnsignedIntegralType(value), createIntermediate);
}

//template <>
//bool Dictionary::setValue<long long>(std::string key, long long value,
//                                     bool createIntermediate) {
//    return setValue(key, IntegralType(value), createIntermediate);
//}

//template <>
//bool Dictionary::setValue<unsigned long long>(std::string key, unsigned long long value,
//                                              bool createIntermediate) {
//    return setValue(key, UnsignedIntegralType(value), createIntermediate);
//}

template <>
bool Dictionary::setValue<float>(std::string key, float value, bool createIntermediate) {
    return setValue(std::move(key), FloatingType(value), createIntermediate);
}

//template <>
//bool Dictionary::setValue<double>(std::string key, double value,
//                                  bool createIntermediate) {
//    return setValue(key, FloatingType(value), createIntermediate);
//}

template <>
bool Dictionary::setValue<glm::vec2>(std::string key, glm::vec2 value,
                                     bool createIntermediate) {
    std::array<FloatingType, 2> v{ value.x, value.y };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::dvec2>(std::string key, glm::dvec2 value,
    bool createIntermediate) {
    std::array<FloatingType, 2> v{ value.x, value.y };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::ivec2>(std::string key, glm::ivec2 value,
                                      bool createIntermediate) {
    std::array<IntegralType, 2> v{ value.x, value.y };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::uvec2>(std::string key, glm::uvec2 value,
                                      bool createIntermediate) {
    std::array<UnsignedIntegralType, 2> v{ value.x, value.y };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::bvec2>(std::string key, glm::bvec2 value,
                                      bool createIntermediate) {
    std::array<IntegralType, 2> v{ value.x, value.y };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::vec3>(std::string key, glm::vec3 value,
                                     bool createIntermediate) {
    std::array<FloatingType, 3> v{ value.x, value.y, value.z };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::dvec3>(std::string key, glm::dvec3 value,
                                      bool createIntermediate) {
    std::array<FloatingType, 3> v{ value.x, value.y, value.z };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::ivec3>(std::string key, glm::ivec3 value,
                                      bool createIntermediate) {
    std::array<IntegralType, 3> v{ value.x, value.y, value.z };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::uvec3>(std::string key, glm::uvec3 value,
                                      bool createIntermediate) {
    std::array<UnsignedIntegralType, 3> v{ value.x, value.y, value.z };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::bvec3>(std::string key, glm::bvec3 value,
                                      bool createIntermediate) {
    std::array<IntegralType, 3> v{ value.x, value.y, value.z };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::vec4>(std::string key, glm::vec4 value,
                                     bool createIntermediate) {
    std::array<FloatingType, 4> v{ value.x, value.y, value.z, value.w };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::dvec4>(std::string key, glm::dvec4 value,
                                      bool createIntermediate) {
    std::array<FloatingType, 4> v{ value.x, value.y, value.z, value.w };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::ivec4>(std::string key, glm::ivec4 value,
                                      bool createIntermediate) {
    std::array<IntegralType, 4> v{ value.x, value.y, value.z, value.w };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::uvec4>(std::string key, glm::uvec4 value,
                                      bool createIntermediate) {
    std::array<UnsignedIntegralType, 4> v{ value.x, value.y, value.z, value.w };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::bvec4>(std::string key, glm::bvec4 value,
                                      bool createIntermediate) {
    std::array<IntegralType, 4> v{ value.x, value.y, value.z, value.w };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::mat2x2>(std::string key, glm::mat2x2 value,
                                       bool createIntermediate) {
    std::array<FloatingType, 4> v{ value[0][0], value[0][1], value[1][0], value[1][1] };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::mat2x3>(std::string key, glm::mat2x3 value,
                                       bool createIntermediate) {
    std::array<FloatingType, 6> v{ value[0][0], value[0][1], value[0][2], value[1][0], value[1][1], value[1][2] };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::mat2x4>(std::string key, glm::mat2x4 value,
                                       bool createIntermediate) {
    std::array<FloatingType, 8> v{ value[0][0], value[0][1], value[0][2], value[0][3], value[1][0], value[1][1], value[1][2], value[1][3] };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::mat3x2>(std::string key, glm::mat3x2 value,
                                       bool createIntermediate) {
    std::array<FloatingType, 6> v{ value[0][0], value[0][1], value[1][0], value[1][1], value[2][0], value[2][1] };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::mat3x3>(std::string key, glm::mat3x3 value,
                                       bool createIntermediate) {
    std::array<FloatingType, 9> v{ value[0][0], value[0][1], value[0][2], value[1][0], value[1][1], value[1][2], value[2][0], value[2][1], value[2][2] };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::mat3x4>(std::string key, glm::mat3x4 value,
                                       bool createIntermediate) {
    std::array<FloatingType, 12> v{ value[0][0], value[0][1], value[0][2], value[0][3], value[1][0], value[1][1], value[1][2], value[1][3], value[2][0], value[2][1], value[2][2], value[2][3] };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::mat4x2>(std::string key, glm::mat4x2 value,
                                       bool createIntermediate) {
    std::array<FloatingType, 8> v{ value[0][0], value[0][1], value[1][0], value[1][1], value[2][0], value[2][1], value[3][0], value[3][1] };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::mat4x3>(std::string key, glm::mat4x3 value,
                                       bool createIntermediate) {
    std::array<FloatingType, 12> v{ value[0][0], value[0][1], value[0][2], value[1][0], value[1][1], value[1][2], value[2][0], value[2][1], value[2][2], value[3][0], value[3][1], value[3][2] };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::mat4x4>(std::string key, glm::mat4x4 value,
                                       bool createIntermediate) {
    std::array<FloatingType, 16> v{ value[0][0], value[0][1], value[0][2], value[0][3], value[1][0], value[1][1], value[1][2], value[1][3], value[2][0], value[2][1], value[2][2], value[2][3], value[3][0], value[3][1], value[3][2], value[3][3] };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::dmat2x2>(std::string key, glm::dmat2x2 value,
    bool createIntermediate) {
    std::array<FloatingType, 4> v{ value[0][0], value[0][1], value[1][0], value[1][1] };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::dmat2x3>(std::string key, glm::dmat2x3 value,
    bool createIntermediate) {
    std::array<FloatingType, 6> v{ value[0][0], value[0][1], value[0][2], value[1][0], value[1][1], value[1][2] };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::dmat2x4>(std::string key, glm::dmat2x4 value,
    bool createIntermediate) {
    std::array<FloatingType, 8> v{ value[0][0], value[0][1], value[0][2], value[0][3], value[1][0], value[1][1], value[1][2], value[1][3] };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::dmat3x2>(std::string key, glm::dmat3x2 value,
    bool createIntermediate) {
    std::array<FloatingType, 6> v{ value[0][0], value[0][1], value[1][0], value[1][1], value[2][0], value[2][1] };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::dmat3x3>(std::string key, glm::dmat3x3 value,
    bool createIntermediate) {
    std::array<FloatingType, 9> v{ value[0][0], value[0][1], value[0][2], value[1][0], value[1][1], value[1][2], value[2][0], value[2][1], value[2][2] };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::dmat3x4>(std::string key, glm::dmat3x4 value,
    bool createIntermediate) {
    std::array<FloatingType, 12> v{ value[0][0], value[0][1], value[0][2], value[0][3], value[1][0], value[1][1], value[1][2], value[1][3], value[2][0], value[2][1], value[2][2], value[2][3] };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::dmat4x2>(std::string key, glm::dmat4x2 value,
    bool createIntermediate) {
    std::array<FloatingType, 8> v{ value[0][0], value[0][1], value[1][0], value[1][1], value[2][0], value[2][1], value[3][0], value[3][1] };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::dmat4x3>(std::string key, glm::dmat4x3 value,
    bool createIntermediate) {
    std::array<FloatingType, 12> v{ value[0][0], value[0][1], value[0][2], value[1][0], value[1][1], value[1][2], value[2][0], value[2][1], value[2][2], value[3][0], value[3][1], value[3][2] };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::setValue<glm::dmat4x4>(std::string key, glm::dmat4x4 value,
    bool createIntermediate) {
    std::array<FloatingType, 16> v{ value[0][0], value[0][1], value[0][2], value[0][3], value[1][0], value[1][1], value[1][2], value[1][3], value[2][0], value[2][1], value[2][2], value[2][3], value[3][0], value[3][1], value[3][2], value[3][3] };
    return setValue(std::move(key), std::move(v), createIntermediate);
}

template <>
bool Dictionary::getValue<bool>(const std::string& key, bool& value) const {
    IntegralType v;
    const bool success = getValue(key, v);
    value = (v == 1);
    return success;
}

template <>
bool Dictionary::getValue<char>(const std::string& key, char& value) const {
    IntegralType v;
    const bool success = getValue(key, v);
    value = static_cast<char>(v);
    return success;
}

template <>
bool Dictionary::getValue<signed char>(const std::string& key, signed char& value) const {
    IntegralType v;
    const bool success = getValue(key, v);
    value = static_cast<signed char>(v);
    return success;
}

template <>
bool Dictionary::getValue<unsigned char>(const std::string& key,
                                         unsigned char& value) const {
    UnsignedIntegralType v;
    const bool success = getValue(key, v);
    value = static_cast<unsigned char>(v);
    return success;
}

template <>
bool Dictionary::getValue<wchar_t>(const std::string& key, wchar_t& value) const {
    IntegralType v;
    const bool success = getValue(key, v);
    value = static_cast<wchar_t>(v);
    return success;
}

template <>
bool Dictionary::getValue<short>(const std::string& key, short& value) const {
    IntegralType v;
    const bool success = getValue(key, v);
    value = static_cast<short>(v);
    return success;
}

template <>
bool Dictionary::getValue<unsigned short>(const std::string& key,
                                          unsigned short& value) const {
    UnsignedIntegralType v;
    const bool success = getValue(key, v);
    value = static_cast<unsigned short>(v);
    return success;
}

template <>
bool Dictionary::getValue<int>(const std::string& key, int& value) const {
    IntegralType v;
    const bool success = getValue(key, v);
    value = static_cast<int>(v);
    return success;
}

template <>
bool Dictionary::getValue<unsigned int>(const std::string& key,
                                        unsigned int& value) const {
    UnsignedIntegralType v;
    const bool success = getValue(key, v);
    value = static_cast<unsigned int>(v);
    return success;
}

template <>
bool Dictionary::getValue<float>(const std::string& key, float& value) const {
    FloatingType v;
    const bool success = getValue(key, v);
    value = static_cast<float>(v);
    return success;
}

template <>
bool Dictionary::getValue<glm::vec2>(const std::string& key, glm::vec2& value) const {
    std::array<FloatingType, 2> v;
    const bool success = getValue(key, v);
    value.x = static_cast<glm::vec2::value_type>(v[0]);
    value.y = static_cast<glm::vec2::value_type>(v[1]);
    return success;
}

template <>
bool Dictionary::getValue<glm::dvec2>(const std::string& key, glm::dvec2& value) const {
    std::array<FloatingType, 2> v;
    const bool success = getValue(key, v);
    value.x = static_cast<glm::dvec2::value_type>(v[0]);
    value.y = static_cast<glm::dvec2::value_type>(v[1]);
    return success;
}

template <>
bool Dictionary::getValue<glm::ivec2>(const std::string& key, glm::ivec2& value) const {
    std::array<IntegralType, 2> v;
    const bool success = getValue(key, v);
    value.x = static_cast<glm::ivec2::value_type>(v[0]);
    value.y = static_cast<glm::ivec2::value_type>(v[1]);
    return success;
}

template <>
bool Dictionary::getValue<glm::uvec2>(const std::string& key, glm::uvec2& value) const {
    std::array<UnsignedIntegralType, 2> v;
    const bool success = getValue(key, v);
    value.x = static_cast<glm::uvec2::value_type>(v[0]);
    value.y = static_cast<glm::uvec2::value_type>(v[1]);
    return success;
}

template <>
bool Dictionary::getValue<glm::bvec2>(const std::string& key, glm::bvec2& value) const {
    std::array<IntegralType, 2> v;
    const bool success = getValue(key, v);
    value.x = (v[0] == 1);
    value.y = (v[1] == 1);
    return success;
}

template <>
bool Dictionary::getValue<glm::vec3>(const std::string& key, glm::vec3& value) const {
    std::array<FloatingType, 3> v;
    const bool success = getValue(key, v);
    value.x = static_cast<glm::vec3::value_type>(v[0]);
    value.y = static_cast<glm::vec3::value_type>(v[1]);
    value.z = static_cast<glm::vec3::value_type>(v[2]);
    return success;
}

template <>
bool Dictionary::getValue<glm::dvec3>(const std::string& key, glm::dvec3& value) const {
    std::array<FloatingType, 3> v;
    const bool success = getValue(key, v);
    value.x = static_cast<glm::dvec3::value_type>(v[0]);
    value.y = static_cast<glm::dvec3::value_type>(v[1]);
    value.z = static_cast<glm::dvec3::value_type>(v[2]);
    return success;
}

template <>
bool Dictionary::getValue<glm::ivec3>(const std::string& key, glm::ivec3& value) const {
    std::array<IntegralType, 3> v;
    const bool success = getValue(key, v);
    value.x = static_cast<glm::ivec3::value_type>(v[0]);
    value.y = static_cast<glm::ivec3::value_type>(v[1]);
    value.z = static_cast<glm::ivec3::value_type>(v[2]);
    return success;
}

template <>
bool Dictionary::getValue<glm::uvec3>(const std::string& key, glm::uvec3& value) const {
    std::array<UnsignedIntegralType, 3> v;
    const bool success = getValue(key, v);
    value.x = static_cast<glm::uvec3::value_type>(v[0]);
    value.y = static_cast<glm::uvec3::value_type>(v[1]);
    value.z = static_cast<glm::uvec3::value_type>(v[2]);
    return success;
}

template <>
bool Dictionary::getValue<glm::bvec3>(const std::string& key, glm::bvec3& value) const {
    std::array<IntegralType, 3> v;
    const bool success = getValue(key, v);
    value.x = (v[0] == 1);
    value.y = (v[1] == 1);
    value.z = (v[2] == 1);
    return success;
}

template <>
bool Dictionary::getValue<glm::vec4>(const std::string& key, glm::vec4& value) const {
    std::array<FloatingType, 4> v;
    const bool success = getValue(key, v);
    value.x = static_cast<glm::vec4::value_type>(v[0]);
    value.y = static_cast<glm::vec4::value_type>(v[1]);
    value.z = static_cast<glm::vec4::value_type>(v[2]);
    value.w = static_cast<glm::vec4::value_type>(v[3]);
    return success;
}

template <>
bool Dictionary::getValue<glm::dvec4>(const std::string& key, glm::dvec4& value) const {
    std::array<FloatingType, 4> v;
    const bool success = getValue(key, v);
    value.x = static_cast<glm::dvec4::value_type>(v[0]);
    value.y = static_cast<glm::dvec4::value_type>(v[1]);
    value.z = static_cast<glm::dvec4::value_type>(v[2]);
    value.w = static_cast<glm::dvec4::value_type>(v[3]);
    return success;
}

template <>
bool Dictionary::getValue<glm::ivec4>(const std::string& key, glm::ivec4& value) const {
    std::array<IntegralType, 4> v;
    const bool success = getValue(key, v);
    value.x = static_cast<glm::ivec4::value_type>(v[0]);
    value.y = static_cast<glm::ivec4::value_type>(v[1]);
    value.z = static_cast<glm::ivec4::value_type>(v[2]);
    value.w = static_cast<glm::ivec4::value_type>(v[3]);
    return success;
}

template <>
bool Dictionary::getValue<glm::uvec4>(const std::string& key, glm::uvec4& value) const {
    std::array<UnsignedIntegralType, 4> v;
    const bool success = getValue(key, v);
    value.x = static_cast<glm::uvec4::value_type>(v[0]);
    value.y = static_cast<glm::uvec4::value_type>(v[1]);
    value.z = static_cast<glm::uvec4::value_type>(v[2]);
    value.w = static_cast<glm::uvec4::value_type>(v[3]);
    return success;
}

template <>
bool Dictionary::getValue<glm::bvec4>(const std::string& key, glm::bvec4& value) const {
    std::array<IntegralType, 4> v;
    const bool success = getValue(key, v);
    value.x = (v[0] == 1);
    value.y = (v[1] == 1);
    value.z = (v[2] == 1);
    value.w = (v[3] == 1);
    return success;
}

template <>
bool Dictionary::getValue<glm::mat2x2>(const std::string& key, glm::mat2x2& value) const {
    std::array<FloatingType, 4> v;
    const bool success = getValue(key, v);
    value[0][0] = static_cast<glm::mat2x2::value_type>(v[0]);
    value[0][1] = static_cast<glm::mat2x2::value_type>(v[1]);
    value[1][0] = static_cast<glm::mat2x2::value_type>(v[2]);
    value[1][1] = static_cast<glm::mat2x2::value_type>(v[3]);
    return success;
}

template <>
bool Dictionary::getValue<glm::mat2x3>(const std::string& key, glm::mat2x3& value) const {
    std::array<FloatingType, 6> v;
    const bool success = getValue(key, v);
    value[0][0] = static_cast<glm::mat2x3::value_type>(v[0]);
    value[0][1] = static_cast<glm::mat2x3::value_type>(v[1]);
    value[0][2] = static_cast<glm::mat2x3::value_type>(v[2]);
    value[1][0] = static_cast<glm::mat2x3::value_type>(v[3]);
    value[1][1] = static_cast<glm::mat2x3::value_type>(v[4]);
    value[1][2] = static_cast<glm::mat2x3::value_type>(v[5]);
    return success;
}

template <>
bool Dictionary::getValue<glm::mat2x4>(const std::string& key, glm::mat2x4& value) const {
    std::array<FloatingType, 8> v;
    const bool success = getValue(key, v);
    value[0][0] = static_cast<glm::mat2x4::value_type>(v[0]);
    value[0][1] = static_cast<glm::mat2x4::value_type>(v[1]);
    value[0][2] = static_cast<glm::mat2x4::value_type>(v[2]);
    value[0][3] = static_cast<glm::mat2x4::value_type>(v[3]);
    value[1][0] = static_cast<glm::mat2x4::value_type>(v[4]);
    value[1][1] = static_cast<glm::mat2x4::value_type>(v[5]);
    value[1][2] = static_cast<glm::mat2x4::value_type>(v[6]);
    value[1][3] = static_cast<glm::mat2x4::value_type>(v[7]);
    return success;
}

template <>
bool Dictionary::getValue<glm::mat3x2>(const std::string& key, glm::mat3x2& value) const {
    std::array<FloatingType, 6> v;
    const bool success = getValue(key, v);
    value[0][0] = static_cast<glm::mat3x2::value_type>(v[0]);
    value[0][1] = static_cast<glm::mat3x2::value_type>(v[1]);
    value[1][0] = static_cast<glm::mat3x2::value_type>(v[2]);
    value[1][1] = static_cast<glm::mat3x2::value_type>(v[3]);
    value[2][0] = static_cast<glm::mat3x2::value_type>(v[4]);
    value[2][1] = static_cast<glm::mat3x2::value_type>(v[5]);
    return success;
}

template <>
bool Dictionary::getValue<glm::mat3x3>(const std::string& key, glm::mat3x3& value) const {
    std::array<FloatingType, 9> v;
    const bool success = getValue(key, v);
    value[0][0] = static_cast<glm::mat3x3::value_type>(v[0]);
    value[0][1] = static_cast<glm::mat3x3::value_type>(v[1]);
    value[0][2] = static_cast<glm::mat3x3::value_type>(v[2]);
    value[1][0] = static_cast<glm::mat3x3::value_type>(v[3]);
    value[1][1] = static_cast<glm::mat3x3::value_type>(v[4]);
    value[1][2] = static_cast<glm::mat3x3::value_type>(v[5]);
    value[2][0] = static_cast<glm::mat3x3::value_type>(v[6]);
    value[2][1] = static_cast<glm::mat3x3::value_type>(v[7]);
    value[2][2] = static_cast<glm::mat3x3::value_type>(v[8]);
    return success;
}

template <>
bool Dictionary::getValue<glm::mat3x4>(const std::string& key, glm::mat3x4& value) const {
    std::array<FloatingType, 12> v;
    const bool success = getValue(key, v);
    value[0][0] = static_cast<glm::mat3x4::value_type>(v[0]);
    value[0][1] = static_cast<glm::mat3x4::value_type>(v[1]);
    value[0][2] = static_cast<glm::mat3x4::value_type>(v[2]);
    value[0][3] = static_cast<glm::mat3x4::value_type>(v[3]);
    value[1][0] = static_cast<glm::mat3x4::value_type>(v[4]);
    value[1][1] = static_cast<glm::mat3x4::value_type>(v[5]);
    value[1][2] = static_cast<glm::mat3x4::value_type>(v[6]);
    value[1][3] = static_cast<glm::mat3x4::value_type>(v[7]);
    value[2][0] = static_cast<glm::mat3x4::value_type>(v[8]);
    value[2][1] = static_cast<glm::mat3x4::value_type>(v[9]);
    value[2][2] = static_cast<glm::mat3x4::value_type>(v[10]);
    value[2][3] = static_cast<glm::mat3x4::value_type>(v[11]);
    return success;
}

template <>
bool Dictionary::getValue<glm::mat4x2>(const std::string& key, glm::mat4x2& value) const {
    std::array<FloatingType, 8> v;
    const bool success = getValue(key, v);
    value[0][0] = static_cast<glm::mat4x2::value_type>(v[0]);
    value[0][1] = static_cast<glm::mat4x2::value_type>(v[1]);
    value[1][0] = static_cast<glm::mat4x2::value_type>(v[2]);
    value[1][1] = static_cast<glm::mat4x2::value_type>(v[3]);
    value[2][0] = static_cast<glm::mat4x2::value_type>(v[4]);
    value[2][1] = static_cast<glm::mat4x2::value_type>(v[5]);
    value[3][0] = static_cast<glm::mat4x2::value_type>(v[6]);
    value[3][1] = static_cast<glm::mat4x2::value_type>(v[7]);
    return success;
}

template <>
bool Dictionary::getValue<glm::mat4x3>(const std::string& key, glm::mat4x3& value) const {
    std::array<FloatingType, 12> v;
    const bool success = getValue(key, v);
    value[0][0] = static_cast<glm::mat4x3::value_type>(v[0]);
    value[0][1] = static_cast<glm::mat4x3::value_type>(v[1]);
    value[0][2] = static_cast<glm::mat4x3::value_type>(v[2]);
    value[1][0] = static_cast<glm::mat4x3::value_type>(v[3]);
    value[1][1] = static_cast<glm::mat4x3::value_type>(v[4]);
    value[1][2] = static_cast<glm::mat4x3::value_type>(v[5]);
    value[2][0] = static_cast<glm::mat4x3::value_type>(v[6]);
    value[2][1] = static_cast<glm::mat4x3::value_type>(v[7]);
    value[2][2] = static_cast<glm::mat4x3::value_type>(v[8]);
    value[3][0] = static_cast<glm::mat4x3::value_type>(v[9]);
    value[3][1] = static_cast<glm::mat4x3::value_type>(v[10]);
    value[3][2] = static_cast<glm::mat4x3::value_type>(v[11]);
    return success;
}

template <>
bool Dictionary::getValue<glm::mat4x4>(const std::string& key, glm::mat4x4& value) const {
    std::array<FloatingType, 16> v;
    const bool success = getValue(key, v);
    value[0][0] = static_cast<glm::mat4x4::value_type>(v[0]);
    value[0][1] = static_cast<glm::mat4x4::value_type>(v[1]);
    value[0][2] = static_cast<glm::mat4x4::value_type>(v[2]);
    value[0][3] = static_cast<glm::mat4x4::value_type>(v[3]);
    value[1][0] = static_cast<glm::mat4x4::value_type>(v[4]);
    value[1][1] = static_cast<glm::mat4x4::value_type>(v[5]);
    value[1][2] = static_cast<glm::mat4x4::value_type>(v[6]);
    value[1][3] = static_cast<glm::mat4x4::value_type>(v[7]);
    value[2][0] = static_cast<glm::mat4x4::value_type>(v[8]);
    value[2][1] = static_cast<glm::mat4x4::value_type>(v[9]);
    value[2][2] = static_cast<glm::mat4x4::value_type>(v[10]);
    value[2][3] = static_cast<glm::mat4x4::value_type>(v[11]);
    value[3][0] = static_cast<glm::mat4x4::value_type>(v[12]);
    value[3][1] = static_cast<glm::mat4x4::value_type>(v[13]);
    value[3][2] = static_cast<glm::mat4x4::value_type>(v[14]);
    value[3][3] = static_cast<glm::mat4x4::value_type>(v[15]);
    return success;
}

template <>
bool Dictionary::getValue<glm::dmat2x2>(const std::string& key, glm::dmat2x2& value) const {
    std::array<FloatingType, 4> v;
    const bool success = getValue(key, v);
    value[0][0] = static_cast<glm::dmat2x2::value_type>(v[0]);
    value[0][1] = static_cast<glm::dmat2x2::value_type>(v[1]);
    value[1][0] = static_cast<glm::dmat2x2::value_type>(v[2]);
    value[1][1] = static_cast<glm::dmat2x2::value_type>(v[3]);
    return success;
}

template <>
bool Dictionary::getValue<glm::dmat2x3>(const std::string& key, glm::dmat2x3& value) const {
    std::array<FloatingType, 6> v;
    const bool success = getValue(key, v);
    value[0][0] = static_cast<glm::dmat2x3::value_type>(v[0]);
    value[0][1] = static_cast<glm::dmat2x3::value_type>(v[1]);
    value[0][2] = static_cast<glm::dmat2x3::value_type>(v[2]);
    value[1][0] = static_cast<glm::dmat2x3::value_type>(v[3]);
    value[1][1] = static_cast<glm::dmat2x3::value_type>(v[4]);
    value[1][2] = static_cast<glm::dmat2x3::value_type>(v[5]);
    return success;
}

template <>
bool Dictionary::getValue<glm::dmat2x4>(const std::string& key, glm::dmat2x4& value) const {
    std::array<FloatingType, 8> v;
    const bool success = getValue(key, v);
    value[0][0] = static_cast<glm::dmat2x4::value_type>(v[0]);
    value[0][1] = static_cast<glm::dmat2x4::value_type>(v[1]);
    value[0][2] = static_cast<glm::dmat2x4::value_type>(v[2]);
    value[0][3] = static_cast<glm::dmat2x4::value_type>(v[3]);
    value[1][0] = static_cast<glm::dmat2x4::value_type>(v[4]);
    value[1][1] = static_cast<glm::dmat2x4::value_type>(v[5]);
    value[1][2] = static_cast<glm::dmat2x4::value_type>(v[6]);
    value[1][3] = static_cast<glm::dmat2x4::value_type>(v[7]);
    return success;
}

template <>
bool Dictionary::getValue<glm::dmat3x2>(const std::string& key, glm::dmat3x2& value) const {
    std::array<FloatingType, 6> v;
    const bool success = getValue(key, v);
    value[0][0] = static_cast<glm::dmat3x2::value_type>(v[0]);
    value[0][1] = static_cast<glm::dmat3x2::value_type>(v[1]);
    value[1][0] = static_cast<glm::dmat3x2::value_type>(v[2]);
    value[1][1] = static_cast<glm::dmat3x2::value_type>(v[3]);
    value[2][0] = static_cast<glm::dmat3x2::value_type>(v[4]);
    value[2][1] = static_cast<glm::dmat3x2::value_type>(v[5]);
    return success;
}

template <>
bool Dictionary::getValue<glm::dmat3x3>(const std::string& key, glm::dmat3x3& value) const {
    std::array<FloatingType, 9> v;
    const bool success = getValue(key, v);
    value[0][0] = static_cast<glm::dmat3x3::value_type>(v[0]);
    value[0][1] = static_cast<glm::dmat3x3::value_type>(v[1]);
    value[0][2] = static_cast<glm::dmat3x3::value_type>(v[2]);
    value[1][0] = static_cast<glm::dmat3x3::value_type>(v[3]);
    value[1][1] = static_cast<glm::dmat3x3::value_type>(v[4]);
    value[1][2] = static_cast<glm::dmat3x3::value_type>(v[5]);
    value[2][0] = static_cast<glm::dmat3x3::value_type>(v[6]);
    value[2][1] = static_cast<glm::dmat3x3::value_type>(v[7]);
    value[2][2] = static_cast<glm::dmat3x3::value_type>(v[8]);
    return success;
}

template <>
bool Dictionary::getValue<glm::dmat3x4>(const std::string& key, glm::dmat3x4& value) const {
    std::array<FloatingType, 12> v;
    const bool success = getValue(key, v);
    value[0][0] = static_cast<glm::dmat3x4::value_type>(v[0]);
    value[0][1] = static_cast<glm::dmat3x4::value_type>(v[1]);
    value[0][2] = static_cast<glm::dmat3x4::value_type>(v[2]);
    value[0][3] = static_cast<glm::dmat3x4::value_type>(v[3]);
    value[1][0] = static_cast<glm::dmat3x4::value_type>(v[4]);
    value[1][1] = static_cast<glm::dmat3x4::value_type>(v[5]);
    value[1][2] = static_cast<glm::dmat3x4::value_type>(v[6]);
    value[1][3] = static_cast<glm::dmat3x4::value_type>(v[7]);
    value[2][0] = static_cast<glm::dmat3x4::value_type>(v[8]);
    value[2][1] = static_cast<glm::dmat3x4::value_type>(v[9]);
    value[2][2] = static_cast<glm::dmat3x4::value_type>(v[10]);
    value[2][3] = static_cast<glm::dmat3x4::value_type>(v[11]);
    return success;
}

template <>
bool Dictionary::getValue<glm::dmat4x2>(const std::string& key, glm::dmat4x2& value) const {
    std::array<FloatingType, 8> v;
    const bool success = getValue(key, v);
    value[0][0] = static_cast<glm::dmat4x2::value_type>(v[0]);
    value[0][1] = static_cast<glm::dmat4x2::value_type>(v[1]);
    value[1][0] = static_cast<glm::dmat4x2::value_type>(v[2]);
    value[1][1] = static_cast<glm::dmat4x2::value_type>(v[3]);
    value[2][0] = static_cast<glm::dmat4x2::value_type>(v[4]);
    value[2][1] = static_cast<glm::dmat4x2::value_type>(v[5]);
    value[3][0] = static_cast<glm::dmat4x2::value_type>(v[6]);
    value[3][1] = static_cast<glm::dmat4x2::value_type>(v[7]);
    return success;
}

template <>
bool Dictionary::getValue<glm::dmat4x3>(const std::string& key, glm::dmat4x3& value) const {
    std::array<FloatingType, 12> v;
    const bool success = getValue(key, v);
    value[0][0] = static_cast<glm::dmat4x3::value_type>(v[0]);
    value[0][1] = static_cast<glm::dmat4x3::value_type>(v[1]);
    value[0][2] = static_cast<glm::dmat4x3::value_type>(v[2]);
    value[1][0] = static_cast<glm::dmat4x3::value_type>(v[3]);
    value[1][1] = static_cast<glm::dmat4x3::value_type>(v[4]);
    value[1][2] = static_cast<glm::dmat4x3::value_type>(v[5]);
    value[2][0] = static_cast<glm::dmat4x3::value_type>(v[6]);
    value[2][1] = static_cast<glm::dmat4x3::value_type>(v[7]);
    value[2][2] = static_cast<glm::dmat4x3::value_type>(v[8]);
    value[3][0] = static_cast<glm::dmat4x3::value_type>(v[9]);
    value[3][1] = static_cast<glm::dmat4x3::value_type>(v[10]);
    value[3][2] = static_cast<glm::dmat4x3::value_type>(v[11]);
    return success;
}

template <>
bool Dictionary::getValue<glm::dmat4x4>(const std::string& key, glm::dmat4x4& value) const {
    std::array<FloatingType, 16> v;
    const bool success = getValue(key, v);
    value[0][0] = static_cast<glm::dmat4x4::value_type>(v[0]);
    value[0][1] = static_cast<glm::dmat4x4::value_type>(v[1]);
    value[0][2] = static_cast<glm::dmat4x4::value_type>(v[2]);
    value[0][3] = static_cast<glm::dmat4x4::value_type>(v[3]);
    value[1][0] = static_cast<glm::dmat4x4::value_type>(v[4]);
    value[1][1] = static_cast<glm::dmat4x4::value_type>(v[5]);
    value[1][2] = static_cast<glm::dmat4x4::value_type>(v[6]);
    value[1][3] = static_cast<glm::dmat4x4::value_type>(v[7]);
    value[2][0] = static_cast<glm::dmat4x4::value_type>(v[8]);
    value[2][1] = static_cast<glm::dmat4x4::value_type>(v[9]);
    value[2][2] = static_cast<glm::dmat4x4::value_type>(v[10]);
    value[2][3] = static_cast<glm::dmat4x4::value_type>(v[11]);
    value[3][0] = static_cast<glm::dmat4x4::value_type>(v[12]);
    value[3][1] = static_cast<glm::dmat4x4::value_type>(v[13]);
    value[3][2] = static_cast<glm::dmat4x4::value_type>(v[14]);
    value[3][3] = static_cast<glm::dmat4x4::value_type>(v[15]);
    return success;
}

template <>
bool Dictionary::hasValue<bool>(const std::string& key) const {
    return hasValue<IntegralType>(key);
}

template <>
bool Dictionary::hasValue<char>(const std::string& key) const {
    return hasValue<IntegralType>(key);
}

template <>
bool Dictionary::hasValue<signed char>(const std::string& key) const {
    return hasValue<IntegralType>(key);
}

template <>
bool Dictionary::hasValue<unsigned char>(const std::string& key) const {
    return hasValue<UnsignedIntegralType>(key);
}

template <>
bool Dictionary::hasValue<wchar_t>(const std::string& key) const {
    return hasValue<IntegralType>(key);
}

template <>
bool Dictionary::hasValue<short>(const std::string& key) const {
    return hasValue<IntegralType>(key);
}

template <>
bool Dictionary::hasValue<unsigned short>(const std::string& key) const {
    return hasValue<UnsignedIntegralType>(key);
}

template <>
bool Dictionary::hasValue<int>(const std::string& key) const {
    return hasValue<IntegralType>(key);
}

template <>
bool Dictionary::hasValue<unsigned int>(const std::string& key) const {
    return hasValue<UnsignedIntegralType>(key);
}

template <>
bool Dictionary::hasValue<float>(const std::string& key) const {
    return hasValue<FloatingType>(key);
}

template <>
bool Dictionary::hasValue<glm::vec2>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 2>>(key);
}

template <>
bool Dictionary::hasValue<glm::dvec2>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 2>>(key);
}

template <>
bool Dictionary::hasValue<glm::ivec2>(const std::string& key) const {
    return hasValue<std::array<IntegralType, 2>>(key);
}

template <>
bool Dictionary::hasValue<glm::uvec2>(const std::string& key) const {
    return hasValue<std::array<UnsignedIntegralType, 2>>(key);
}

template <>
bool Dictionary::hasValue<glm::bvec2>(const std::string& key) const {
    return hasValue<std::array<IntegralType, 2>>(key);
}

template <>
bool Dictionary::hasValue<glm::vec3>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 3>>(key);
}

template <>
bool Dictionary::hasValue<glm::dvec3>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 3>>(key);
}

template <>
bool Dictionary::hasValue<glm::ivec3>(const std::string& key) const {
    return hasValue<std::array<IntegralType, 3>>(key);
}

template <>
bool Dictionary::hasValue<glm::uvec3>(const std::string& key) const {
    return hasValue<std::array<UnsignedIntegralType, 3>>(key);
}

template <>
bool Dictionary::hasValue<glm::bvec3>(const std::string& key) const {
    return hasValue<std::array<IntegralType, 3>>(key);
}

template <>
bool Dictionary::hasValue<glm::vec4>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 4>>(key);
}

template <>
bool Dictionary::hasValue<glm::dvec4>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 4>>(key);
}

template <>
bool Dictionary::hasValue<glm::ivec4>(const std::string& key) const {
    return hasValue<std::array<IntegralType, 4>>(key);
}

template <>
bool Dictionary::hasValue<glm::uvec4>(const std::string& key) const {
    return hasValue<std::array<UnsignedIntegralType, 4>>(key);
}

template <>
bool Dictionary::hasValue<glm::bvec4>(const std::string& key) const {
    return hasValue<std::array<IntegralType, 4>>(key);
}

template <>
bool Dictionary::hasValue<glm::mat2x2>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 4>>(key);
}

template <>
bool Dictionary::hasValue<glm::mat2x3>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 6>>(key);
}

template <>
bool Dictionary::hasValue<glm::mat2x4>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 8>>(key);
}

template <>
bool Dictionary::hasValue<glm::mat3x2>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 6>>(key);
}

template <>
bool Dictionary::hasValue<glm::mat3x3>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 9>>(key);
}

template <>
bool Dictionary::hasValue<glm::mat3x4>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 12>>(key);
}

template <>
bool Dictionary::hasValue<glm::mat4x2>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 8>>(key);
}

template <>
bool Dictionary::hasValue<glm::mat4x3>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 12>>(key);
}

template <>
bool Dictionary::hasValue<glm::mat4x4>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 16>>(key);
}

template <>
bool Dictionary::hasValue<glm::dmat2x2>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 4>>(key);
}

template <>
bool Dictionary::hasValue<glm::dmat2x3>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 6>>(key);
}

template <>
bool Dictionary::hasValue<glm::dmat2x4>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 8>>(key);
}

template <>
bool Dictionary::hasValue<glm::dmat3x2>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 6>>(key);
}

template <>
bool Dictionary::hasValue<glm::dmat3x3>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 9>>(key);
}

template <>
bool Dictionary::hasValue<glm::dmat3x4>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 12>>(key);
}

template <>
bool Dictionary::hasValue<glm::dmat4x2>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 8>>(key);
}

template <>
bool Dictionary::hasValue<glm::dmat4x3>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 12>>(key);
}

template <>
bool Dictionary::hasValue<glm::dmat4x4>(const std::string& key) const {
    return hasValue<std::array<FloatingType, 16>>(key);
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
