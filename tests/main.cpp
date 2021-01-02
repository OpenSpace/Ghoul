/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2021                                                               *
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

#define CATCH_CONFIG_RUNNER
#include "catch2/catch.hpp"

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/logging/consolelog.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/supportmacros.h>

using namespace ghoul::filesystem;
using namespace ghoul::logging;

int main(int argc, char** argv) {
    LogManager::initialize(LogLevel::Fatal);
    LogMgr.addLog(std::make_unique<ConsoleLog>());

    FileSystem::initialize();

    const std::string Root = absPath(GHOUL_ROOT_DIR);
    const std::string TestDirectory = Root + "/tests";
    const std::string ScriptDirectory = Root + "/scripts";

    FileSys.registerPathToken("${UNIT_TEST}", TestDirectory);
    FileSys.registerPathToken("${UNIT_SCRIPT}", ScriptDirectory);

    if (!FileSys.directoryExists(TestDirectory)) {
        LFATALC("main", "Fix me");
        return 0;
    }

    int result = Catch::Session().run(argc, argv);
    return result;
}
