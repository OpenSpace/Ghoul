/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2016                                                               *
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

#include <ghoul/logging/log.h>

#include <chrono>
#include <iomanip>

namespace ghoul {
namespace logging {

Log::Log(bool timeStamping, bool dateStamping, bool categoryStamping,
                                                                    bool logLevelStamping)
    : _timeStamping(timeStamping)
    , _dateStamping(dateStamping)
    , _categoryStamping(categoryStamping)
    , _logLevelStamping(logLevelStamping)
{}

bool Log::isTimeStamping() const {
    return _timeStamping;
}

void Log::setTimeStamping(bool timeStamping) {
    _timeStamping = timeStamping;
}

bool Log::isDateStamping() const {
    return _dateStamping;
}

void Log::setDateStamping(bool dateStamping) {
    _dateStamping = dateStamping;
}

bool Log::isCategoryStamping() const {
    return _categoryStamping;
}

void Log::setCategoryStamping(bool categoryStamping) {
    _categoryStamping = categoryStamping;
}

bool Log::isLogLevelStamping() const {
    return _logLevelStamping;
}

void Log::setLogLevelStamping(bool logLevelStamping) {
    _logLevelStamping = logLevelStamping;
}

std::string Log::getTimeString() const {
    auto now = std::chrono::system_clock::now();
    time_t time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%T");
    return ss.str();
}

std::string Log::getDateString() const {
    auto now = std::chrono::system_clock::now();
    time_t time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%F");
    return ss.str();
}

void Log::flush() {}

} // namespace logging
} // namespace ghoul
