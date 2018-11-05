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

namespace ghoul {

std::string to_string(logging::LogLevel level) {
    switch (level) {
        case logging::LogLevel::AllLogging: return "All";
        case logging::LogLevel::Trace:      return "Trace";
        case logging::LogLevel::Debug:      return "Debug";
        case logging::LogLevel::Info:       return "Info";
        case logging::LogLevel::Warning:    return "Warning";
        case logging::LogLevel::Error:      return "Error";
        case logging::LogLevel::Fatal:      return "Fatal";
        case logging::LogLevel::NoLogging:  return "None";
        default:                   throw MissingCaseException();
    }
}

template <>
logging::LogLevel from_string(const std::string& level) {
    static const std::map<std::string, logging::LogLevel> levels = {
        { "All"    , logging::LogLevel::AllLogging },
        { "Trace"  , logging::LogLevel::Trace },
        { "Debug"  , logging::LogLevel::Debug },
        { "Info"   , logging::LogLevel::Info },
        { "Warning", logging::LogLevel::Warning },
        { "Error"  , logging::LogLevel::Error },
        { "Fatal"  , logging::LogLevel::Fatal },
        { "None"   , logging::LogLevel::NoLogging }
    };

    auto it = levels.find(level);
    ghoul_assert(it != levels.end(), "Missing entry in 'levels' map");
    return it->second;
}

} // namespace ghoul::logging
