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

#include <ghoul/logging/visualstudiooutputlog.h>

#ifdef WIN32
#include <Windows.h>
#endif

namespace ghoul::logging {

VisualStudioOutputLog::VisualStudioOutputLog(TimeStamping timeStamping,
                                             DateStamping dateStamping,
                                             CategoryStamping categoryStamping,
                                             LogLevelStamping logLevelStamping,
                                             LogLevel minimumLogLevel)
    : Log(timeStamping, dateStamping, categoryStamping, logLevelStamping, minimumLogLevel)
{}

#ifdef WIN32
void VisualStudioOutputLog::log(LogLevel level, const std::string& category,
                                const std::string& message)
{
    std::string fullMessage = createFullMessageString(level, category, message) + '\n';
    OutputDebugString(fullMessage.c_str());
}
#else
void VisualStudioOutputLog::log(LogLevel, const std::string&, const std::string&) {}
#endif

} // namespace ghoul::logging
