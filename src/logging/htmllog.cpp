/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2014                                                               *
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

#include "ghoul/logging/htmllog.h"

#include <cassert>

namespace ghoul {
namespace logging {

HTMLLog::HTMLLog(std::string filename, bool writeToAppend, bool timeStamping,
				bool dateStamping, bool categoryStamping, bool logLevelStamping)
    : TextLog(std::move(filename), writeToAppend, timeStamping, dateStamping,
				categoryStamping, logLevelStamping)
{
    if (hasValidFile()) {
        std::string output = \
            "<html>\n\
            \t<head>\n\
            \t\t<title>Log File</title>\n\
            \t</head>\n\
            \t<body>\n\
            \n\
            \t<table cellpadding=3 cellspacing=0 border=1>\n\
            \t\t<CAPTION>Log File</CAPTION>\n\
            \n\
            \t\t<THEAD>\n\
            \t\t\t<TR>\n";
        if (isDateStamping())
            output += "\t\t\t\t<th>Date</th>\n";
        if (isTimeStamping())
            output += "\t\t\t\t<th>Time</th>\n";
        if (isCategoryStamping())
            output += "\t\t\t\t<th>Category</th>\n";
        if (isLogLevelStamping())
            output += "\t\t\t\t<th>Level</th>\n";
        output += "\t\t\t\t<th>Message</th>\n\
                  \t\t\t</tr>\n\
                  \t\t<tbody>\n";
        writeLine(output);
    }
}

HTMLLog::~HTMLLog() {
    if (hasValidFile()) {
        const std::string output = \
            "\t\t</tbody>\n\
            \t</table>\n\
            \t</body>\n\
            </html>";
        writeLine(output);
    }
}


void HTMLLog::log(LogManager::LogLevel level, const std::string& category,
                  const std::string& message)
{
    std::string output = "\t\t\t<tr bgcolor=\"" + colorForLevel(level) + "\">\n";
    if (isDateStamping())
        output += "\t\t\t\t<td>" + getDateString() + "</td>\n";
    if (isTimeStamping())
        output += "\t\t\t\t<td>" + getTimeString() + "</td>\n";
    if (isCategoryStamping())
        output += "\t\t\t\t<td>" + category + "</td>\n";
    if (isLogLevelStamping())
        output += "\t\t\t\t<td>" + LogManager::stringFromLevel(level) + "</td>\n";
    output += "\t\t\t\t<td>" + message + "</td>\n\t\t\t</tr>\n";

    writeLine(output);
}

std::string HTMLLog::colorForLevel(LogManager::LogLevel level) {
    switch (level) {
        case LogManager::LogLevel::Debug:
            return "#00CC00";
        case LogManager::LogLevel::Info:
            return "#FFFFFF";
        case LogManager::LogLevel::Warning:
            return "#FFFF00";
        case LogManager::LogLevel::Error:
            return "#FF0000";
        case LogManager::LogLevel::Fatal:
            return "#00FFFF";
        case LogManager::LogLevel::NoLogging:
            return "#FFFFFF";
    }
    assert(false);
    return "#FF00FF";
}

} // namespace logging
} // namespace ghoul
