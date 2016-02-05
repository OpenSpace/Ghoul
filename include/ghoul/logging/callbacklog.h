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

#ifndef __CALLBACKLOG_H__
#define __CALLBACKLOG_H__

#include <ghoul/logging/log.h>

#include <functional>

namespace ghoul {
namespace logging {

/**
 * A concrete subclass of Log that passes logs to the provided callback function. The
 * callback is specified using <code>std::function</code>. Trying to log messages when the
 * callback object has been deleted results in undefined behavior. The formatting of the
 * log messages depends on the stamping settings. The different possibilities are:
 * \verbatim
[DATE | TIME] CATEGORY (LEVEL) MESSAGE
[DATE] CATEGORY (LEVEL) MESSAGE
[TIME] CATEGORY (LEVEL) MESSAGE
 \endverbatim
 * And the remaining possibilities with <code>CATEGORY</code> and <code>LEVEL</code>
 * missing.
 */
class CallbackLog : public Log {
public:
    /// The type of function that is used as a callback in this log
    typedef std::function<void (std::string)> CallbackFunction;

    /**
     * Constructor that calls the Log constructor and initializes this CallbackLog.
     * \param callbackFunction The callback function that is called for each log
     * message.
     * \param timeStamping Determines if the log should print the time when a message
     * is logged
     * \param dateStamping Determines if the log should print the time when a message
     * is logged
     * \param categoryStamping Determines if the log should print the categories
     * \param logLevelStamping Determines if the log should print the log level
     */
    CallbackLog(CallbackFunction callbackFunction, bool timeStamping = true,
        bool dateStamping = true, bool categoryStamping = true,
        bool logLevelStamping = true);

    /**
     * Method that logs a message with a given level and category to the console.
     * \param level The log level with which the message shall be logged
     * \param category The category of this message. Can be used by each subclass
     * individually
     * \param message The message body of the log message
     */
    void log(LogManager::LogLevel level, const std::string& category,
        const std::string& message) override;

    /**
     * Replaces the old callback with this <code>callbackFunction</code>. This function
     * is not checked and it is the caller`s responsiblity to assure that the function
     * object is callable.
     * \param callbackFunction The new callback function that will be called henceforth
     */
    void setCallback(CallbackFunction callbackFunction);

    /**
     * Returns the callback function that is used in this CallbackLog.
     * \return The callback function that is used in this CallbackLog
     */
    const CallbackFunction& callback() const;

protected:
    CallbackFunction _callbackFunction;
};

} // namespace logging
} // namespace ghoul

#endif // __CALLBACKLOG_H__
