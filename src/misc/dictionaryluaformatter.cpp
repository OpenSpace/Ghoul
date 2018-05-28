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

#include <ghoul/misc/dictionaryluaformatter.h>

#include <ghoul/glm.h>
#include <ghoul/misc/dictionary.h>
#include <functional>
#include <numeric>
#include <string>

namespace {

std::string formatDouble(double d) {
    // This check is to silence -Wfloat-equal on GCC due to floating point comparison
    if (std::equal_to<>()(d, 0.0)) {
        return "0";
    }
    int exponent = static_cast<int>(std::log10(std::abs(d)));
    double base = d / std::pow(10, exponent);
    return std::to_string(base) + "E" + std::to_string(exponent);
}

}  // namespace

namespace ghoul {

DictionaryLuaFormatter::LuaFormattingError::LuaFormattingError(const std::string& msg)
    : RuntimeError(msg, "Dictionary")
{}

std::string DictionaryLuaFormatter::format(const Dictionary& dictionary) const {
    if (dictionary.empty()) {
        return "{}";
    }

    auto convert = [this, dictionary](const std::string& key) -> std::string {
        return key + "=" + formatValue(dictionary, key);
    };

    std::vector<std::string> keys = dictionary.keys();

    std::string lua = std::accumulate(
        std::next(keys.begin()),
        keys.end(),
        convert(*keys.begin()),
        [convert](std::string a, std::string key) {
            return a + "," + convert(key);
        }
    );

    return "{" + lua + "}";
}

std::string DictionaryLuaFormatter::formatValue(const Dictionary& dictionary,
                                                 const std::string& key) const
{
    if (dictionary.hasValue<Dictionary>(key)) {
        Dictionary subDictionary = dictionary.value<Dictionary>(key);
        return format(subDictionary);
    }

    if (dictionary.hasValue<glm::dvec4>(key)) {
        glm::dvec4 vec = dictionary.value<glm::dvec4>(key);
        return "{" + formatDouble(vec.x) + "," +
            formatDouble(vec.y) + "," +
            formatDouble(vec.z) + "," +
            formatDouble(vec.w) + "}";
    }

    if (dictionary.hasValue<glm::dvec3>(key)) {
        glm::dvec3 vec = dictionary.value<glm::dvec3>(key);
        return "{" + formatDouble(vec.x) + "," +
            formatDouble(vec.y) + "," +
            formatDouble(vec.z) + "}";
    }

    if (dictionary.hasValue<glm::dvec2>(key)) {
        glm::dvec2 vec = dictionary.value<glm::dvec2>(key);
        return "{" + formatDouble(vec.x) + "," +
            formatDouble(vec.y) + "}";
    }

    if (dictionary.hasValue<double>(key)) {
        double value = dictionary.value<double>(key);
        return formatDouble(value);
    }

    if (dictionary.hasValue<int>(key)) {
        int value = dictionary.value<int>(key);
        return std::to_string(value);
    }

    if (dictionary.hasValue<std::string>(key)) {
        std::string value = dictionary.value<std::string>(key);

        std::string luaString;
        for (const char& c : value) {
            switch (c) {
            case '"':
                luaString += "\\\"";
                break;
            case '\\':
                luaString += "\\\\";
                break;
            case '\b':
                luaString += "\\b";
                break;
            case '\f':
                luaString += "\\f";
                break;
            case '\n':
                luaString += "\\n";
                break;
            case '\r':
                luaString += "\\r";
                break;
            case '\t':
                luaString += "\\t";
                break;
            default:
                luaString += c;
            }
        }

        return "\"" + luaString + "\"";
    }

    throw LuaFormattingError(
        "Key '" + key + "' has invalid type for formatting dictionary as lua"
    );
}

}  // namespace ghoul
