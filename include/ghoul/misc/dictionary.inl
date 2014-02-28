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

#include <ghoul/glm.h>
#include <ghoul/logging/logmanager.h>

namespace ghoul {

template <typename T>
bool Dictionary::setValue(const std::string& key, T&& value, bool createIntermediate) {
	std::string first;
	std::string rest;
	const bool hasRestPath = splitKey(key, first, rest);
	if (!hasRestPath) {
        // if no rest exists,  key == first
        // and we can just insert the value
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
		LERRORC("Dictionary", "Error converting key '" << first <<
			"' to type 'Dictionary', was '" << keyIt->second.type().name() << "'");
		return false;
	}
	// Proper tail-recursion
	return dict->setValue(rest, value);
}
	
template <typename T>
bool Dictionary::getValue(const std::string& key, T& value) const {
    // If we can find the key directly, we can return it immediately
	const std::map<std::string, boost::any>::const_iterator it = find(key);
	if (it != cend()) {
		const T* const v = boost::any_cast<T>(&(it->second));
        // See if it has the correct type
		if (v == nullptr) {
			LERRORC("Dictionary", "Wrong type of key '" << key << "': Expected '" <<
				typeid(T).name() << "', got '" << it->second.type().name() << "'");
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
		LERRORC("Dictionary", "Key '" << first << "' was not found in dictionary");
		return false;
	}

	const Dictionary* const dict = boost::any_cast<Dictionary>(&(keyIt->second));
    // See if it is actually a Dictionary at this location
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

} // namespace ghoul
