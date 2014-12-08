/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2014                                                               *
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

/**
 * @defgroup LOGGING_MACRO_GROUP Logging Macros
 *
 * @{
 */

/// Logs the 'message' with the 'category' at a level of 'logLevel'
#define LOGC(__loglevel__, __category__, __message__) \
    do { \
        std::ostringstream __tmp__; __tmp__ << __message__; \
        if (ghoul::logging::LogManager::isInitialized()) \
            LogMgr.logMessage( \
                (__loglevel__), \
                (__category__), \
                __tmp__.str() \
            ); \
        else { \
            std::cout << (__category__) << " (" << \
            ghoul::logging::LogManager::stringFromLevel(__loglevel__) << \
            ") : " << __tmp__.str() << std::endl; \
        } \
    } while (false)


/// Logs the 'message' with the 'category' at a level of LogManager::LogLevel::Debug
#define LDEBUGC(__category__, __message__) \
    LOGC(ghoul::logging::LogManager::LogLevel::Debug, __category__, __message__)

/// Logs the 'message' with the 'category' at a level of LogManager::LogLevel::Info
#define LINFOC(__category__, __message__) \
    LOGC(ghoul::logging::LogManager::LogLevel::Info, __category__, __message__)

/// Logs the 'message' with the 'category' at a level of LogManager::LogLevel::Warning
#define LWARNINGC(__category__, __message__) \
    LOGC(ghoul::logging::LogManager::LogLevel::Warning, __category__, __message__)

/// Logs the 'message' with the 'category' at a level of LogManager::LogLevelError
#define LERRORC(__category__, __message__) \
    LOGC(ghoul::logging::LogManager::LogLevel::Error, __category__, __message__)

/// Logs the 'message' with the 'category' at a level of LogManager::LogLevelFatal
#define LFATALC(__category__, __message__) \
    LOGC(ghoul::logging::LogManager::LogLevel::Fatal, __category__, __message__)

#define LOG(__loglevel__, __message__) LOGC(__loglevel__, _loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogManager::LogLevel::Debug. A variable called
 * _loggerCat needs to be defined and should contain the category.
 */
#define LDEBUG(__message__) LDEBUGC(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogManager::LogLevel::Info. A variable called
 * _loggerCat needs to be defined and should contain the category.
 */
#define LINFO(__message__) LINFOC(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogManager::LogLevel::Info. A variable called
 * _loggerCat needs to be defined and should contain the category.
 */
#define LINFO(__message__) LINFOC(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogManager::LogLevel::Warning. A variable called
 * _loggerCat needs to be defined and should contain the category.
 */
#define LWARNING(__message__) LWARNINGC(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogManager::LogLevel::Error. A variable called
 * _loggerCat needs to be defined and should contain the category.
 */
#define LERROR(__message__) LERRORC(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogManager::LogLevel::Fatal. A variable called
 * _loggerCat needs to be defined and should contain the category.
 */
#define LFATAL(__message__) LFATALC(_loggerCat, __message__)

/** @} */