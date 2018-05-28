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

#ifndef __GHOUL___CLIPBOARD___H__
#define __GHOUL___CLIPBOARD___H__

#include <string>

namespace ghoul {

/**
 * This function retrieves the contents of the system-wide clipboard and returns the
 * textual representation. If the clipboard does not contain anything, or if there was an
 * error accessing the clipboard, an empty string is returned.
 *
 * \return The contents of the system-wide clipboard
 */
std::string clipboardText();

/**
 * Writes a passed text into the system-wide clipboard, overwriting its previous content.
 *
 * \param text The text that is to be written into the clipboard
 *
 * \throw RuntimeError If there was an error writing the text to the clipboard
 */
void setClipboardText(const std::string& text);

} // namespace ghoul

#endif // __GHOUL___CLIPBOARD___H__
