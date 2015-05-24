/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2015                                                               *
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

#ifndef __MISC_H__
#define __MISC_H__

#include <string>
#include <vector>

namespace ghoul {

/**
 * Separates the provided <code>input</code> URI into the separate parts that are
 * added into the passed <code>tokens</code> vector. If <code>input</code> is
 * <code>a.b.c.d 1.e</code>, <code>tokens</code> will contain one entry for
 * <code>a</code>, <code>b</code>, <code>c</code>, <code>d 1</code>, and
 * <code>e</code>. Enough space in the vector will automatically be allocated. If an
 * error occurred, <code>false</code> will be returned and the <code>tokens</code>
 * vector will be empty.
 * \param input The input URI that is to be tokenized using the <code>separator</code>
 * \param separator The separator that is used for tokenize the string
 * \return The results of the tokenization operation
 */
std::vector<std::string> tokenizeString(
    const std::string& input,
    const char separator);

} // namespace ghoul

#endif // __MISC_H__
