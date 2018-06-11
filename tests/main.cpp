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

#ifdef __unix__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion-null"
#endif // __unix__

// When running the unit tests we don't want to be asked what to do in the case of an
// assertion

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4619) // #pragma warning: there is no warning number '4800' 
#endif // _MSC_VER

#include <ghoul/misc/supportmacros.h>

#include "gtest/gtest.h"

#ifdef _MSC_VER
#pragma warning (pop)
#endif // _MSC_VER

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/logging/consolelog.h>
#include <ghoul/logging/logmanager.h>

namespace constants {
    static std::string TestDirectory;
    static std::string ScriptDirectory;
} // namespace constants

#include "tests/test_buffer.inl"
#include "tests/test_commandlineparser.inl"
#include "tests/test_common.inl"
#include "tests/test_crc32.inl"
#include "tests/test_dictionary.inl"
#include "tests/test_dictionaryjsonformatter.inl"
#include "tests/test_dictionaryluaformatter.inl"
#include "tests/test_filesystem.inl"
#include "tests/test_luatodictionary.inl"
#include "tests/test_templatefactory.inl"
#include "tests/test_csvreader.inl"

using namespace ghoul::filesystem;
using namespace ghoul::logging;

#ifndef GHOUL_ROOT_DIR
#define GHOUL_ROOT_DIR "../../../ext/ghoul"
#endif

// #define PRINT_OUTPUT

int main(int argc, char** argv) {
    LogManager::initialize(LogLevel::Fatal);
    LogMgr.addLog(std::make_unique<ConsoleLog>());

    FileSystem::initialize();
    
    const std::string root = absPath(GHOUL_ROOT_DIR);
    constants::TestDirectory = root + "/tests";
    constants::ScriptDirectory = root + "/scripts";

    if (!FileSys.directoryExists(constants::TestDirectory)) {
        LFATALC("main", "Fix me");
        return 0;
    }

#ifdef PRINT_OUTPUT
    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();
#endif

    testing::InitGoogleTest(&argc, argv);
    bool b = RUN_ALL_TESTS();

#ifdef PRINT_OUTPUT
    std::string output = testing::internal::GetCapturedStdout();
    std::string error = testing::internal::GetCapturedStderr();

    std::ofstream o("output.txt");
    o << output;

    std::ofstream e("error.txt");
    e << error;
#endif

    return b;
}

#ifdef __unix__
#pragma GCC diagnostic pop
#endif // __unix__
