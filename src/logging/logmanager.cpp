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

#include <ghoul/logging/logmanager.h>

#include "ghoul/logging/log.h"

#include <algorithm>
#include <map>
#include <vector>

namespace ghoul {
namespace logging {

std::string LogManager::stringFromLevel(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return "Debug";
        case LogLevel::Info:
            return "Info";
        case LogLevel::Warning:
            return "Warning";
        case LogLevel::Error:
            return "Error";
        case LogLevel::Fatal:
            return "Fatal";
        case LogLevel::NoLogging:
            return "None";
    }
    ghoul_assert(false, "Missing case label");
}

LogManager::LogLevel LogManager::levelFromString(const std::string& level) {
    static const std::map<std::string, LogLevel> levels = {
        { "Debug"  , LogLevel::Debug },
        { "Info"   , LogLevel::Info },
        { "Warning", LogLevel::Warning },
        { "Error"  , LogLevel::Error },
        { "Fatal"  , LogLevel::Fatal },
        { "None"   , LogLevel::NoLogging }
    };

    auto it = levels.find(level);
    ghoul_assert(it != levels.end(), "Missing entry in 'levels' map");
    return it->second;
}

LogManager::LogManager(LogManager::LogLevel level, ImmediateFlush immediateFlush)
    : _level(level)
    , _immediateFlush(immediateFlush)
    , _logCounter({0, 0, 0, 0, 0})
{}

void LogManager::addLog(std::shared_ptr<Log> log) {
    auto it = std::find(_logs.begin(), _logs.end(), log);
    if (it == _logs.end()) {
        _logs.push_back(std::move(log));
    }
}

void LogManager::removeLog(std::shared_ptr<Log> log) {
    auto it = std::find(_logs.begin(), _logs.end(), log);
    if (it != _logs.end()) {
        _logs.erase(it);
    }
}

void LogManager::logMessage(LogManager::LogLevel level, const std::string& category,
                                                               const std::string& message)
{
    if (level >= _level) {
        // Acquire lock, automatically released at end of scope
        std::lock_guard<std::mutex> lock(_mutex);

        for (const auto& log : _logs) {
            log->log(level, category, message);
            if (_immediateFlush) {
                log->flush();
            }
        }

        int l = std::underlying_type<LogLevel>::type(level);
        ++(_logCounter[l]);
    }
}

void LogManager::logMessage(LogManager::LogLevel level, const std::string& message) {
    logMessage(level, "", message);
}

void LogManager::flushLogs() {
    for (const auto& log : _logs) {
        log->flush();
    }
}


int LogManager::messageCounter(LogManager::LogLevel level) {
    return _logCounter[std::underlying_type<LogLevel>::type(level)];
}

void LogManager::resetMessageCounters() {
    _logCounter = { 0, 0, 0, 0, 0 };
}

} // namespace logging
} // namespace ghoul
