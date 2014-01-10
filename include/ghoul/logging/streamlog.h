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

#ifndef __STREAMLOG_H__
#define __STREAMLOG_H__

#include "log.h"
#include <ostream>

namespace ghoul {
namespace logging {

/**
 * A concrete subclass of Log that logs the messages to a given std::ostream. The
 * formatting of the log messages depends on the stamping settings. The various
 * possibilities are:
 * "[DATE | TIME] CATEGORY (LEVEL) MESSAGE"
 * "[DATE] CATEGORY (LEVEL) MESSAGE"
 * "[TIME] CATEGORY (LEVEL) MESSAGE"
 * And the remaining possibilities with CATEGORY and (LEVEL) missing.
 * Ownership of the stream is not transferred.
 */
class StreamLog : public Log {
public:
    /**
     * Constructor that calls Log::Log(bool, bool, bool, bool) constructor.
     * \param stream The initialized stream this Log should use
     * \param timeStamping Determines if the log should print the time when a message
     * is logged in the log messages
     * \param dateStamping Determines if the log should print the time when a message
     * is logged in the log messages
     * \param categoryStamping Determines if the log should print the categories in
     * the log messages
     * \param logLevelStamping Determines if the log should print the log level in
     * the log messages
     */
    StreamLog(std::ostream& stream, bool timeStamping = false, bool dateStamping = false,
              bool categoryStamping = true, bool logLevelStamping = true);

    /**
     * Method that logs a message with a given level and category to the stream.
     * \param level The log level with which the message shall be logged
     * \param category The category of this message. Can be used by each subclass
     * individually
     * \param message The message body of the log message
     */
    virtual void log(LogManager::LogLevel level, const std::string& category,
                     const std::string& message);

    /// Flushes the stream and, thereby, all messages that are in the associated buffer
    void flush();

protected:
    // Not implemented on purpose; use should generate a linker error
    StreamLog(const StreamLog& rhs);
    StreamLog& operator=(const StreamLog& rhs);
    std::ostream& _stream; ///< The stream to which the log messages will be sent
};

} // namespace logging
} // namespace ghoul

#endif // __STREAMLOG_H__
