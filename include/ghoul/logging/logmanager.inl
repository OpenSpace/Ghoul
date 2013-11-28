/**************************************************************************************************
 * GHOUL                                                                                          *
 * General Helpful Open Utility Library                                                           *
 *                                                                                                *
 * Copyright (c) 2012 Alexander Bock                                                              *
 *                                                                                                *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software  *
 * and associated documentation files (the "Software"), to deal in the Software without           *
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,     *
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the  *
 * Software is furnished to do so, subject to the following conditions:                           *
 *                                                                                                *
 * The above copyright notice and this permission notice shall be included in all copies or       *
 * substantial portions of the Software.                                                          *
 *                                                                                                *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING  *
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND     *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 *************************************************************************************************/

namespace ghoul {
namespace logging {

/// Logs the 'message' with the category 'category' with a level of LogManager::LogLevelDebug
#define LDEBUGC(__category__, __message__) \
    do { \
        std::ostringstream tmp; \
        tmp << __message__; \
        LogMgr.logMessage(ghoul::logging::LogManager::LogLevelDebug, __category__, tmp.str()); \
    } while (false)

/// Logs the 'message' with the category 'category' with a level of LogManager::LogLevelInfo
#define LINFOC(__category__, __message__) \
    do { \
        std::ostringstream tmp; \
        tmp << __message__; \
        LogMgr.logMessage(ghoul::logging::LogManager::LogLevelInfo, __category__, tmp.str()); \
    } while (false)

/// Logs the 'message' with the category 'category' with a level of LogManager::LogLevelWarning
#define LWARNINGC(__category__, __message__) \
    do { \
        std::ostringstream tmp; \
        tmp << __message__; \
        LogMgr.logMessage(ghoul::logging::LogManager::LogLevelWarning, __category__, tmp.str()); \
    } while (false)

/// Logs the 'message' with the category 'category' with a level of LogManager::LogLevelError
#define LERRORC(__category__, __message__) \
    do { \
        std::ostringstream tmp; \
        tmp << __message__; \
        LogMgr.logMessage(ghoul::logging::LogManager::LogLevelError, __category__, tmp.str()); \
    } while (false)

/// Logs the 'message' with the category 'category' with a level of LogManager::LogLevelFatal
#define LFATALC(__category__, __message__) \
    do { \
        std::ostringstream tmp; \
        tmp << __message__; \
        LogMgr.logMessage(ghoul::logging::LogManager::LogLevelFatal, __category__, tmp.str()); \
    } while (false)

/**
 * Logs the 'message' with a level of LogManager::LogLevelDebug. A variable called _loggerCat needs
 * to be defined and should contain the category.
 */
#define LDEBUG(__message__) LDEBUGC(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogManager::LogLevelInfo. A variable called _loggerCat needs
 * to be defined and should contain the category.
 */
#define LINFO(__message__) LINFOC(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogManager::LogLevelInfo. A variable called _loggerCat needs
 * to be defined and should contain the category.
 */
#define LINFO(__message__) LINFOC(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogManager::LogLevelWarning. A variable called _loggerCat
 * needs to be defined and should contain the category.
 */
#define LWARNING(__message__) LWARNINGC(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogManager::LogLevelError. A variable called _loggerCat needs
 * to be defined and should contain the category.
 */
#define LERROR(__message__) LERRORC(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogManager::LogLevelFatal. A variable called _loggerCat needs
 * to be defined and should contain the category.
 */
#define LFATAL(__message__) LFATALC(_loggerCat, __message__)

/**
 * Logs the 'message' with the category 'category' with a level of LogManager::LogLevelDebug.
 * This method will not trigger an assertion when the LogManager has not been initialized and will
 * discard the message in that case. If it has been initialized, it behaves like the unsafe
 * version.
 */
#define LDEBUGC_SAFE(__category__, __message__) \
    do { \
        std::ostringstream tmp; \
        tmp << __message__; \
        if (ghoul::logging::LogManager::isInitialized()) \
            LogMgr.logMessage(ghoul::logging::LogManager::LogLevelDebug, __category__, tmp.str()); \
    } while (false)

/**
 * Logs the 'message' with the category 'category' with a level of LogManager::LogLevelInfo.
 * This method will not trigger an assertion when the LogManager has not been initialized and will
 * discard the message in that case. If it has been initialized, it behaves like the unsafe
 * version.
 */
#define LINFOC_SAFE(__category__, __message__) \
    do { \
        std::ostringstream tmp; \
        tmp << __message__; \
        if (ghoul::logging::LogManager::isInitialized()) \
            LogMgr.logMessage(ghoul::logging::LogManager::LogLevelInfo, __category__, tmp.str()); \
    } while (false)

/**
 * Logs the 'message' with the category 'category' with a level of LogManager::LogLevelWarning.
 * This method will not trigger an assertion when the LogManager has not been initialized but will
 * print the message to the cout instead. If it has been initialized, it behaves like the unsafe
 * version.
 */
#define LWARNINGC_SAFE(__category__, __message__) \
    do { \
        std::ostringstream tmp; \
        tmp << __message__; \
        if (ghoul::logging::LogManager::isInitialized()) \
            LogMgr.logMessage(ghoul::logging::LogManager::LogLevelWarning, __category__, \
                                tmp.str()); \
        else \
            std::cout << __category__ << ": " << tmp.str() << std::endl; \
    } while (false)

/**
 * Logs the 'message' with the category 'category' with a level of LogManager::LogLevelError.
 * This method will not trigger an assertion when the LogManager has not been initialized but will
 * print the message to the cout instead. If it has been initialized, it behaves like the unsafe
 * version.
 */
#define LERRORC_SAFE(__category__, __message__) \
    do { \
        std::ostringstream tmp; \
        tmp << __message__; \
        if (ghoul::logging::LogManager::isInitialized()) \
            LogMgr.logMessage(ghoul::logging::LogManager::LogLevelError, __category__, tmp.str()); \
        else \
            std::cout << __category__ << ": " << tmp.str() << std::endl; \
    } while (false)

/**
 * Logs the 'message' with the category 'category' with a level of LogManager::LogLevelFatal.
 * This method will not trigger an assertion when the LogManager has not been initialized but will
 * print the message to the cout instead. If it has been initialized, it behaves like the unsafe
 * version.
 */
#define LFATALC_SAFE(__category__, __message__) \
    do { \
        std::ostringstream tmp; \
        tmp << __message__; \
        if (ghoul::logging::LogManager::isInitialized()) \
            LogMgr.logMessage(ghoul::logging::LogManager::LogLevelFatal, __category__, tmp.str()); \
        else \
            std::cout << __category__ << ": " << tmp.str() << std::endl; \
    } while (false)

/**
 * Logs the 'message' with a level of LogManager::LogLevelDebug. A variable called _loggerCat needs
 * to be defined and should contain the category. This method will not trigger an assertion when
 * the LogManager has not been initialized but will print the message to the cout instead. If it
 * has been initialized, it behaves like the unsafe version.
 */
#define LDEBUG_SAFE(__message__) LDEBUGC_SAFE(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogManager::LogLevelInfo. A variable called _loggerCat needs
 * to be defined and should contain the category. This method will not trigger an assertion when
 * the LogManager has not been initialized but will print the message to the cout instead. If it
 * has been initialized, it behaves like the unsafe version.
 */
#define LINFO_SAFE(__message__) LINFOC_SAFE(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogManager::LogLevelInfo. A variable called _loggerCat needs
 * to be defined and should contain the category. This method will not trigger an assertion when
 * the LogManager has not been initialized but will print the message to the cout instead. If it
 * has been initialized, it behaves like the unsafe version.
 */
#define LINFO_SAFE(__message__) LINFOC_SAFE(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogManager::LogLevelWarning. A variable called _loggerCat
 * needs to be defined and should contain the category. This method will not trigger an assertion
 * when the LogManager has not been initialized but will print the message to the cout instead. If
 * it has been initialized, it behaves like the unsafe version.
 */
#define LWARNING_SAFE(__message__) LWARNINGC_SAFE(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogManager::LogLevelError. A variable called _loggerCat needs
 * to be defined and should contain the category. This method will not trigger an assertion when
 * the LogManager has not been initialized but will print the message to the cout instead. If it
 * has been initialized, it behaves like the unsafe version.
 */
#define LERROR_SAFE(__message__) LERRORC_SAFE(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogManager::LogLevelFatal. A variable called _loggerCat needs
 * to be defined and should contain the category. This method will not trigger an assertion when
 * the LogManager has not been initialized but will print the message to the cout instead. If it
 * has been initialized, it behaves like the unsafe version.
 */
#define LFATAL_SAFE(__message__) LFATALC_SAFE(_loggerCat, __message__)

} // namespace logging
} // namespace ghoul
