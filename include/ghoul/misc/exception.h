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

#ifndef __GHOUL___EXCEPTION___H__
#define __GHOUL___EXCEPTION___H__

#include <stdexcept>
#include <string>

namespace ghoul {

/**
 * Superclass for all exceptions that are thrown in Ghoul. The total message of the
 * exception consists of the <code>message</code> prefixed with the <code>component</code>
 * that threw the exception if it was set.
 */
struct RuntimeError : public std::runtime_error {
    /**
     * Main constructor constructing the exception with the provided \p message and
     * \p component.
     * \param message The main message of the exception
     * \param component The optional component
     * \pre \p message must not be empty
     */
    explicit RuntimeError(std::string message, std::string component = "");

    /// The main message describing the exception
    std::string message;

    /// The name of the component that threw the exception
    std::string component;
};

/**
 * Exception that is thrown if an IO access failed because a file could was not found
 */
struct FileNotFoundError : public RuntimeError {
    /**
     * Main constructor constructing the exception with the provided missing \p file and
     * the \p component that threw the exception
     * \param file The file that was missing which caused this exception to be thrown
     * \param component The optional compoment that caused this exception to be thrown
     * \pre \p file must not be empty
     */
    explicit FileNotFoundError(std::string file, std::string component = "");

    /// The file that was missing
    std::string file;
};

} // namespace ghoul

#endif // __GHOUL___EXCEPTION___H__
