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

#include <ghoul/logging/htmllog.h>

#include <ghoul/misc/assert.h>
#include <iterator>

namespace ghoul::logging {

HTMLLog::HTMLLog(const std::string& filename, Append writeToAppend,
                 TimeStamping timeStamping, DateStamping dateStamping,
                 CategoryStamping categoryStamping, LogLevelStamping logLevelStamping,
                 const std::vector<std::string>& cssIncludes,
                 const std::vector<std::string>& jsIncludes, LogLevel minimumLogLevel)
    : TextLog(
        std::move(filename),
        writeToAppend,
        timeStamping,
        dateStamping,
        categoryStamping,
        logLevelStamping,
        minimumLogLevel
    )
{
    _useCustomStyling = cssIncludes.size() > 1 || jsIncludes.size() > 1;

    std::string output = \
        "<html>\n\
        \t<head>\n\
        \t\t<title>Log File</title>\
        \t\t<style>\n";

    std::back_insert_iterator<std::string> backInserter(output);

    for (const std::string& c : cssIncludes) {
        std::ifstream cssInput(c);
        std::copy(
            std::istreambuf_iterator<char>{cssInput},
            std::istreambuf_iterator<char>(),
            backInserter
        );
    }

    output += \
        "\t\t</style>\n\
        \t\t<script>\n\
        " ;

    for (const std::string& j : jsIncludes) {
        std::ifstream jsInput(j);
        std::copy(
            std::istreambuf_iterator<char>{jsInput},
            std::istreambuf_iterator<char>(),
            backInserter
        );
    }

    output += \
        "\t\t</script>\n\
        \t</head>\n\
        \t<body>\n\
        \n\
        \t<table>\n\
        \n\
        \t\t<thead>\n\
        \t\t\t<tr>\n";
    if (isDateStamping()) {
        output += "\t\t\t\t<th class=\"log-date\">Date</th>\n";
    }
    if (isTimeStamping()) {
        output += "\t\t\t\t<th class=\"log-time\">Time</th>\n";
    }
    if (isCategoryStamping()) {
        output += "\t\t\t\t<th class=\"log-category\">Category</th>\n";
    }
    if (isLogLevelStamping()) {
        output += "\t\t\t\t<th class=\"log-level\">Level</th>\n";
    }
    output += "\t\t\t\t<th class=\"log-message\">Message</th>\n\
              \t\t\t</tr>\n\
              \t\t<tbody>\n";
    writeLine(std::move(output));
}

HTMLLog::~HTMLLog() {
    writeLine("\t\t</tbody>\n\
              \t</table>\n\
              \t</body>\n\
              </html>"
    );
}

void HTMLLog::log(LogLevel level, const std::string& category,
                  const std::string& message)
{
    std::string output;
    if (_useCustomStyling) {
        output = "\t\t\t<tr class=\"" + classForLevel(level) + "\">\n";
    } else {
        output = "\t\t\t<tr bgcolor=\"" + colorForLevel(level) + "\">\n";
    }
    if (isDateStamping()) {
        output += "\t\t\t\t<td class=\"log-date\">" + dateString() + "</td>\n";
    }
    if (isTimeStamping()) {
        output += "\t\t\t\t<td class=\"log-time\">" + timeString() + "</td>\n";
    }
    if (isCategoryStamping()) {
        output += "\t\t\t\t<td class=\"log-category\">" + category + "</td>\n";
    }
    if (isLogLevelStamping()) {
        output += "\t\t\t\t<td class=\"log-level\">" +
            stringFromLevel(level) + "</td>\n";
    }

    output += "\t\t\t\t<td class=\"log-message\">";
    for (char c : message) {
        switch (c) {
            case '<':
                output += "&lt;";
                break;
            case '>':
                output += "&gt;";
                break;
            case '&':
                output += "&amp;";
                break;
            case '\n':
                output += "<br>";
                break;
            default:
                output += c;
                break;
        }
    }

    output += "</td>\n\t\t\t</tr>\n";
    writeLine(std::move(output));
}

std::string HTMLLog::classForLevel(LogLevel level) {
    switch (level) {
        case LogLevel::Trace:     return "log-level-trace";
        case LogLevel::Debug:     return "log-level-debug";
        case LogLevel::Info:      return "log-level-info";
        case LogLevel::Warning:   return "log-level-warning";
        case LogLevel::Error:     return "log-level-error";
        case LogLevel::Fatal:     return "log-level-fatal";
        case LogLevel::NoLogging: return "log-level-no-logging";
        default:                  throw MissingCaseException();
    }
}

std::string HTMLLog::colorForLevel(LogLevel level) {
    switch (level) {
        case LogLevel::Trace:     return "#999999";
        case LogLevel::Debug:     return "#00CC00";
        case LogLevel::Info:      return "#FFFFFF";
        case LogLevel::Warning:   return "#FFFF00";
        case LogLevel::Error:     return "#FF0000";
        case LogLevel::Fatal:     return "#00FFFF";
        case LogLevel::NoLogging: return "#FFFFFF";
        default:                  throw MissingCaseException();
    }
}

} // namespace ghoul::logging
