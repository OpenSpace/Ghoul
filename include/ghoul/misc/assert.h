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

#ifndef __ASSERT_H__
#define __ASSERT_H__

#include <ghoul/misc/exception.h>

#include <string>
#include <sstream>

namespace ghoul {

/**
 * Exception that gets thrown if an assertion is triggered and the user selects the
 * <code>AssertionException</code> option
 */
struct AssertionException : public std::runtime_error {
    explicit AssertionException(std::string expression, std::string message,
                                std::string file, std::string function, int line);
};

/**
 * Ghoul internal assert command. Is called by the ghoul_assert macro.
 * \param expression The expression that caused the assertion
 * \param message The message that was provided for the assertion
 * \param file The file in which the assertion triggered
 * \param function The function in which the assertion triggered
 * \param line The line in the \p file that triggered the assertion
 */
#ifdef __APPLE__
// This tells the clang static analyzer that internal_assert is just like assert itself
void internal_assert(std::string expression, std::string message, std::string file,
    std::string function, int line) __attribute__((analyzer_noreturn));
#else
void internal_assert(std::string expression, std::string message, std::string file,
    std::string function, int line);
#endif // __APPLE__


} // namespace ghoul

#if !(defined(NDEBUG) || defined(DEBUG))

/**
* @defgroup ASSERT_MACRO_GROUP Assertion Macros
*
* @{
*/

#if defined(__GNUC__) || defined(__clang__)
#  define GHL_ASSERT_FUNCTION __PRETTY_FUNCTION__
#else
#  define GHL_ASSERT_FUNCTION __FUNCTION__
#endif

/**
 * Assertion that prints the message (if provided) and gives the option of aborting,
 * exiting or ignoring the assertion. Not defined in release mode.
 */
#define ghoul_assert(__condition__, __message__)                                         \
    do {                                                                                 \
		if (!(__condition__)) {                                                          \
			std::ostringstream oss;                                                      \
			oss << __message__;                                                          \
			ghoul::internal_assert(                                                      \
                #__condition__,                                                          \
                oss.str(),                                                               \
                __FILE__,                                                                \
                GHL_ASSERT_FUNCTION,                                                     \
                __LINE__                                                                 \
            );                                                                           \
		}                                                                                \
	} while (false)


#else 
#define ghoul_assert(__condition__, __message__)
#endif // NDEBUG

/** @}  */

#endif // __ASSERT_H__
