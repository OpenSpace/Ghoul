/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2024                                                               *
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
#include <cmath>
#include <functional>
#include <numeric>
#include <string>

namespace ghoul {

namespace {
    std::string formatValue(const Dictionary& dictionary, const std::string& key,
                            PrettyPrint prettyPrint, const std::string& indentation,
                            int indentationSteps);

    std::string format(const Dictionary& d, PrettyPrint prettyPrint,
                       const std::string& indentation, int indentationSteps)
    {
        if (d.isEmpty()) {
            return "{}";
        }

        std::string indent;
        if (prettyPrint) {
            for (int i = 0; i < indentationSteps; i++) {
                indent += indentation;
            }
        }

        auto convert = [&](const std::string& key) {
            return
                (prettyPrint ? indentation : "") + key +
                (prettyPrint ? " " : "") + "=" + (prettyPrint ? " " : "") +
                formatValue(d, key, prettyPrint, indentation, indentationSteps + 1);
        };

        std::vector<std::string> keys;
        for (std::string_view k : d.keys()) {
            keys.emplace_back(k);
        }

        std::string lua = std::accumulate(
            std::next(keys.begin()),
            keys.end(),
            convert(*keys.begin()),
            [&](const std::string& a, const std::string& key) {
                return a + "," + (prettyPrint ? "\n" : "") + indent + convert(key);
            }
        );

        return std::string("{") + (prettyPrint ? "\n" : "") + indent + lua +
            (prettyPrint ? "\n" : "") + indent + "}";
    }

    template <typename T>
    std::string formatVector(const std::vector<T>& vec) {
        static_assert(
            std::is_same_v<T, double> || std::is_same_v<T, int>,
            "Only double or ints allowed"
        );

        if (vec.empty()) {
            return "{}";
        }

        std::string values;
        for (size_t i = 0; i < vec.size() - 1; i++) {
            values += fmt::format("{},", vec[i]);
        }
        values += fmt::format("{}", vec.back());

        return fmt::format("{{{}}}", values);
    }

    std::string formatValue(const Dictionary& dictionary, const std::string& key,
                            PrettyPrint prettyPrint, const std::string& indentation,
                            int indentationSteps)
    {
        if (dictionary.hasValue<Dictionary>(key)) {
            const Dictionary subDictionary = dictionary.value<Dictionary>(key);
            return format(subDictionary, prettyPrint, indentation, indentationSteps);
        }

        if (dictionary.hasValue<double>(key)) {
            const double value = dictionary.value<double>(key);
            return fmt::format("{}", value);
        }

        if (dictionary.hasValue<int>(key)) {
            const int value = dictionary.value<int>(key);
            return std::to_string(value);
        }

        if (dictionary.hasValue<bool>(key)) {
            const bool value = dictionary.value<bool>(key);
            return value ? "true" : "false";
        }

        if (dictionary.hasValue<std::vector<int>>(key)) {
            const std::vector<int> vec = dictionary.value<std::vector<int>>(key);
            return formatVector(vec);
        }

        if (dictionary.hasValue<std::vector<double>>(key)) {
            const std::vector<double> vec = dictionary.value<std::vector<double>>(key);
            return formatVector(vec);
        }

        if (dictionary.hasValue<std::string>(key)) {
            const std::string value = dictionary.value<std::string>(key);

            std::string luaString;
            for (const char c : value) {
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

        throw LuaFormattingError(fmt::format(
            "Key '{}' has invalid type for formatting dictionary as Lua", key
        ));
    }
}  // namespace

LuaFormattingError::LuaFormattingError(const std::string& msg)
    : RuntimeError(msg, "Dictionary")
{}

std::string formatLua(const Dictionary& dictionary, PrettyPrint prettyPrint,
                      const std::string& indentation)
{
    return format(dictionary, prettyPrint, indentation, 0);
}

}  // namespace ghoul
