/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012 Alexander Bock                                                     *
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

#include "logging/consolelog.h"
#include <iostream>

#ifdef WIN32
#include <windows.h>
#endif

namespace ghoul {
namespace logging {

#ifdef WIN32
namespace {
    WORD _originalConsoleColors; ///< storing the original colors to be able to reset them again
}
#endif

ConsoleLog::ConsoleLog(bool colorOutput, bool timeStamping, bool dateStamping,
                       bool categoryStamping, bool logLevelStamping)
    : StreamLog(std::cout, timeStamping, dateStamping, categoryStamping, logLevelStamping)
    , _colorOutput(colorOutput)
{
#ifdef WIN32
    // retrieve the current color scheme of the console and store it
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO* consoleInfo = new CONSOLE_SCREEN_BUFFER_INFO;
    GetConsoleScreenBufferInfo(hConsole, consoleInfo);
    _originalConsoleColors = consoleInfo->wAttributes;
    delete consoleInfo;
#endif
}

void ConsoleLog::log(LogManager::LogLevel level, const std::string& category,
                     const std::string& message)
{
    if (_colorOutput)
        setColorForLevel(level);

    StreamLog::log(level, category, message);

    if (_colorOutput)
        resetColor();
}

void ConsoleLog::setColorForLevel(LogManager::LogLevel level) {
#ifdef WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD colorIndex = _originalConsoleColors;
    switch (level) {
        case LogManager::LogLevelDebug:
            colorIndex = 10;        // green
            break;
        case LogManager::LogLevelInfo:
            colorIndex = _originalConsoleColors;        // default color scheme
            break;
        case LogManager::LogLevelWarning:
            colorIndex = 14;        // yellow on black
            break;
        case LogManager::LogLevelError:
            colorIndex = 12;        // red
            break;
        case LogManager::LogLevelFatal:
            colorIndex = 11;        // cyan
            break;
    }
    SetConsoleTextAttribute(hConsole, colorIndex);
#elif __unix__
    switch (level) {
        case LogManager::LogLevelDebug:
            _stream << "\033[22;32m";   // green
            break;
        case LogManager::LogLevelInfo:
            _stream << "\033[0m";       // white
            break;
        case LogManager::LogLevelWarning:
            _stream << "\033[01;40;33m";// yellow on black
            break;
        case LogManager::LogLevelError:
            _stream << "\033[22;31m";   // red
            break;
        case LogManager::LogLevelFatal:
            _stream << "\033[22;34m";   // blue
            break;
    }
#elif __APPLE__
#pragma unused(level)
#endif
}

void ConsoleLog::resetColor() {
    setColorForLevel(LogManager::LogLevel::Info);
}

} // namespace logging
} // namespace ghoul
