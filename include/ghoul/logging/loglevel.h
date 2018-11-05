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

#ifndef __GHOUL___LOGLEVEL___H__
#define __GHOUL___LOGLEVEL___H__

#include <ghoul/misc/stringconversion.h>
#include <string>

namespace ghoul::logging {

/**
 * Enumerates all available LogLevel for the LogManager. The LogLevels are guaranteed
 * to be strictly ordered from least important to important.
 */
enum class LogLevel {
    /**
     * All log messages are accepted.
     */
    AllLogging = 0,
    /**
     * Used for high-throughput messages, for example to trace function calls.
     */
    Trace = 1,
    /**
     * Used for Debug output.
     */
    Debug = 2,
    /**
     * Used for informational messages which can be ignored, but might be informative.
     */
    Info = 3,
    /**
     * Warnings which do not represent a problem in itself, but may hint to a wrong
     * configuration.
     */
    Warning = 4,
    /**
     * Errors which will pose problems, but do not necessarily require the immediate
     * end of the application.
     */
    Error = 5,
    /**
     * Error which will be so severe that the application cannot recover from them.
     */
    Fatal = 6,
    /**
     * Used as a placeholder to inhibit all LogMessages.
     */
    NoLogging = 7
};

} // namespace ghoul::logging

namespace ghoul {

/**
* Returns the string representation of the passed LogLevel. The name of
* each level is equal to its enum value.
*
* \return The string representation of the passed LogLevel
*/
std::string to_string(logging::LogLevel level);

/**
* Returns the LogLevel for the passed string representation. The name of
* each level is equal to its enum value.
*
* \return The the LogLevel for the passed string representation
*/
template <>
logging::LogLevel from_string(const std::string& level);


} // namespace

#endif // __GHOUL___LOGLEVEL___H__
