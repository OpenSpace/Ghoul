/**************************************************************************************************
 * GHOUL                                                                                          *
 * General Helpful Open Utility Library                                                           *
 *                                                                                                *
 * Copyright (c) 2012 Alexander Bock                                                              *
 *                                                                                                *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software  *
 * and associated documentation files (the "Software"), to deal in the Software without           *
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,     *
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the  *
 * Software is furnished to do so, subject to the following conditions:                           *
 *                                                                                                *
 * The above copyright notice and this permission notice shall be included in all copies or       *
 * substantial portions of the Software.                                                          *
 *                                                                                                *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING  *
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND     *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 *************************************************************************************************/

#include "logging/logmanager.h"
#include "logging/log.h"
#include <cassert>
#include <algorithm>
#include <vector>

namespace ghoul {
namespace logging {

LogManager* LogManager::_manager = nullptr;

/**
 * \intern
 * The LogManager_P is the private version of the LogManager and uses template instanciation to
 * increase the performance of the logging. Specifying the LogManager::LogLevel as a template
 * argument allows the compiler to optimize the check if a passed message should be relayed or not.
 * The same argument holds for the IMMEDIATEFLUSH that is used to control if the Log::flush()
 * methods should be called after each log(LogManager::LogLevel, std::string, std::string) call to
 * the Logs. After template argument insertion, the appropriate checks will be class-constant and
 * can therefore be optimized.
 * The same technique allows the ignoring of the Debug messages in a Release build.
 * The sole cost of this is a virtual function call in the log message.
 * \tparam LEVEL Determines the lowest level of LogMessages this LogManager accepts
 * \tparam IMMEDIATEFLUSH Controls if the Log::flush() message should be called after each log
 * message.
 */
template <LogManager::LogLevel LEVEL, bool IMMEDIATEFLUSH>
class LogManager_P : public LogManager {
public:
    void logMessage(LogLevel level, const std::string& category, const std::string& message);
};

template<LogManager::LogLevel LEVEL, bool IMMEDIATEFLUSH>
void LogManager_P<LEVEL,IMMEDIATEFLUSH>::logMessage(LogManager::LogLevel level, const std::string& category, const std::string& message) {
    if (level >= LEVEL) {
        std::vector<Log*>::const_iterator it = _logs.begin();
        for (; it != _logs.end(); ++it) {
            (*it)->log(level, category, message);
            if (IMMEDIATEFLUSH)
                (*it)->flush();
        }
    }
}

#if !defined _DEBUG
// ignore logging of Debug messages in Debug mode
template<>
void LogManager_P<LogManager::LogLevelDebug,true>::logMessage(LogManager::LogLevel, const std::string&, const std::string&) {}

template<>
void LogManager_P<LogManager::LogLevelDebug,false>::logMessage(LogManager::LogLevel, const std::string&, const std::string&) {}
#endif

std::string LogManager::stringFromLevel(LogLevel level) {
    switch (level) {
        case LogLevelDebug:
            return "Debug";
        case LogLevelInfo:
            return "Info";
        case LogLevelWarning:
            return "Warning";
        case LogLevelError:
            return "Error";
        case LogLevelFatal:
            return "Fatal";
    }
    assert(false);
    return "";
}

void LogManager::initialize(LogManager::LogLevel level, bool immediateFlush) {
    assert(_manager == nullptr);
    switch (level) {
        case LogLevelDebug:
            if (immediateFlush)
                _manager = new LogManager_P<LogLevelDebug, true>();
            else
                _manager = new LogManager_P<LogLevelDebug, false>();
            break;
        case LogLevelInfo:
            if (immediateFlush)
                _manager = new LogManager_P<LogLevelInfo, true>();
            else
                _manager = new LogManager_P<LogLevelInfo, false>();
            break;
        case LogLevelWarning:
            if (immediateFlush)
                _manager = new LogManager_P<LogLevelWarning, true>();
            else
                _manager = new LogManager_P<LogLevelWarning, false>();
            break;
        case LogLevelError:
            if (immediateFlush)
                _manager = new LogManager_P<LogLevelError, true>();
            else
                _manager = new LogManager_P<LogLevelError, false>();
            break;
        case LogLevelFatal:
            if (immediateFlush)
                _manager = new LogManager_P<LogLevelFatal, true>();
            else
                _manager = new LogManager_P<LogLevelFatal, false>();
            break;
    }
    assert(_manager != 0);
}

void LogManager::deinitialize() {
    delete _manager;
    _manager = nullptr;
}

LogManager::~LogManager() {
    std::vector<Log*>::iterator it = _logs.begin();
    for (; it != _logs.end(); ++it) {
        delete (*it);
    }
    _logs.clear();
}

LogManager& LogManager::ref() {
    assert(_manager != nullptr);
    return *_manager;
}

bool LogManager::isInitialized() {
    return (_manager != nullptr);
}

void LogManager::addLog(Log* log) {
    std::vector<Log*>::const_iterator it = std::find(_logs.begin(), _logs.end(), log);
    if (it == _logs.end())
        _logs.push_back(log);
}

void LogManager::removeLog(Log* log) {
    std::vector<Log*>::iterator it = std::find(_logs.begin(), _logs.end(), log);
    if (it != _logs.end())
        _logs.erase(it);
}

void LogManager::logMessage(LogManager::LogLevel level, const std::string& message) {
    _manager->logMessage(level, "", message);
}

} // namespace logging
} // namespace ghoul
