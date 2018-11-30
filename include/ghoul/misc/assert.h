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

#ifndef __GHOUL___ASSERT___H__
#define __GHOUL___ASSERT___H__

#include <string>
#include <stdexcept>

namespace ghoul {

/**
 * Exception that gets thrown if an assertion is triggered and the user selects the
 * <code>AssertionException</code> option.
 */
struct AssertionException : public std::runtime_error {
    explicit AssertionException(std::string exp, std::string msg, std::string file,
        std::string func, int line);
};

/**
 * Exception that gets thrown if switch-case statement is missing a case.
 */
struct MissingCaseException : public std::logic_error {
    MissingCaseException();
};

/**
 * Ghoul internal assert command. Is called by the ghoul_assert macro.
 *
 * \param expression The expression that caused the assertion
 * \param message The message that was provided for the assertion
 * \param file The file in which the assertion triggered
 * \param function The function in which the assertion triggered
 * \param line The line in the \p file that triggered the assertion
 */
#ifdef __APPLE__
// This tells the clang static analyzer that internal_assert is just like assert itself
void internal_assert(std::string expression, std::string message, std::string file,
    std::string function, int line);
#elif WIN32
// This tells Visual studio that internal_assert is just like assert itself
__declspec(noreturn) void internal_assert(std::string expression, std::string message,
    std::string file, std::string function, int line);
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
 * This macro asserts on the <code>__condition__</code> and prints the optional
 * <code>__message__</code>. In addition, it gives the option of aborting, exiting, or
 * ignoring the assertion. The macro is optimized away in Release mode. Due to this fact,
 * the <code>__condition__</code> must not have any sideeffects.
 */

#ifdef GHL_THROW_ON_ASSERT
#define ghoul_assert(__condition__, __message__)                                         \
    do {                                                                                 \
        if (!(__condition__)) {                                                          \
            throw ghoul::AssertionException(                                             \
                #__condition__,                                                          \
                __message__,                                                             \
                __FILE__,                                                                \
                GHL_ASSERT_FUNCTION,                                                     \
                __LINE__                                                                 \
            );                                                                           \
        }                                                                                \
    } while (false)
#else
#define ghoul_assert(__condition__, __message__)                                         \
    do {                                                                                 \
        if (!(__condition__)) {                                                          \
            ghoul::internal_assert(                                                      \
                #__condition__,                                                          \
                __message__,                                                             \
                __FILE__,                                                                \
                GHL_ASSERT_FUNCTION,                                                     \
                __LINE__                                                                 \
            );                                                                           \
        }                                                                                \
    } while (false)

#endif

#else

#define ghoul_assert(__condition__, __message__) {}

#endif // NDEBUG

/** @}  */

#endif // __GHOUL___ASSERT___H__
