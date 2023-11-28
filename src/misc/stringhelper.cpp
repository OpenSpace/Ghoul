/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2023                                                               *
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

#include <ghoul/misc/stringhelper.h>

#include <algorithm>
#include <cctype>

namespace ghoul {

void toUpperCase(std::string& s) {
    std::transform(
        s.begin(),
        s.end(),
        s.begin(),
        [](unsigned char c) { return std::toupper(c); }
    );
}

void toLowerCase(std::string& s) {
    std::transform(
        s.begin(),
        s.end(),
        s.begin(),
        [](unsigned char c) { return std::tolower(c); }
    );
}

std::vector<std::string> tokenizeString(const std::string& input, char separator) {
    size_t separatorPos = input.find(separator);
    if (separatorPos == std::string::npos) {
        return { input };
    }
    else {
        std::vector<std::string> result;
        size_t prevSeparator = 0;
        while (separatorPos != std::string::npos) {
            result.push_back(input.substr(prevSeparator, separatorPos - prevSeparator));
            prevSeparator = separatorPos + 1;
            separatorPos = input.find(separator, separatorPos + 1);
        }
        result.push_back(input.substr(prevSeparator));
        return result;
    }
}

std::string join(std::vector<std::string> input, const std::string& separator) {
    std::string result;
    for (std::string& s : input) {
        result += std::move(s) + separator;
    }

    return result.substr(0, result.size() - separator.size());
}

void trimWhitespace(std::string& value) {
    // Trim from the left until the first non-whitespace character
    value.erase(
        value.begin(),
        std::find_if(value.begin(), value.end(), [](int ch) { return !std::isspace(ch); })
    );

    // Trim from the right until the first non-whitespace character
    value.erase(
        std::find_if(
            value.rbegin(),
            value.rend(),
            [](int ch) { return !std::isspace(ch); }
        ).base(),
        value.end()
    );
}

void trimSurroundingCharacters(std::string& valueString, const char charToRemove) {
    while (valueString.front() == charToRemove) {
        valueString.erase(0, 1);
    }
    while (valueString.back() == charToRemove) {
        valueString.pop_back();
    }
}

std::string replaceAll(std::string string, const std::string& from,
                       const std::string& to)
{
    ghoul_assert(!from.empty(), "from must not be the empty string");

    size_t pos = string.find(from);
    while (pos != std::string::npos) {
        string.replace(pos, from.length(), to);

        // In case 'to' contains 'from', ex replacing 'x' with 'yx'
        size_t offset = pos + to.length();
        pos = string.find(from, offset);
    }
    return string;
}

std::string encodeUrl(const std::string& string) {
    std::string result;
    result = replaceAll(string, " ", "%20");
    result = replaceAll(result, "#", "%23");
    result = replaceAll(result, "$", "%24");
    result = replaceAll(result, "&", "%26");
    result = replaceAll(result, "+", "%2B");
    result = replaceAll(result, ",", "%2C");
    result = replaceAll(result, "/", "%2F");
    result = replaceAll(result, ":", "%3A");
    result = replaceAll(result, ";", "%3B");
    result = replaceAll(result, "=", "%3D");
    result = replaceAll(result, "?", "%3F");
    result = replaceAll(result, "@", "%40");
    result = replaceAll(result, "[", "%5B");
    result = replaceAll(result, "]", "%5D");
    return result;
}

} // namespace ghoul
