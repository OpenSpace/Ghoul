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

#ifndef __LOG_H__
#define __LOG_H__

#include "logmanager.h"
#include <string>

namespace ghoul {
namespace logging {

/**
 * Abstract base class for all Log%s that can be added to a LogManager. Base classes must
 * implement the #log and #flush methods. The log message will only be called with
 * LogManager::LogLevel levels which were filtered by the LogManager the Log belongs to.
 * After finishing the #flush method, all previously written log messages should be
 * stored/printed/transmitted even if the program crashes.
 * All subclasses are usable without a LogManager as well by directly instantiating them.
 *
 * \see TextLog \see ConsoleLog \see StreamLog
 */
class Log {
public:
    /// Virtual destructor
    virtual ~Log();

    /**
     * Method that logs a message with a given level and category. The method of logging
     * is dependent on the explicit subclass.
     * \param level The log level with which the message shall be logged
     * \param category The category of this message. Can be used by each subclass
     * individually
     * \param message The message body of the log message
     */
    virtual void log(LogManager::LogLevel level, const std::string& category,
                     const std::string& message) = 0;

    /**
     * Flushes the Log. This has different effects on different subclasses, but after this
     * method finishes, the logs will be safe against a program crash.
     */
    virtual void flush();

protected:
    /**
     * Base constructor, which initializes the passed parameters.
     * \param timeStamping Determines if the log should print the time when a message
     * is logged in the log messages
     * \param dateStamping Determines if the log should print the time when a message
     * is logged in the log messages
     * \param categoryStamping Determines if the log should print the categories in
     * the log messages
     * \param logLevelStamping Determines if the log should print the log level in
     * the log messages
     */
    Log(bool timeStamping = true, bool dateStamping = true, bool categoryStamping = true,
        bool logLevelStamping = true);

    /// Is the log printing the logging time?
    bool isTimeStamping() const;

    /// Set the log printing of the time
    void setTimeStamping(bool timeStamping);

    /// Is the log printing the logging date?
    bool isDateStamping() const;

    /// Set the log printing of the date
    void setDateStamping(bool dateStamping);

    /// Is the log printing the category?
    bool isCategoryStamping() const;

    /// Set the log printing of the category
    void setCategoryStamping(bool categoryStamping);

    /// Is the log printing the log level?
    bool isLogLevelStamping() const;

    /// Set the log printing of the log level
    void setLogLevelStamping(bool logLevelStamping);

    /**
     * Returns the current time as a string. The format for the time is "HH:MM:SS" and the
     * clock is 24h.
     * \return The current time as a string
     */
    std::string getTimeString() const;

    /**
     * Returns the current date as a string. The date format is "YYYY-MM-DD".
     * \return The current date as a string
     */
    std::string getDateString() const;

protected:
    // Not defined on purpose. Using should throw linker error.
    Log& operator=(const Log&);
    Log(const Log&);
    
private:
    bool _timeStamping; ///< Is the log printing the time?
    bool _dateStamping; ///< Is the log printing the date?
    bool _categoryStamping; ///< Is the log printing the category?
    bool _logLevelStamping; ///< Is the log printing the log level?
};

} // namespace logging
} // namespace ghoul

#endif // __LOG_H__
