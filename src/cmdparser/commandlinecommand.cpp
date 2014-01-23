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

#include <ghoul/cmdparser/commandlinecommand.h>

#include <sstream>

namespace ghoul {
namespace cmdparser {

CommandlineCommand::CommandlineCommand(const std::string& name, const std::string& shortName, const std::string& infoText,
                 const std::string& parameterList, const int argumentNum, const bool allowMultipleCalls)
    : _name(name)
    , _shortName(shortName)
    , _infoText(infoText)
    , _parameterList(parameterList)
    , _argumentNum(argumentNum)
    , _allowsMultipleCalls(allowMultipleCalls)
    , _errorMsg("")
{}

CommandlineCommand::~CommandlineCommand() {}

const std::string& CommandlineCommand::name() const {
    return _name;
}

const std::string& CommandlineCommand::shortName() const {
    return _shortName;
}

const std::string& CommandlineCommand::parameterList() const {
    return _parameterList;
}

const std::string& CommandlineCommand::infoText() const {
    return _infoText;
}

int CommandlineCommand::argumentNumber() const {
    return _argumentNum;
}

bool CommandlineCommand::allowsMultipleCalls() const {
    return _allowsMultipleCalls;
}

std::string CommandlineCommand::usage() const {
    std::string result = "[";
    if (shortName() != "")
        result = result + "<" + shortName() + "|" + name() + ">";
    else
        result += name();

    if (parameterList() != "")
        result = result + " " + parameterList();

    result += "]";

    return result;
}

std::string CommandlineCommand::help() const {
    std::string result;
    if (shortName() != "")
        result = shortName() + "|" + name() + ": \t" + infoText();
    else
        result = name() + ": \t" + infoText();

    return result;
}

const std::string& CommandlineCommand::errorMessage() const {
    return _errorMsg;
}

bool CommandlineCommand::checkParameters(const std::vector<std::string>& parameters) {
    return (parameters.size() == static_cast<size_t>(argumentNumber()));
}

bool CommandlineCommand::isValueInSet(const std::string& value, 
                                      const std::set<std::string>& set)
{
    return (set.find(value) != set.end());
}

} // namespace cmdparser
} // namespace ghoul
