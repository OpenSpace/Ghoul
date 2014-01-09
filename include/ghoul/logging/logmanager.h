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

#ifndef __LOGMANAGER_H__
#define __LOGMANAGER_H__

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace ghoul {
namespace logging {

class Log;

/**
 * The central singleton class that is responsible for handling Logs and logging methods.
 * This singleton class provides methods to add new Logs, remove Logs, and relay messages
 * to all Logs added to the LogManager. A log message consists of a LogManager::LogLevel,
 * a category and a message. The category is mainly used as a prefix and/or grouping
 * within the log files and may have other meanings depending on Logs.
 * The LogManager instance has to be initialized with the LogManager::init(LogLevel, bool)
 * method and can be accessed using the LogManager::ref() method afterwards. Initializing
 * an instance twice or accessing an uninitialized LogManager will result in an assertion.
 * The logging is performed thread-safe.
 *
 * The different LogManager::LogLevel available are: Debug, Info, Warning, Error, Fatal.
 * If a LogManager was created with a LogLevel x, all messages with LogLevel y <= x will
 * be passed to Log handler.
 *
 * Macros are defined to make logging messages easier. These macros are: LDEBUG, LDEBUGC,
 * LINFO, LINFOC, LWARNING, LWARNINGC, LERROR, LERRORC, LFATAL, LFATALC. The *C versions
 * of the macros requires the category and the message as a parameter. The versions
 * without the C require a variable named _loggerCat to be defined in the scope of the
 * macro "call"
 */
class LogManager {
public:
    virtual ~LogManager();
    
    /**
     * Enumerates all available LogLevel for the LogManager. The LogLevels are guaranteed
     * to be strictly ordered from least important to important.
     */
    enum class LogLevel {
        Debug = 1 << 0, ///< Used for Debug output; will never be used in release
        Info = 1 << 1,
        Warning = 1 << 2,
        Error = 1 << 3,
        Fatal = 1 << 4
    };

    /**
     * Creates and initializes an empty LogManager with the passed level.
     * \param [in] level The lowest LogLevel that will be passed to the containing Logs.
     * A LogLevel of Debug will never be passed along in a Release build.
     * \param [in] immediateFlush Determines if all Logs will be flushed out immediately
     * after a message was received. In the case of file-based logs, the files will be
     * written out to disk and in case of a console log, the console will be updated.
     * Passing <code>true</code> will slow down the execution but guarantees that a crash
     * immediately after a log message won't lead to data loss.
     * Calling init when the LogManager is already initialized will trigger an assertion.
     */
    static void initialize(LogLevel level = LogLevel::Info, bool immediateFlush = false);

    /**
     * Deinitializes and deletes the LogManager. All the stored Logs in this LogManager
     * will be deleted as well. After deinit() returns, the LogManager can be initialized
     * again with different values. Calling deinit() on an uninitialized LogManager is
     * allowed.
     */
    static void deinitialize();

    /**
     * Returns the reference to the singleton LogManager. Triggers an assertion if the
     * LogManager has not been initialized yet.
     * \return A reference to an singleton LogManager
     */
    static LogManager& ref();

    /**
     * Returns the initialization state of the LogManager.
     * \return The initialization state of the LogManager
     */
    static bool isInitialized();

    /**
     * The main method to log messages. If the <code>level</code> is >= the level this
     * LogManager was created with, the message will be passed to the stored Logs. The
     * category will be used in different ways depending on the Log in question, but
     * examples are grouping or prepending to the message.
     * \param [in] level The level of the message that should be passed to the Logs
     * \param [in] category The category of the message, which will be used depending on
     * the Logs
     * \param [in] message The message that will be passed to the Logs. May contain
     * control sequences.
     */
    virtual void logMessage(LogManager::LogLevel level, const std::string& category,
                            const std::string& message) = 0;

    /**
     * The main method to log messages. If the <code>level</code> is >= the level this
     * LogManager was created with, the message will be passed to the stored \ref Logs.
     * The category of the message will be an empty string, which causes it to be ignored
     * by most Logs.
     * \param [in] level The level of the message that should be passed to the Logs
     * \param [in] message The message that will be passed to the Logs. May contain
     * control sequences.
     */
    void logMessage(LogManager::LogLevel level, const std::string& message);

    /**
     * Adds the passed log to the list of managed Logs. The ownership of the Log is passed
     * to the LogManager. The Log will be deleted when the LogManager is deinitialized.
     * Adding the same Log twice has no effect and is permitted.
     * \param [in] log The log that should be added to this LogManager
     */
    void addLog(Log* log);

    /**
     * Removes the passed log from the list of managed Logs. This transfers the ownership
     * of the Log to the caller and he is responsible for deleting the Log. Trying to
     * remove a Log that is not part of this LogManager has no effect and is permitted.
     * \param [in] log The Log that should be removed from this LogManager
     */
    void removeLog(Log* log);

    /**
     * Returns the string representation of the passed LogLevel. The name of each LogLevel
     * is equal to its enum value.
     * \return The string representation of the passed LogLevel.
     */
    static std::string stringFromLevel(LogLevel level);

protected:
    std::vector<Log*> _logs;  ///< stores the Logs which are managed by this LogManager

private:
    static LogManager* _manager; ///< singleton member
};

} // namespace logging
} // namespace ghoul

#define LogMgr (ghoul::logging::LogManager::ref())

#include "logmanager.inl"

#endif // __LOGMANAGER_H__
