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

#include "voreen/core/utils/cmdparser/command_loglevel.h"

#include "tgt/logmanager.h"

namespace voreen {

Command_LogLevel::Command_LogLevel(tgt::LogLevel* dbgLevel)
: Command("--loglevel", "-l", "Sets the verbosity of the tgt::Log", "<debug|warning|info|error|fatal>")
, dbgLevel_(dbgLevel)
{}

bool Command_LogLevel::execute(const std::vector<std::string>& parameters) {
    std::string argument = parameters[0];
    if (argument == "debug")
        *dbgLevel_ = tgt::Debug;
    else if (argument == "warning")
        *dbgLevel_ = tgt::Warning;
    else if (argument == "info")
        *dbgLevel_ = tgt::Info;
    else if (argument == "error")
        *dbgLevel_ = tgt::Error;
    else if (argument == "fatal")
        *dbgLevel_ = tgt::Fatal;
    else // This case should not happen
        return false;

    return true;
}

bool Command_LogLevel::checkParameters(const std::vector<std::string>& parameters) {
    std::set<std::string> set;
    set.insert("debug");
    set.insert("warning");
    set.insert("info");
    set.insert("error");
    set.insert("fatal");

    return ((parameters.size() == 1) && isValueInSet(parameters[0], set));
}

} // namespace
