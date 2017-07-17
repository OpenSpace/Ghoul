/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2017                                                               *
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

#include <iostream>
#include <sstream>

namespace ghoul::logging {
    
LogLevel LogManager::logLevel() const {
    return _level;
}

} // namespace ghoul::logging

/**
 * @defgroup LOGGING_MACRO_GROUP Logging Macros
 *
 * @{
 */

/// Logs the 'message' with the 'category' at a level of 'logLevel'
#define LOGC(__loglevel__, __category__, __message__) \
    do { \
        if (ghoul::logging::LogManager::isInitialized()) { \
            if (__loglevel__ >= LogMgr.logLevel()) { \
                std::ostringstream __tmp__; \
                __tmp__ << __message__; \
                LogMgr.logMessage( \
                    (__loglevel__), \
                    (__category__), \
                    __tmp__.str() \
                ); \
            } \
        } else { \
            std::ostringstream __tmp__; \
            __tmp__ << __message__; \
            std::cout << (__category__) << " (" << \
            ghoul::logging::stringFromLevel(__loglevel__) << \
            ") : " << __tmp__.str() << std::endl; \
        } \
    } while (false)


#ifdef GHOUL_LOGGING_ENABLE_TRACE
/// Logs the 'message' with the 'category' at a level of LogLevel::Trace
#define LTRACEC(__category__, __message__) \
LOGC(ghoul::logging::LogLevel::Trace, __category__, __message__)
#else
// If we compile without trace, we remove all trace code
#define LTRACEC(__category, __message)
#endif

/// Logs the 'message' with the 'category' at a level of LogLevel::Debug
#define LDEBUGC(__category__, __message__) \
    LOGC(ghoul::logging::LogLevel::Debug, __category__, __message__)

/// Logs the 'message' with the 'category' at a level of LogLevel::Info
#define LINFOC(__category__, __message__) \
    LOGC(ghoul::logging::LogLevel::Info, __category__, __message__)

/// Logs the 'message' with the 'category' at a level of LogLevel::Warning
#define LWARNINGC(__category__, __message__) \
    LOGC(ghoul::logging::LogLevel::Warning, __category__, __message__)

/// Logs the 'message' with the 'category' at a level of LogLevelError
#define LERRORC(__category__, __message__) \
    LOGC(ghoul::logging::LogLevel::Error, __category__, __message__)

/// Logs the 'message' with the 'category' at a level of LogLevelFatal
#define LFATALC(__category__, __message__) \
    LOGC(ghoul::logging::LogLevel::Fatal, __category__, __message__)

#define LOG(__loglevel__, __message__) LOGC(__loglevel__, _loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogLevel::Trace. A variable called
 * _loggerCat needs to be defined and should contain the category.
 */
#define LTRACE(__message__) LTRACEC(_loggerCat, __message__)
                
/**
 * Logs the 'message' with a level of LogLevel::Debug. A variable called
 * _loggerCat needs to be defined and should contain the category.
 */
#define LDEBUG(__message__) LDEBUGC(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogLevel::Info. A variable called
 * _loggerCat needs to be defined and should contain the category.
 */
#define LINFO(__message__) LINFOC(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogLevel::Info. A variable called
 * _loggerCat needs to be defined and should contain the category.
 */
#define LINFO(__message__) LINFOC(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogLevel::Warning. A variable called
 * _loggerCat needs to be defined and should contain the category.
 */
#define LWARNING(__message__) LWARNINGC(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogLevel::Error. A variable called
 * _loggerCat needs to be defined and should contain the category.
 */
#define LERROR(__message__) LERRORC(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogLevel::Fatal. A variable called
 * _loggerCat needs to be defined and should contain the category.
 */
#define LFATAL(__message__) LFATALC(_loggerCat, __message__)

/** @} */
