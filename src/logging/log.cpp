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

#include "ghoul/logging/log.h"
#include "ghoul/logging/logmanager.h"
#include <ctime>
#include <stdio.h>

namespace ghoul {
namespace logging {

Log::Log(bool timeStamping, bool dateStamping, bool categoryStamping,
         bool logLevelStamping)
    : _timeStamping(timeStamping)
    , _dateStamping(dateStamping)
    , _categoryStamping(categoryStamping)
    , _logLevelStamping(logLevelStamping)
{}

Log::~Log() {}

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
    time_t timeSinceEpoch = 0;
    time(&timeSinceEpoch);

    char buffer[256];
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
    tm localTime;
    localtime_s(&localTime, &timeSinceEpoch);
    sprintf_s(buffer, "%.2i:%.2i:%.2i",
              localTime.tm_hour, localTime.tm_min, localTime.tm_sec);
#else
    const tm* const localTime = localtime(&timeSinceEpoch);
    sprintf(buffer, "%.2i:%.2i:%.2i",
            localTime->tm_hour, localTime->tm_min, localTime->tm_sec);
#endif
    return std::string(buffer);
}

std::string Log::getDateString() const {
    time_t timeSinceEpoch = 0;
    time(&timeSinceEpoch);

    char buffer[256];
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
    tm localTime;
    localtime_s(&localTime, &timeSinceEpoch);
    sprintf_s(buffer, "%.2i-%.2i-%.2i",
              localTime.tm_year + 1900, localTime.tm_mon + 1, localTime.tm_mday);
#else
    const tm* const localTime = localtime(&timeSinceEpoch);
    sprintf(buffer, "%.2i:%.2i:%.2i",
            localTime->tm_mday, localTime->tm_mon + 1, localTime->tm_year + 1900);
#endif
    return std::string(buffer);
}

void Log::flush() {}

} // namespace logging
} // namespace ghoul
