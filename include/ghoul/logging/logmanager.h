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
class LogManager {
public:
    virtual ~LogManager();
    
    /**
     * Enumerates all available LogLevel for the LogManager. The LogLevels are guaranteed
     * to be strictly ordered from least important to important.
     */
    enum class LogLevel {
        /**
         * Used for Debug output; will never be used in release
         */
        Debug = 1 << 0,
        /**
         * Used for informational messages which can be ignored, but might be
         * informative
         */
        Info = 1 << 1,
        /**
         * Warnings which do not represent a problem in itself, but may hint to a wrong
         * configuration
         */
        Warning = 1 << 2,
        /**
         * Errors which will pose problems, but do not necessarily require the immediate
         * end of the application
         */
        Error = 1 << 3,
        /**
         * Error which will be so severe that the application cannot recover from them
         */
        Fatal = 1 << 4
    };

    /**
     * Creates and initializes an empty LogManager with the passed LogManager::LogLevel.
     * \param level The lowest LogManager::LogLevel that will be passed to the containing
     * Log%s. A LogManager::LogLevel of LogManager::LogLevel::Debug will never be passed
     * along in a Release build.
     * \param immediateFlush Determines if all Log%s will be flushed out immediately
     * after a message was received. In the case of file-backed logs, the files will be
     * written out to disk and in case of a console log, the console will be updated.
     * Passing <code>true</code> will slow down the execution but guarantees that a crash
     * immediately after a log message won't lead to data loss.
     * Calling #initialize when the LogManager is already initialized will trigger an
     * assertion.
     */
    static void initialize(LogLevel level = LogLevel::Info, bool immediateFlush = false);

    /**
     * Deinitializes and deletes the LogManager. All the stored Logs in this LogManager
     * will be deleted as well. After this method returns, the LogManager can be
     * initialized again with different values. Calling this with an uninitialized
     * LogManager will trigger an assertion.
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
     * LogManager was created with, the <code>message</code> will be passed to the stored
     * Log%s. The <code>category</code> will be used in different ways depending on the
     * Log in question, but examples are grouping or prepending to the message.
     * \param level The level of the message that should be passed to the Log%s
     * \param category The category of the message, which will be used depending on
     * the Log%s
     * \param message The message that will be passed to the Log%s. May contain
     * control sequences.
     */
    virtual void logMessage(LogManager::LogLevel level, const std::string& category,
                            const std::string& message) = 0;

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
     * Adds the passed log to the list of managed Log%s. The ownership of the Log is
     * passed to the LogManager. The Log will be deleted when the LogManager is
     * #deinitialize%d. Adding the same Log twice has no effect and is permitted.
     * \param log The Log that should be added to this LogManager
     */
    void addLog(Log* log);

    /**
     * Removes the passed log from the list of managed Log%s. This transfers the ownership
     * of the Log back to the caller and he is responsible for deleting the Log. Trying to
     * remove a Log that is not part of this LogManager has no effect and is permitted.
     * \param log The Log that should be removed from this LogManager
     */
    void removeLog(Log* log);

    /**
     * Returns the string representation of the passed LogManager::LogLevel. The name of
     * each level is equal to its enum value.
     * \return The string representation of the passed LogManager::LogLevel
     */
    static std::string stringFromLevel(LogLevel level);

protected:
    std::vector<Log*> _logs;  ///< Stores the Logs which are managed by this LogManager

private:
    static LogManager* _manager; ///< Singleton member
};

} // namespace logging
} // namespace ghoul

#define LogMgr (ghoul::logging::LogManager::ref())

#include "logmanager.inl"

#endif // __LOGMANAGER_H__
