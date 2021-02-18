/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2021                                                               *
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

#include <ghoul/logging/log.h>
#include <ghoul/misc/assert.h>
#include <algorithm>
#include <map>
#include <vector>

namespace ghoul::logging {

LogManager* LogManager::_instance = nullptr;

LogManager::LogManager(LogLevel level, ImmediateFlush immediateFlush)
    : _level(level)
    , _immediateFlush(immediateFlush)
{}

void LogManager::initialize(LogLevel level, ImmediateFlush immediateFlush) {
    if (_instance) {
        _instance->_level = level;
        _instance->_immediateFlush = immediateFlush;
    }
    _instance = new LogManager(level, immediateFlush);
}

void LogManager::deinitialize() {
    ghoul_assert(isInitialized(), "LogManager is not initialized");

    _instance->_logs.clear();
}

bool LogManager::isInitialized() {
    return _instance != nullptr;
}

LogManager& LogManager::ref() {
    ghoul_assert(isInitialized(), "LogManager is not initialized");
    return *_instance;
}

void LogManager::addLog(std::unique_ptr<Log> log) {
    _logs.push_back(std::move(log));
}

void LogManager::removeLog(Log* log) {
    const auto it = std::find_if(
        _logs.cbegin(),
        _logs.cend(),
        [log](const std::unique_ptr<Log>& l) { return l.get() == log; }
    );
    if (it != _logs.end()) {
        _logs.erase(it);
    }
}

void LogManager::flushLogs() {
    for (const std::unique_ptr<Log>& log : _logs) {
        log->flush();
    }
}

void LogManager::logMessage(LogLevel level, std::string_view category,
                            std::string_view message)
{
    if (!ghoul::logging::LogManager::isInitialized()) {
        _consoleLog.log(level, category, message);
        return;
    }

    if (level >= _level) {
        std::lock_guard lock(_mutex);

        _consoleLog.log(level, category, message);
        if (_immediateFlush) {
            _consoleLog.flush();
        }
        for (const std::unique_ptr<Log>& log : _logs) {
            if (level >= log->logLevel()) {
                log->log(level, category, message);
                if (_immediateFlush) {
                    log->flush();
                }
            }
        }

        const int l = std::underlying_type<LogLevel>::type(level);
        ++(_logCounters[l]);
    }
}

void LogManager::logMessage(LogLevel level, std::string_view message) {
    logMessage(level, "", message);
}

LogLevel LogManager::logLevel() const {
    return _level;
}

int LogManager::messageCounter(LogLevel level) {
    return _logCounters[std::underlying_type<LogLevel>::type(level)];
}

void LogManager::resetMessageCounters() {
    std::fill(_logCounters.begin(), _logCounters.end(), 0);
}

} // namespace ghoul::logging
