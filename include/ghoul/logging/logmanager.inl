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

/// Logs the 'message' with the 'category' at a level of LogManager::LogLevel::Debug
#define LDEBUGC(__category__, __message__) \
    do { \
        std::ostringstream tmp; tmp << __message__; \
        LogMgr.logMessage( \
            ghoul::logging::LogManager::LogLevel::Debug, __category__, tmp.str()); \
    } while (false)

/// Logs the 'message' with the 'category' at a level of LogManager::LogLevel::Info
#define LINFOC(__category__, __message__) \
    do { \
        std::ostringstream tmp; tmp << __message__; \
        LogMgr.logMessage( \
            ghoul::logging::LogManager::LogLevel::Info, __category__, tmp.str()); \
    } while (false)

/// Logs the 'message' with the 'category' at a level of LogManager::LogLevel::Warning
#define LWARNINGC(__category__, __message__) \
    do { \
        std::ostringstream tmp; tmp << __message__; \
        LogMgr.logMessage( \
            ghoul::logging::LogManager::LogLevel::Warning, __category__, tmp.str()); \
    } while (false)

/// Logs the 'message' with the 'category' at a level of LogManager::LogLevelError
#define LERRORC(__category__, __message__) \
    do { \
        std::ostringstream tmp; tmp << __message__; \
        LogMgr.logMessage( \
            ghoul::logging::LogManager::LogLevel::Error, __category__, tmp.str()); \
    } while (false)

/// Logs the 'message' with the 'category' at a level of LogManager::LogLevelFatal
#define LFATALC(__category__, __message__) \
    do { \
        std::ostringstream tmp; tmp << __message__; \
        LogMgr.logMessage( \
            ghoul::logging::LogManager::LogLevel::Fatal, __category__, tmp.str()); \
    } while (false)

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

/**
 * Logs the 'message' with the 'category' at a level of LogManager::LogLevel::Debug.
 * This method will not trigger an assertion when the LogManager has not been initialized
 * abut will print the message to the cout instead. If it has been initialized, it behaves
 * like the unsafe version.
 */
#define LDEBUGC_SAFE(__category__, __message__) \
    do { \
        std::ostringstream tmp; tmp << __message__; \
        if (ghoul::logging::LogManager::isInitialized()) \
            LogMgr.logMessage( \
                ghoul::logging::LogManager::LogLevel::Debug, __category__, tmp.str()); \
        else \
            std::cout << __category__ << ": " << tmp.str() << std::endl; \
    } while (false)

/**
 * Logs the 'message' with the 'category' at a level of LogManager::LogLevel::Info.
 * This method will not trigger an assertion when the LogManager has not been initialized
 * but will print the message to the cout instead. If it has been initialized, it behaves
 * like the unsafe version.
 */
#define LINFOC_SAFE(__category__, __message__) \
    do { \
        std::ostringstream tmp; tmp << __message__; \
        if (ghoul::logging::LogManager::isInitialized()) \
            LogMgr.logMessage( \
                ghoul::logging::LogManager::LogLevel::Info, __category__, tmp.str()); \
        else \
            std::cout << __category__ << ": " << tmp.str() << std::endl; \
    } while (false)

/**
 * Logs the 'message' with the 'category' at a level of LogManager::LogLevel::Warning.
 * This method will not trigger an assertion when the LogManager has not been initialized
 * but will print the message to the cout instead. If it has been initialized, it behaves
 * like the unsafe version.
 */
#define LWARNINGC_SAFE(__category__, __message__) \
    do { \
        std::ostringstream tmp; tmp << __message__; \
        if (ghoul::logging::LogManager::isInitialized()) \
            LogMgr.logMessage( \
                ghoul::logging::LogManager::LogLevel::Warning, __category__, tmp.str()); \
        else \
            std::cout << __category__ << ": " << tmp.str() << std::endl; \
    } while (false)

/**
 * Logs the 'message' with the 'category' at a level of LogManager::LogLevel::Error.
 * This method will not trigger an assertion when the LogManager has not been initialized
 * but will print the message to the cout instead. If it has been initialized, it behaves
 * like the unsafe version.
 */
#define LERRORC_SAFE(__category__, __message__) \
    do { \
        std::ostringstream tmp; tmp << __message__; \
        if (ghoul::logging::LogManager::isInitialized()) \
            LogMgr.logMessage( \
                ghoul::logging::LogManager::LogLevel::Error, __category__, tmp.str()); \
        else \
            std::cout << __category__ << ": " << tmp.str() << std::endl; \
    } while (false)

/**
 * Logs the 'message' with the 'category' at a level of LogManager::LogLevel::Fatal.
 * This method will not trigger an assertion when the LogManager has not been initialized
 * but will print the message to the cout instead. If it has been initialized, it behaves
 * like the unsafe version.
 */
#define LFATALC_SAFE(__category__, __message__) \
    do { \
        std::ostringstream tmp; tmp << __message__; \
        if (ghoul::logging::LogManager::isInitialized()) \
            LogMgr.logMessage( \
                ghoul::logging::LogManager::LogLevel::Fatal, __category__, tmp.str()); \
        else \
            std::cout << __category__ << ": " << tmp.str() << std::endl; \
    } while (false)

/**
 * Logs the 'message' with a level of LogManager::LogLevel::Debug. A variable called
 * _loggerCat needs to be defined and should contain the category. This method will not
 * trigger an assertion when the LogManager has not been initialized but will print the
 * message to the cout instead. If it has been initialized, it behaves like the unsafe
 * version.
 */
#define LDEBUG_SAFE(__message__) LDEBUGC_SAFE(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogManager::LogLevel::Info. A variable called
 * _loggerCat needs to be defined and should contain the category. This method will not
 * trigger an assertion when the LogManager has not been initialized but will print the
 * message to the cout instead. If it has been initialized, it behaves like the unsafe
 * version.
 */
#define LINFO_SAFE(__message__) LINFOC_SAFE(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogManager::LogLevel::Warning. A variable called
 * _loggerCat needs to be defined and should contain the category. This method will not
 * trigger an assertion when the LogManager has not been initialized but will print the
 * message to the cout instead. If it has been initialized, it behaves like the unsafe
 * version.
 */
#define LWARNING_SAFE(__message__) LWARNINGC_SAFE(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogManager::LogLevel::Error. A variable called
 * _loggerCat needs to be defined and should contain the category. This method will not
 * trigger an assertion when the LogManager has not been initialized but will print the
 * message to the cout instead. If it has been initialized, it behaves like the unsafe
 * version.
 */
#define LERROR_SAFE(__message__) LERRORC_SAFE(_loggerCat, __message__)

/**
 * Logs the 'message' with a level of LogManager::LogLevel::Fatal. A variable called
 * _loggerCat needs to be defined and should contain the category. This method will not
 * trigger an assertion when the LogManager has not been initialized but will print the
 * message to the cout instead. If it has been initialized, it behaves like the unsafe
 * version.
 */
#define LFATAL_SAFE(__message__) LFATALC_SAFE(_loggerCat, __message__)
