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
		(*this)[key] = value;
		return true;
	}

	const std::map<std::string, boost::any>::iterator keyIt = find(first);
	if (keyIt == cend()) {
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
	const std::map<std::string, boost::any>::const_iterator it = find(key);
	if (it != cend()) {
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

	const std::map<std::string, boost::any>::const_iterator keyIt = find(first);
	if (keyIt == cend()) {
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
	const std::map<std::string, boost::any>::const_iterator it = find(key);
	if (it != cend())
		return (it->second.type() == typeid(T));

	std::string first;
	std::string rest;
	splitKey(key, first, rest);

	const std::map<std::string, boost::any>::const_iterator keyIt = find(first);
	if (keyIt == cend())
		return false;

	const Dictionary* const dict = boost::any_cast<Dictionary>(&(keyIt->second));
	if (dict == nullptr)
		return false;
	// Proper tail-recursion
	return dict->hasValue<T>(rest);
}

} // namespace ghoul
