/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2022                                                               *
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

#ifndef __GHOUL___MISC___H__
#define __GHOUL___MISC___H__

#include <string>
#include <vector>

namespace ghoul {

/**
 * Separates the provided \p input URI into separate parts. If \p input is
 * <code>a.b.c.d 1.e</code>, the returned vector will contain one entry for
 * <code>a</code>, <code>b</code>, <code>c</code>, <code>d 1</code>, and <code>e</code>.
 * If an error occurred, an exception will be thrown.
 *
 * \param input The input URI that is to be tokenized using the \p separator
 * \param separator The separator that is used for tokenize the string
 * \return The results of the tokenization operation
 *
 * \throw RuntimeError If there was an error tokenizing the string
 */
std::vector<std::string> tokenizeString(const std::string& input, char separator = '.');

/**
 * Joins the strings located in the \p input using the provided \p separator and returns
 * the joined list.
 *
 * \param input The list of strings that will be joined
 * \param separator The separator that will be used in the joined string
 */
std::string join(std::vector<std::string> input, const std::string& separator = ".");

/**
 * Removes whitespace at the beginning and the end of the string.
 *
 * \param value The string from which to remove the whitespace
 */
void trimWhitespace(std::string& value);

/**
 * Removes a specific character from both the beginning and ending of a string. Any number
 * of the character(s) (if repeating) will be removed. Useful for strings that are
 * surrounded by unwanted characters such as quotes, brackets, etc.
 *
 * \param valueString The string from which to remove the unwanted characters.
 * \param charToRemove The character to remove from both the beginning & end of string
 */
void trimSurroundingCharacters(std::string& valueString, const char charToRemove);

} // namespace ghoul

#endif // __GHOUL___MISC___H__
