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

// Defines
#define GHL_ASSERT_FILE __FILE__
#define GHL_ASSERT_LINE __LINE__
#if defined(__GNUC__) || defined(__clang__)
#  define GHL_ASSERT_FUNCTION __PRETTY_FUNCTION__
#else
#  define GHL_ASSERT_FUNCTION __FUNCTION__
#endif

#include <ghoul/misc/exception.h>

#include <string>
#include <sstream>
#include <cstdio>
#include <exception>
#include <stdexcept>

#ifdef WIN32
// Check if this is still true with Visual Studio 2015 ---abock
#define GHL_NOEXCEPT
#else
#define GHL_NOEXCEPT noexcept
#endif // WIN32

#ifdef WIN32
#define GHL_NORETURN
#elif __APPLE__
// This tells the clang static analyzer that internal_assert is just like assert itself
#define GHL_NORETURN __attribute__((analyzer_noreturn))
#else
#define GHL_NORETURN
#endif

namespace ghoul {

class AssertException : public RuntimeError {
public:
    AssertException();
};

	/**
	 * Ghoul internal assert command. Is called  by
	 * the ghoul_assert methods.
	 * macro.
	 */
	void internal_assert(
		const std::string& expression,
		const std::string& message, 
		const std::string& file, 
		const std::string& function, 
		int line) GHL_NORETURN;


} // namespace ghoul

#if !defined(NDEBUG)

/**
* @defgroup ASSERT_MACRO_GROUP Assertion Macros
*
* @{
*/

/**
 * Assertion that prints the message (if provided) and gives the option of aborting,
 * exiting or ignoring the assertion. Not defined in release mode.
 */
#define ghoul_assert(__condition__, __message__) \
	do { \
		if(!(__condition__)) { \
			std::ostringstream oss; \
			oss << __message__; \
			ghoul::internal_assert(#__condition__, oss.str(), GHL_ASSERT_FILE, GHL_ASSERT_FUNCTION, GHL_ASSERT_LINE);\
		} \
	} while (false)


/** @} */
#else 

// The assertion macros defined for release mode
#define ghoul_assert(__condition__, __message__)

#endif // !defined(NDEBUG)

#endif // __ASSERT_H__
