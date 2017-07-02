/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2017                                                               *
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

#include <ghoul/misc/dictionaryjsonformatter.h>

#include <ghoul/misc/dictionary.h>

#include <ghoul/glm.h>

#include <numeric>
#include <string>

namespace ghoul {

std::string DictionaryJsonFormatter::format(const Dictionary& dictionary) const {
    if (dictionary.empty()) {
        return "{}";
    }

    auto convert = [this, dictionary](const std::string& key) -> std::string {
        return "\"" + key + "\":" + formatValue(dictionary, key);
    };

    std::vector<std::string> keys = dictionary.keys();
    
    std::string json = std::accumulate(
        std::next(keys.begin()),
        keys.end(),
        convert(*keys.begin()),
        [convert](std::string a, std::string key) {
            return a + "," + convert(key);
        }
    );

    return "{" + json + "}";
}

std::string DictionaryJsonFormatter::formatDouble(double d) const {
    if (d == 0) {
        return "0";
    }
    double exponent = std::log10(std::abs(d));
    double base = d / std::pow(10, exponent);
    return std::to_string(base) + "E" + std::to_string(exponent);
}

std::string DictionaryJsonFormatter::formatValue(const Dictionary& dictionary,
                                                 const std::string& key) const
{
    if (dictionary.hasValue<Dictionary>(key)) {
        Dictionary subDictionary = dictionary.value<Dictionary>(key);
        return format(subDictionary);
    }

    if (dictionary.hasValue<glm::vec4>(key)) {
        glm::vec4 vec = dictionary.value<glm::vec4>(key);
        return "[" + formatDouble(vec.x) + "," +
            formatDouble(vec.y) + "," +
            formatDouble(vec.z) + "," +
            formatDouble(vec.w) + "]";
    }

    if (dictionary.hasValue<glm::vec3>(key)) {
        glm::vec3 vec = dictionary.value<glm::vec3>(key);
        return "[" + formatDouble(vec.x) + "," +
            formatDouble(vec.y) + "," +
            formatDouble(vec.z) + "]";
    }

    if (dictionary.hasValue<glm::vec2>(key)) {
        glm::vec2 vec = dictionary.value<glm::vec2>(key);
        return "[" + formatDouble(vec.x) + "," +
            formatDouble(vec.y) + "]";
    }

    if (dictionary.hasValue<float>(key)) {
        float value = dictionary.value<float>(key);
        return formatDouble(value);
    }

    if (dictionary.hasValue<int>(key)) {
        int value = dictionary.value<int>(key);
        return std::to_string(value);
    }

    if (dictionary.hasValue<std::string>(key)) {
        std::string value = dictionary.value<std::string>(key);

        std::string jsonString = "";
        for (const char& c : value) {
            switch (c) {
            case '"':
                jsonString += "\\\"";
                break;
            case '\\':
                jsonString += "\\\\";
                break;
            case '\b':
                jsonString += "\\b";
                break;
            case '\f':
                jsonString += "\\f";
                break;
            case '\n':
                jsonString += "\\n";
                break;
            case '\r':
                jsonString += "\\r";
                break;
            case '\t':
                jsonString += "\\t";
                break;
            default:
                jsonString += c;
            }
        }

        return "\"" + jsonString + "\"";
    }

    throw JsonFormattingError(
        "Key '" + key + "' has invalid type for formatting dictionary as json"
    );
}

DictionaryJsonFormatter::JsonFormattingError::JsonFormattingError(
    const std::string& message)
    : RuntimeError(message, "Dictionary")
{}

}  // namespace ghoul
