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

#ifndef __GHOUL___DICTIONARYJSONFORMATTER___H__
#define __GHOUL___DICTIONARYJSONFORMATTER___H__

#include <ghoul/misc/dictionaryformatter.h>

#include <ghoul/misc/exception.h>

namespace ghoul {

class Dictionary;

/**
 * This concrete implementation of a DictionaryFormatter converts the passed Dictionary
 * into a JSON format.
 *
 * At the moment, the only types that are supported in this conversion are:
 * <code>glm::vec2</code>, <code>glm::vec3</code>, <code>glm::vec4</code>,
 * <code>float</code>, <code>int</code>, and <code>std::string</code>.
 */
class DictionaryJsonFormatter : public DictionaryFormatter {
public:
    /**
     * This exception is thrown if an unknown type is being converted.
     */
    struct JsonFormattingError : public RuntimeError {
        explicit JsonFormattingError(std::string msg);
    };

    /**
     * Converts the passed \p dictionary into a JSON string representation.
     *
     * \param dictionary The Dictionary that should be converted
     * \return A JSON string representing the Dictionary
     *
     * \throw JsonFormattingError If the \p key points to a type that cannot be converted
     */
    std::string format(const Dictionary& dictionary) const override;

private:
    /**
     * Converts a single value \p key out of the \p dictionary by manually iterating all
     * the types and trying to access them.
     *
     * \param dictionary The Dictionary from which the \p key should be extracted and
     *        converted
     * \param key The key in the Dictionary that should be converted
     * \return A JSON representation of the \p key's value
     *
     * \throw JsonFormattingError If the \p key points to a type that cannot be converted
     */
    std::string formatValue(const Dictionary& dictionary, const std::string& key) const;
};

}  // namespace ghoul

#endif // __GHOUL___DICTIONARYJSONFORMATTER___H__
