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

#include "logging/consolelog.h"
#include <iostream>

#ifdef WIN32
#include <windows.h>
#endif

namespace {
	const std::string keyColorOutput = "ColorOutput";
}

namespace ghoul {
namespace logging {

ConsoleLog::ConsoleLog(bool colorOutput, bool timeStamping, bool dateStamping,
                       bool categoryStamping, bool logLevelStamping)
    : StreamLog(std::cout, timeStamping, dateStamping, categoryStamping, logLevelStamping)
    , _colorOutput(colorOutput)
{
}

ConsoleLog::ConsoleLog(const Dictionary& dictionary)
	: StreamLog(std::cout, dictionary)
{
	if (dictionary.hasKeyAndValue<bool>(keyColorOutput))
		dictionary.getValue(keyColorOutput, _colorOutput);
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
    WORD colorIndex = 0;
    switch (level) {
     case LogManager::LogLevel::Debug:
            colorIndex = FOREGROUND_GREEN;
            break;
        case LogManager::LogLevel::Info:
            colorIndex = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN;
            break;
        case LogManager::LogLevel::Warning:
            colorIndex = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            break;
        case LogManager::LogLevel::Error:
            colorIndex = FOREGROUND_RED | FOREGROUND_INTENSITY;
            break;
        case LogManager::LogLevel::Fatal:
            colorIndex = FOREGROUND_RED | FOREGROUND_BLUE| FOREGROUND_INTENSITY;
            break;
		default:
			colorIndex = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN;
			break;
    }
    SetConsoleTextAttribute(hConsole, colorIndex);
#elif __unix__
    switch (level) {
        case LogManager::LogLevel::Debug:
            _stream << "\033[22;32m";   // green
            break;
        case LogManager::LogLevel::Info:
            _stream << "\033[0m";       // white
            break;
        case LogManager::LogLevel::Warning:
            _stream << "\033[01;40;33m";// yellow on black
            break;
        case LogManager::LogLevel::Error:
            _stream << "\033[22;31m";   // red
            break;
        case LogManager::LogLevel::Fatal:
            _stream << "\033[22;34m";   // blue
            break;
		default:
			_stream << "\033[0m";       // white
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
