/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012 Alexander Bock                                                     *
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

#include <ghoul/glm.h>

namespace ghoul {

template <typename T>
bool Dictionary::setValue(const std::string& key, T&& value) {
	std::string first;
	std::string rest;
	const bool hasRestPath = splitKey(key, first, rest);
	if (!hasRestPath) {
		// key == first
		_map[key] = value;
		return true;
	}

	const std::map<std::string, boost::any>::iterator keyIt = _map.find(first);
	if (keyIt == _map.cend()) {
		LERRORC("Dictionary", "Key '" << first << "' was not found in dictionary");
		return false;
	}

	Dictionary* const dict = boost::any_cast<Dictionary>(&(keyIt->second));
	if (dict == nullptr) {
		LERRORC("Dictionary", "Error converting key '" << first <<
			"' to type 'Dictionary', was '" << keyIt->second.type().name() << "'");
		return false;
	}
	// Proper tail-recursion
	return dict->setValue(rest, value);
}
	
template <typename T>
bool Dictionary::getValue(const std::string& key, T& value) const {
	const std::map<std::string, boost::any>::const_iterator it = _map.find(key);
	if (it != _map.cend()) {
		const T* const v = boost::any_cast<T>(&(it->second));
		if (v == nullptr) {
			LERRORC("Dictionary", "Wrong type of key '" << key << "': Expected '" <<
				typeid(T).name() << "', got '" << it->second.type().name() << "'");
			return false;
		}
		value = *v;
		return true;
	}

	std::string first;
	std::string rest;
	splitKey(key, first, rest);

	const std::map<std::string, boost::any>::const_iterator keyIt = _map.find(first);
	if (keyIt == _map.cend()) {
		LERRORC("Dictionary", "Key '" << first << "' was not found in dictionary");
		return false;
	}

	const Dictionary* const dict = boost::any_cast<Dictionary>(&(keyIt->second));
	if (dict == nullptr) {
		LERRORC("Dictionary", "Error converting key '" << first << 
			"' to type 'Dictionary', was '" << keyIt->second.type().name() << "'");
		return false;
	}
	// Proper tail-recursion
	return dict->getValue<T>(rest, value);
}

template <typename T>
bool Dictionary::hasValue(const std::string& key) const {
	const std::map<std::string, boost::any>::const_iterator it = _map.find(key);
	if (it != _map.cend())
		return (it->second.type() == typeid(T));

	std::string first;
	std::string rest;
	splitKey(key, first, rest);

	const std::map<std::string, boost::any>::const_iterator keyIt = _map.find(first);
	if (keyIt == _map.cend())
		return false;

	const Dictionary* const dict = boost::any_cast<Dictionary>(&(keyIt->second));
	if (dict == nullptr)
		return false;
	// Proper tail-recursion
	return dict->hasValue<T>(rest);
}

//
//#ifdef WIN32
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const bool& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const char& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const signed char& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const unsigned char& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const wchar_t& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const short& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const unsigned short& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const int& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const unsigned int& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const long& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const unsigned long& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const long long& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const unsigned long long& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const float& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const double& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const long double& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const std::string& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const std::string& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::vec2& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::vec3& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::vec4& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::dvec2& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::dvec3& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::dvec4& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::ivec2& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::ivec3& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::ivec4& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::uvec2& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::uvec3& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::uvec4& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::bvec2& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::bvec3& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::bvec4& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::mat2x2& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::mat2x3& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::mat2x4& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::mat3x2& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::mat3x3& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::mat3x4& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::mat4x2& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::mat4x3& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::mat4x4& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::dmat2x2& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::dmat2x3& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::dmat2x4& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::dmat3x2& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::dmat3x3& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::dmat3x4& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::dmat4x2& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::dmat4x3& value);
//extern template void Dictionary::setValue<>(
//                            const std::string& key, const glm::dmat4x4& value);
//
//extern template bool Dictionary::getValue<>(
//							const std::string& key, bool& value) const;
//extern template bool Dictionary::getValue<>(
//							const std::string& key, char& value) const;
//extern template bool Dictionary::getValue<>(
//							const std::string& key, signed char& value) const;
//extern template bool Dictionary::getValue<>(
//							const std::string& key, unsigned char& value) const;
//extern template bool Dictionary::getValue<>(
//							const std::string& key, wchar_t& value) const;
//extern template bool Dictionary::getValue<>(
//							const std::string& key, short& value) const;
//extern template bool Dictionary::getValue<>(
//							const std::string& key, unsigned short& value) const;
//extern template bool Dictionary::getValue<>(
//							const std::string& key, int& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, unsigned int& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, long& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, unsigned long& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, long long& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, unsigned long long& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, float& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, double& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, long double& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, std::string& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, glm::vec2& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, glm::vec3& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, glm::vec4& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, glm::dvec2& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, glm::dvec3& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, glm::dvec4& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, glm::ivec2& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, glm::ivec3& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, glm::ivec4& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, glm::uvec2& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, glm::uvec3& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, glm::uvec4& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, glm::bvec2& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, glm::bvec3& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, glm::bvec4& value) const;
//extern template bool Dictionary::getValue<>(
//	const std::string& key, glm::mat2x2& value) const;
//extern template bool Dictionary::getValue<>(
//                            const std::string& key, glm::mat2x3& value);
//extern template bool Dictionary::getValue<>(
//                            const std::string& key, glm::mat2x4& value);
//extern template bool Dictionary::getValue<>(
//                            const std::string& key, glm::mat3x2& value);
//extern template bool Dictionary::getValue<>(
//                            const std::string& key, glm::mat3x3& value);
//extern template bool Dictionary::getValue<>(
//                            const std::string& key, glm::mat3x4& value);
//extern template bool Dictionary::getValue<>(
//                            const std::string& key, glm::mat4x2& value);
//extern template bool Dictionary::getValue<>(
//                            const std::string& key, glm::mat4x3& value);
//extern template bool Dictionary::getValue<>(
//                            const std::string& key, glm::mat4x4& value);
//extern template bool Dictionary::getValue<>(
//                            const std::string& key, glm::dmat2x2& value);
//extern template bool Dictionary::getValue<>(
//                            const std::string& key, glm::dmat2x3& value);
//extern template bool Dictionary::getValue<>(
//                            const std::string& key, glm::dmat2x4& value);
//extern template bool Dictionary::getValue<>(
//                            const std::string& key, glm::dmat3x2& value);
//extern template bool Dictionary::getValue<>(
//                            const std::string& key, glm::dmat3x3& value);
//extern template bool Dictionary::getValue<>(
//                            const std::string& key, glm::dmat3x4& value);
//extern template bool Dictionary::getValue<>(
//                            const std::string& key, glm::dmat4x2& value);
//extern template bool Dictionary::getValue<>(
//                            const std::string& key, glm::dmat4x3& value);
//extern template bool Dictionary::getValue<>(
//                            const std::string& key, glm::dmat4x4& value);
//#endif

} // namespace ghoul
