/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2024                                                               *
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

#include <ghoul/format.h>
#include <ghoul/misc/assert.h>
#include <iostream>
#include <string_view>

#ifdef WIN32
#include <Windows.h>
#endif // WIN32

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
    junk = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, nullptr, 0);
    ghoul_assert(junk == 0, "sysctl call failed");

    // We're being debugged if the P_TRACED flag is set.
    return ((info.kp_proc.p_flag & P_TRACED) != 0);
}
#endif // __APPLE__

namespace ghoul::logging {

ConsoleLog::ConsoleLog(ColorOutput colorOutput, LogLevel minimumLogLevel)
    : Log(
        Log::TimeStamping::No, Log::DateStamping::No, Log::CategoryStamping::Yes,
        Log::LogLevelStamping::Yes, minimumLogLevel
    )
    , _colorOutput(colorOutput)
{}

void ConsoleLog::log(LogLevel level, std::string_view category, std::string_view message)
{
    constexpr int CategoryLength = 20;
    constexpr char FillerCharacter = ' ';

    if (_colorOutput) {
        setColorForLevel(level);
    }

    // (W) Category          Message text
    //  ^  ^                 ^
    //  3  20                message.size()
    // + 2 for spaces in between

    const int totalLength = 5 + CategoryLength + static_cast<int>(message.size());

    std::string res;
    res.reserve(totalLength);
    switch (level) {
        case LogLevel::AllLogging: res += "(A) "; break;
        case LogLevel::Trace:      res += "(T) "; break;
        case LogLevel::Debug:      res += "(D) "; break;
        case LogLevel::Info:       res += "(I) "; break;
        case LogLevel::Warning:    res += "(W) "; break;
        case LogLevel::Error:      res += "(E) "; break;
        case LogLevel::Fatal:      res += "(F) "; break;
        case LogLevel::NoLogging:  res += "(-) "; break;
    }

    if (category.length() <= CategoryLength) {
        res += category;
        res += std::string(CategoryLength - category.length(), FillerCharacter);
        res += ' ';
    }
    else {
        // the message is longer than our 25 width space. We'd like to keep 4 characters
        // at the end for context

        // Onelongcategorystringthatneedstobeshortended ->
        // Onelongcatego...nded

        // slightlylongerstringhere ->
        // slightlylonge...here

        // shorterstillinthisline ->
        // shorterstillin..line

        const size_t nDots = std::min<size_t>(category.length() - CategoryLength, 2);
        // 20(length) - 4(remaining four characters at the end) - number of dots
        res += std::format(
            "{}{}{} ",
            category.substr(0, CategoryLength - 4 - nDots),
            std::string(nDots, '.'),
            category.substr(category.size() - 4)
        );
    }

    res += message;
    std::cout << res << '\n';


    if (_colorOutput) {
        resetColor();
    }
}

void ConsoleLog::flush() {
    std::cout.flush();
}

void ConsoleLog::setColorForLevel(LogLevel level) {
#ifdef WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    const WORD colorIndex = [](LogLevel level) -> WORD {
        switch (level) {
            case LogLevel::Trace:
                return FOREGROUND_INTENSITY;
            case LogLevel::Debug:
                return FOREGROUND_GREEN;
            case LogLevel::Info:
                return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN;
            case LogLevel::Warning:
                return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            case LogLevel::Error:
                return FOREGROUND_RED | FOREGROUND_INTENSITY;
            case LogLevel::Fatal:
                return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            case LogLevel::NoLogging:
                return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN;
            case LogLevel::AllLogging:
                return 0;
            default:
                throw MissingCaseException();
        }
    }(level);

    // Get the old color information
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo = { 0 };
    GetConsoleScreenBufferInfo(hConsole, &csbiInfo);

    // Or-ing the new foreground color with the old values for the background
    const WORD Background = BACKGROUND_BLUE | BACKGROUND_GREEN |
                            BACKGROUND_RED | BACKGROUND_INTENSITY;
    SetConsoleTextAttribute(hConsole, colorIndex | (csbiInfo.wAttributes & Background));
#elif defined __unix__
    switch (level) {
        case LogLevel::Trace:
            std::cout << "\033[0;37m";    // grey
            break;
        case LogLevel::Debug:
            std::cout << "\033[22;32m";   // green
            break;
        case LogLevel::Info:
            std::cout << "\033[0m";       // white
            break;
        case LogLevel::Warning:
            std::cout << "\033[01;40;33m";// yellow on black
            break;
        case LogLevel::Error:
            std::cout << "\033[22;31m";   // red
            break;
        case LogLevel::Fatal:
            std::cout << "\033[22;35m";   // blue
            break;
        case LogLevel::NoLogging:
            std::cout << "\033[0m";       // white
            break;
        case LogLevel::AllLogging:
            break;
    }
#elif __APPLE__
    // The Xcode debugger is a bit strange with those control commands, so we have to
    // disable them if where are being debugged
    bool inDebugger = runningInDebugger();
    if (inDebugger) {
        return;
    }

    switch (level) {
        case LogLevel::Trace:
            std::cout << "\033[0;37m";    // grey
            break;
        case LogLevel::Debug:
            std::cout << "\033[22;32m";   // green
            break;
        case LogLevel::Info:
            std::cout << "\033[0m";       // white
            break;
        case LogLevel::Warning:
            std::cout << "\033[01;40;33m";// yellow on black
            break;
        case LogLevel::Error:
            std::cout << "\033[22;31m";   // red
            break;
        case LogLevel::Fatal:
            std::cout << "\033[22;35m";   // blue
            break;
        case LogLevel::NoLogging:
            std::cout << "\033[0m";       // white
            break;
        case LogLevel::AllLogging:
            break;
    }
#endif
}

void ConsoleLog::resetColor() {
    setColorForLevel(LogLevel::Info);
}

} // namespace ghoul::logging
