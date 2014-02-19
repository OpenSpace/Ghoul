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

#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#include <boost/any.hpp>
#include <map>
#include <string>
#include <vector>

namespace ghoul {

/*
 * TODO:
 * - documentation
 */


class Dictionary : private std::map<std::string, boost::any> {
public:
	Dictionary();
	Dictionary(std::initializer_list<std::pair<std::string, boost::any>> l);

	// recursive
	const std::vector<std::string> keys(const std::string& location = "") const;

	// recursive
	bool hasKey(const std::string& key) const;

	// recursive (if intermediates exist)
    template <typename T>
    bool setValue(const std::string& key, T&& value);

	// recursive
    template <typename T>
    bool getValue(const std::string& key, T& value) const;

	// recursive
	template <typename T>
	bool hasValue(const std::string& key) const;

	// on fail: typeid(void)
	const std::type_info& type(const std::string& key) const;

	// total number of stored values (no recursion)
    size_t size() const;

private:
	// returns if there exists a rest
	bool splitKey(const std::string& key, std::string& first, std::string& rest) const;
};

} // namespace ghoul

#include "dictionary.inl"

#endif
