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

#include <ghoul/logging/loglevel.h>

#include <ghoul/misc/assert.h>

#include <map>

namespace ghoul::logging {

std::string stringFromLevel(LogLevel level) {
    switch (level) {
        case LogLevel::AllLogging: return "All";
        case LogLevel::Trace:      return "Trace";
        case LogLevel::Debug:      return "Debug";
        case LogLevel::Info:       return "Info";
        case LogLevel::Warning:    return "Warning";
        case LogLevel::Error:      return "Error";
        case LogLevel::Fatal:      return "Fatal";
        case LogLevel::NoLogging:  return "None";
        default:                   throw MissingCaseException();

    }
}

LogLevel levelFromString(const std::string& level) {
    static const std::map<std::string, LogLevel> levels = {
        { "All"    , LogLevel::AllLogging },
        { "Trace"  , LogLevel::Trace },
        { "Debug"  , LogLevel::Debug },
        { "Info"   , LogLevel::Info },
        { "Warning", LogLevel::Warning },
        { "Error"  , LogLevel::Error },
        { "Fatal"  , LogLevel::Fatal },
        { "None"   , LogLevel::NoLogging }
    };

    auto it = levels.find(level);
    ghoul_assert(it != levels.end(), "Missing entry in 'levels' map");
    return it->second;
}

} // namespace ghoul::logging
