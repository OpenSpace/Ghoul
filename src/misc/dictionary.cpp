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

using std::string;

namespace {
	const std::string _loggerCat = "Dictionary";
}

namespace ghoul {

//namespace dictionaryhelper {
//    std::string getValueAsString(Dictionary* D, std::string key);
//    std::string lua_serializer(Dictionary* D, const std::string indentation = "");
//} // namespace helper
    
Dictionary::Dictionary() {}

Dictionary::Dictionary(const std::initializer_list<std::pair<string, boost::any>>& l) {
	for (const auto& p : l)
		(*this)[p.first] = p.second;
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

const std::type_info& Dictionary::type(const std::string& key) const {
	auto it = find(key);
	if (it != cend())
		return it->second.type();

	std::string first;
	std::string rest;
	splitKey(key, first, rest);

	auto keyIt = find(first);
	if (keyIt == cend())
		return typeid(void);

	const Dictionary* const dict = boost::any_cast<Dictionary>(&(keyIt->second));
	if (!dict)
		return typeid(void);
	
	// Proper tail-recursion
	return dict->type(rest);
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
