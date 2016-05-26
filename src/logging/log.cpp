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

Log::Log(TimeStamping timeStamping, DateStamping dateStamping, 
         CategoryStamping categoryStamping, LogLevelStamping logLevelStamping)
    : _timeStamping(timeStamping == TimeStamping::Yes ? true : false)
    , _dateStamping(dateStamping == DateStamping::Yes ? true : false)
    , _categoryStamping(categoryStamping == CategoryStamping::Yes ? true : false)
    , _logLevelStamping(logLevelStamping == LogLevelStamping::Yes ? true : false)
{}

bool Log::isTimeStamping() const {
    return _timeStamping;
}

void Log::setTimeStamping(TimeStamping timeStamping) {
    _timeStamping = timeStamping == TimeStamping::Yes ? true : false;
}

bool Log::isDateStamping() const {
    return _dateStamping;
}

void Log::setDateStamping(DateStamping dateStamping) {
    _dateStamping = dateStamping == DateStamping::Yes ? true : false;
}

bool Log::isCategoryStamping() const {
    return _categoryStamping;
}

void Log::setCategoryStamping(CategoryStamping categoryStamping) {
    _categoryStamping = categoryStamping == CategoryStamping::Yes ? true : false;
}

bool Log::isLogLevelStamping() const {
    return _logLevelStamping;
}

void Log::setLogLevelStamping(LogLevelStamping logLevelStamping) {
    _logLevelStamping = logLevelStamping == LogLevelStamping::Yes ? true : false;
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

std::string Log::createFullMessageString(LogManager::LogLevel level,
                                         const std::string& category,
                                         const std::string& message) const
{
    std::string output;
    if (isDateStamping())
        output += "[" + getDateString();
    if (isTimeStamping())
        output += " | " + getTimeString();

    if (isDateStamping() || isTimeStamping())
        output += "] ";
    if (isCategoryStamping() && (category != ""))
        output += category + " ";
    if (isLogLevelStamping())
        output += "(" + LogManager::stringFromLevel(level) + ")";
    if (output != "")
        output += "\t";
    output += message;

    return output;
}

void Log::flush() {}

} // namespace logging
} // namespace ghoul
