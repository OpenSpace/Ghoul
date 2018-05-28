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

#ifndef __GHOUL___HTMLLOG___H__
#define __GHOUL___HTMLLOG___H__

#include <ghoul/logging/textlog.h>

#include <vector>

namespace ghoul::logging {

/**
 * A subclass of TextLog that logs the messages to a structured HTML file on hard disk.
 * The log containing all components will contain a table with the following format:
 * \verbatim
--------------------------------------------
| DATE | TIME | CATEGORY | LEVEL | MESSAGE |
|      |      |          |       |         |
|      |      |          |       |         |
\endverbatim
 * If a specific value should not be stamped, the according table entry will be missing
 * from the HTML file. The file will be opened in the constructor and closed in the
 * destructor of this class. A HTMLLog is always created anew and cannot be appended to.
 * That means that the user of this class has to perform log file rotation.
 */
class HTMLLog : public TextLog {
public:
    /**
     * Constructor that calls TextLog constructor and opens the file that will log the
     * messages. If the file does not exist, it will be created.
     *
     * \param filename The path and filename of the file that will receive the log
     *        messages
     * \param writeToAppend If this is <code>true</code>, the log messages will be
     *        appended to the file. If it is <code>false</code> the file will be
     *        overwritten without a warning.
     * \param timeStamping Determines if the log should print the time when a message is
     *        logged in the log messages
     * \param dateStamping Determines if the log should print the time when a message is
     *        logged in the log messages
     * \param categoryStamping Determines if the log should print the categories in the
     *        log messages
     * \param logLevelStamping Determines if the log should print the log level in the log
     *        messages
     *
     * \pre \p filename must not be empty
     */
    HTMLLog(const std::string& filename, Append writeToAppend = Append::Yes,
        TimeStamping timeStamping = TimeStamping::Yes,
        DateStamping dateStamping = DateStamping::Yes,
        CategoryStamping categoryStamping = CategoryStamping::Yes,
        LogLevelStamping logLevelStamping = LogLevelStamping::Yes,
        const std::vector<std::string>& cssIncludes = std::vector<std::string>(),
        const std::vector<std::string>& jsIncludes = std::vector<std::string>(),
        LogLevel minimumLogLevel = LogLevel::AllLogging);

    /// Destructor that closes and finalizes the HTML file
    virtual ~HTMLLog();

    /**
     * Method that logs a message with a given <code>level</code> and
     * <code>category</code> to the text file.
     *
     * \param level The log level with which the message shall be logged
     * \param category The category of this message. Can be used by each subclass
     *        individually
     * \param message The message body of the log message
     */
    void log(LogLevel level, const std::string& category,
        const std::string& message) override;

protected:
    /**
     * Returns a css class string for the passed level
     * LogLevel::Trace -> log-level-trace<br>
     * LogLevel::Debug -> log-level-debug<br>
     * LogLevel::Info -> log-level-info<br>
     * LogLevel::Warning -> log-level-warning<br>
     * LogLevel::Error -> log-level-error<br>
     * LogLevel::Fatal -> log-level-fatal<br>
     */
    static std::string classForLevel(LogLevel level);

    /**
     * Returns a HTML color string for the passed color.
     * LogLevel::Trace -> Grey<br>
     * LogLevel::Debug -> Green<br>
     * LogLevel::Info -> Black<br>
     * LogLevel::Warning -> Yellow<br>
     * LogLevel::Error -> Red<br>
     * LogLevel::Fatal -> Cyan<br>
     */
    static std::string colorForLevel(LogLevel level);

private:
    bool _useCustomStyling;
};

} // namespace ghoul::logging

#endif // __GHOUL___HTMLLOG___H__
