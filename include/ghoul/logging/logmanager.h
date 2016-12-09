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

#ifndef __GHOUL___LOGMANAGER___H__
#define __GHOUL___LOGMANAGER___H__

#include <ghoul/designpattern/singleton.h>

#include <ghoul/misc/boolean.h>

#include <array>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace ghoul {
namespace logging {

class Log;

/**
 * The central singleton class that is responsible for handling Log%s and logging methods.
 * This singleton class provides methods to add new Log%s, remove Log%s, and relay
 * messages to all Log%s added to the LogManager. A log message consists of a
 * LogManager::LogLevel, a category and a message. The category is mainly used as a prefix
 * and/or grouping within the log files and may have other meanings depending on the
 * specific Log%s. The LogManager instance has to be initialized with the #initialize
 * method and can be accessed using the #ref method afterwards. Initializing an instance
 * twice or accessing an uninitialized LogManager will result in an assertion. The logging
 * is performed thread-safe.
 *
 * The different LogManager::LogLevel available are: LogManager::LogLevel::Debug,
 * LogManager::LogLevel::Info, LogManager::LogLevel::Warning, LogManager::LogLevel::Error,
 * LogManager::LogLevel::Fatal.
 * If a LogManager was created with a LogManager::LogLevel x, all messages with
 * LogManager::LogLevel y <= x will be passed to Log handler.
 *
 * Macros are defined to make logging messages easier. These macros are: #LDEBUG,
 * #LDEBUGC, #LINFO, #LINFOC, #LWARNING, #LWARNINGC, #LERROR, #LERRORC, #LFATAL, #LFATALC.
 * The *C versions of the macros requires the category and the message as a parameter. The
 * versions without the C require an <code>std::string</code> variable named
 * <code>_loggerCat</code> to be defined in the scope of the macro "call".
 */
class LogManager : public Singleton<LogManager> {
public:
    using ImmediateFlush = ghoul::Boolean;

    /**
     * Enumerates all available LogLevel for the LogManager. The LogLevels are guaranteed
     * to be strictly ordered from least important to important.
     */
    enum class LogLevel {
        /**
         * Used for Debug output; will never be used in release
         */
        Debug = 0,
        /**
         * Used for informational messages which can be ignored, but might be
         * informative
         */
        Info = 1,
        /**
         * Warnings which do not represent a problem in itself, but may hint to a wrong
         * configuration
         */
        Warning = 2,
        /**
         * Errors which will pose problems, but do not necessarily require the immediate
         * end of the application
         */
        Error = 3,
        /**
         * Error which will be so severe that the application cannot recover from them
         */
        Fatal = 4,
        /**
         * Used as a placeholder to inhibit all LogMessages
         */
        NoLogging = 5
    };

    /**
     * Creates and initializes an empty LogManager with the passed LogManager::LogLevel.
     * \param level The lowest LogManager::LogLevel that will be passed to the containing
     * Log%s.
     * \param immediateFlush Determines if all Log%s will be flushed out immediately
     * after a message was received. In the case of file-backed logs, the files will be
     * written out to disk and in case of a console log, the console will be updated.
     * Passing <code>true</code> will slow down the execution but guarantees that a crash
     * immediately after a log message won't lead to data loss.
     */
    LogManager(LogLevel level = LogLevel::Info,
        ImmediateFlush immediateFlush = ImmediateFlush::No);

    /**
     * The main method to log messages. If the <code>level</code> is >= the level this
     * LogManager was created with, the <code>message</code> will be passed to the stored
     * Log%s. The <code>category</code> will be used in different ways depending on the
     * Log in question, but examples are grouping or prepending to the message.
     * \param level The level of the message that should be passed to the Log%s
     * \param category The category of the message, which will be used depending on
     * the Log%s
     * \param message The message that will be passed to the Log%s. May contain
     * control sequences.
     */
    void logMessage(LogManager::LogLevel level, const std::string& category,
        const std::string& message);

    /**
     * The main method to log messages. If the <code>level</code> is >= the level this
     * LogManager was created with, the <code>message</code> will be passed to the stored
     * Log%s. The <code>category</code> of the message will be an empty string, which
     * causes it to be ignored by most Log%s.
     * \param level The level of the message that should be passed to the Log%s
     * \param message The message that will be passed to the Log%s. May contain
     * control sequences.
     */
    void logMessage(LogManager::LogLevel level, const std::string& message);

    /**
     * Returns the message counter status for the passed LogLevel \p level.
     * \param level The LogLevel for which the counter is returned
     * \return The number of messages that have been logged for the passed \p level since
     * creation of the LogManager or the last call to resetCounter
     */
    int messageCounter(LogManager::LogLevel level);

    /**
     * Resets the internal log message counting back to 0 for all log levels.
     */
    void resetMessageCounters();

    /**
     * Adds the passed log to the list of managed Log%s. The ownership of the Log is
     * passed to the LogManager. The Log will be deleted when the LogManager is
     * #deinitialize%d. Adding the same Log twice has no effect and is permitted.
     * \param log The Log that should be added to this LogManager
     */
    void addLog(std::shared_ptr<Log> log);

    /**
     * Removes the passed log from the list of managed Log%s. This transfers the ownership
     * of the Log back to the caller and he is responsible for deleting the Log. Trying to
     * remove a Log that is not part of this LogManager has no effect and is permitted.
     * \param log The Log that should be removed from this LogManager
     */
    void removeLog(std::shared_ptr<Log> log);

    /**
     * Flushes all of the registered Log%s. This can be useful in cases when an
     * unscheduled shutdown is imminent, but all messages must be written first. Will call
     * the Log::flush method on all logs.
     */
    void flushLogs();

    /**
     * Returns the string representation of the passed LogManager::LogLevel. The name of
     * each level is equal to its enum value.
     * \return The string representation of the passed LogManager::LogLevel
     */
    static std::string stringFromLevel(LogLevel level);

    /**
     * Returns the LogManager::LogLevel for the passed string representation. The name of
     * each level is equal to its enum value.
     * \return The the LogManager::LogLevel for the passed string representation
     */
    static LogLevel levelFromString(const std::string& level);

private:
    /// The mutex that is protecting the #logMessage calls
    std::mutex _mutex;
    
    /// The LogLevel
    LogManager::LogLevel _level;
    
    /// Whether all logs should be flushed immediately
    bool _immediateFlush;
    
    /// Stores the Logs which are managed by this LogManager
    std::vector<std::shared_ptr<Log>> _logs;

    /// Stores the number of messages for each log level (5)
    std::array<int, 5> _logCounter;
};

} // namespace logging
} // namespace ghoul

#define LogMgr (ghoul::logging::LogManager::ref())

#include "logmanager.inl"

#endif // __GHOUL___LOGMANAGER___H__
