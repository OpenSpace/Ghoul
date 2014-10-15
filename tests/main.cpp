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

#include "gtest/gtest.h"

#include <ghoul/cmdparser/cmdparser>
#include <ghoul/filesystem/filesystem>
#include <ghoul/logging/logging>

#include "tests/test_common.inl"
#include "tests/test_luatodictionary.inl"
#include "tests/test_commandlineparser.inl"
#include "tests/test_dictionary.inl"
#include "tests/test_filesystem.inl"

using namespace ghoul::cmdparser;
using namespace ghoul::filesystem;
using namespace ghoul::logging;

#ifndef GHOUL_ROOT_DIR
#define GHOUL_ROOT_DIR "../../../ext/ghoul"
#endif

int main(int argc, char** argv) {
    LogManager::initialize(LogManager::LogLevel::None);
    LogMgr.addLog(new ConsoleLog);

    FileSystem::initialize();
    
    const std::string root = GHOUL_ROOT_DIR;
    const std::string testdir = root + "/tests";
    const std::string scriptdir = root + "/scripts";

    const bool extDir = FileSys.directoryExists(testdir);
    if (extDir) {
        FileSys.registerPathToken("${SCRIPTS_DIR}", scriptdir);
        FileSys.registerPathToken("${TEST_DIR}", testdir);
    }
    else {
        LFATALC("main", "Fix me");
        return 0;
    }

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
