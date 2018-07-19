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

#include <ghoul/logging/log.h>

#include <chrono>
#include <fmt/format.h>

#ifdef WIN32
#include <Windows.h>
#else // WIN32
#include <iomanip>
#include <sstream>
#include <sys/time.h>
#endif // WIN32

namespace ghoul::logging {

Log::Log(TimeStamping timeStamping, DateStamping dateStamping,
         CategoryStamping categoryStamping, LogLevelStamping logLevelStamping,
         LogLevel minimumLogLevel)
    : _timeStamping(timeStamping)
    , _dateStamping(dateStamping)
    , _categoryStamping(categoryStamping)
    , _logLevelStamping(logLevelStamping)
    , _logLevel(minimumLogLevel)
{}

bool Log::isTimeStamping() const {
    return _timeStamping;
}

void Log::setTimeStamping(TimeStamping timeStamping) {
    _timeStamping = timeStamping;
}

bool Log::isDateStamping() const {
    return _dateStamping;
}

void Log::setDateStamping(DateStamping dateStamping) {
    _dateStamping = dateStamping;
}

bool Log::isCategoryStamping() const {
    return _categoryStamping;
}

void Log::setCategoryStamping(CategoryStamping categoryStamping) {
    _categoryStamping = categoryStamping;
}

bool Log::isLogLevelStamping() const {
    return _logLevelStamping;
}

void Log::setLogLevelStamping(LogLevelStamping logLevelStamping) {
    _logLevelStamping = logLevelStamping;
}

LogLevel Log::logLevel() const {
    return _logLevel;
}

std::string Log::timeString() const {
#ifdef WIN32
    SYSTEMTIME t = {};
    GetLocalTime(&t);

    return fmt::format(
        "{:0>2}:{:0>2}:{:0>2}.{:0<3}", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds
    );
#else
    struct timeval t;
    gettimeofday(&t, nullptr);
    tm* m = gmtime(&t.tv_sec);

    return fmt::format(
        "{:0>2}:{:0>2}:{:0>2}.{:0<3}", m->tm_hour, m->tm_min, m->tm_sec, t.tv_usec / 1000
    );
#endif
}

std::string Log::dateString() const {
#ifdef WIN32
    SYSTEMTIME t = {};
    GetLocalTime(&t);

    return fmt::format("{}-{:0>2}-{:0>2}", t.wYear, t.wMonth, t.wDay);
#else
    auto now = std::chrono::system_clock::now();
    time_t time = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;

    ss << std::put_time(std::localtime(&time), "%F");
    return ss.str();
#endif
}

std::string Log::createFullMessageString(LogLevel level, const std::string& category,
                                         const std::string& message) const
{
    std::string output;
    if (isDateStamping()) {
        output += "[" + dateString();
    }
    if (isTimeStamping()) {
        output += " | " + timeString();
    }

    if (isDateStamping() || isTimeStamping()) {
        output += "] ";
    }
    if (isCategoryStamping() && (!category.empty())) {
        output += category + " ";
    }
    if (isLogLevelStamping()) {
        output += "(" + stringFromLevel(level) + ")";
    }
    if (!output.empty()) {
        output += "\t";
    }
    output += message;

    return output;
}

void Log::flush() {}

} // namespace ghoul::logging
