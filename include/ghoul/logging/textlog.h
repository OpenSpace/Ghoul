/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2018                                                               *
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

#ifndef __GHOUL___TEXTLOG___H__
#define __GHOUL___TEXTLOG___H__

#include <ghoul/logging/log.h>

#include <ghoul/misc/boolean.h>
#include <fstream>

namespace ghoul::logging {

/**
 * A concrete subclass of Log that logs the messages to a plain text file on hard disk.
 * The formatting of the log messages depends on the stamping settings. The different
 * possibilities are:
 * \verbatim
"[DATE | TIME] CATEGORY (LEVEL) MESSAGE"
"[DATE] CATEGORY (LEVEL) MESSAGE"
"[TIME] CATEGORY (LEVEL) MESSAGE"
\endverbatim
 * And the remaining possibilities with <code>CATEGORY</code> and <code>LEVEL</code>
 * missing. Only the #log method needs to be overwritten in a subclass, if a
 * different output format is required. The file will be opened in the constructor and
 * closed in the destructor of this class. A parameter in the constructor controls of the
 * file will be reset before writing the first time.
 */
class TextLog : public Log {
public:
    BooleanType(Append);

    /**
    * Constructor that calls Log constructor and opens the file that will log the
    * messages. If the file does not exist, it will be created. If the path to the file is
    * invalid, an <code>std::ios_base::failure</code> exception will be thrown.
    *
    * \param filename The path and filename of the file that will receive the log
    *        messages
    * \param writeToAppend If this is <code>true</code>, the log messages will be appended
    *        to the file. If it is <code>false</code> the file will be overwritten without
    *        a warning.
    * \param timeStamping Determines if the log should print the time when a message is
    *        logged in the log messages
    * \param dateStamping Determines if the log should print the time when a message is
    *        logged in the log messages
    * \param categoryStamping Determines if the log should print the categories in the
    *        log messages
    * \param logLevelStamping Determines if the log should print the log level in the log
    *        messages
    *
    * \throw std::ios_base::failure If the opening of the file failed
    * \pre \p filename must not be empty
    */
    TextLog(const std::string& filename, Append writeToAppend = Append::Yes,
        TimeStamping timeStamping = TimeStamping::Yes,
        DateStamping dateStamping = DateStamping::Yes,
        CategoryStamping categoryStamping = CategoryStamping::Yes,
        LogLevelStamping logLevelStamping = LogLevelStamping::Yes,
        LogLevel minimumLogLevel = LogLevel::AllLogging);

    /// Destructor closing and releasing the file handle
    ~TextLog();

    /**
     * Method that logs a <code>message</code> with a given <code>level</code> and
     * <code>category</code> to the text file.
     *
     * \param level The log level with which the message shall be logged
     * \param category The category of this message. Can be used by each subclass
     *        individually
     * \param message The message body of the log message
     */
    void log(LogLevel level, const std::string& category,
        const std::string& message) override;

    /// Flushes the text file and, thereby, all messages that are in the associated buffer
    void flush() override;

protected:
    /**
     * Writes the passed 'line' to the opened file. The line will be printed 'as-is' and
     * no endline or other control sequence will be added.
     *
     * \param line The line of text that should be printed to the file
     */
    void writeLine(std::string line);

    bool _printFooter; ///< Should a line be printed at the end after the file is closed?

private:
    std::fstream _file;
};

} // namespace ghoul::logging

#endif // __GHOUL___TEXTLOG___H__
