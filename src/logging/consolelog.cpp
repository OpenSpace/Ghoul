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

#include <ghoul/logging/consolelog.h>
#include <iostream>

#ifdef WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#include <assert.h>
#include <unistd.h>
#include <sys/sysctl.h>

// Function is taken from https://developer.apple.com/library/mac/qa/qa1361/_index.html
// Returns true if the current process is being debugged (either running under the
// debugger or has a debugger attached post facto).
static bool runningInDebugger() {
    int junk;
    int mib[4];
    struct kinfo_proc info;
    size_t size;
    
    // Initialize the flags so that, if sysctl fails for some bizarre
    // reason, we get a predictable result.
    info.kp_proc.p_flag = 0;
    
    // Initialize mib, which tells sysctl the info we want, in this case
    // we're looking for information about a specific process ID.
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();
    
    // Call sysctl.
    size = sizeof(info);
    junk = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);
    assert(junk == 0);
    
    // We're being debugged if the P_TRACED flag is set.
    return ( (info.kp_proc.p_flag & P_TRACED) != 0 );
}
#endif

namespace ghoul {
namespace logging {

ConsoleLog::ConsoleLog(ColorOutput colorOutput, TimeStamping timeStamping,
            DateStamping dateStamping, CategoryStamping categoryStamping,
            LogLevelStamping logLevelStamping)
    : StreamLog(std::cout, timeStamping, dateStamping, categoryStamping, logLevelStamping)
    , _colorOutput(colorOutput == ColorOutput::Yes)
{
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
        case LogManager::LogLevel::NoLogging:
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
            _stream << "\033[22;35m";   // blue
            break;
        case LogManager::LogLevel::NoLogging:
			_stream << "\033[0m";       // white
			break;
    }
#elif __APPLE__
    // The Xcode debugger is a bit strange with those control commands, so we have to
    // disable them if where are being debugged
    bool inDebugger = runningInDebugger();
    if (!inDebugger) {
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
                _stream << "\033[22;35m";   // blue
                break;
            case LogManager::LogLevel::NoLogging:
                _stream << "\033[0m";       // white
                break;
        }
    }
#endif
}

void ConsoleLog::resetColor() {
    setColorForLevel(LogManager::LogLevel::Info);
}

} // namespace logging
} // namespace ghoul
