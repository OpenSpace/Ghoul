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

#include "voreen/core/utils/cmdparser/command.h"

#include <sstream>

namespace voreen {

Command::Command(const std::string& name, const std::string& shortName, const std::string& infoText,
                 const std::string& parameterList, const int argumentNum, const bool allowMultipleCalls)
    : name_(name)
    , shortName_(shortName)
    , infoText_(infoText)
    , parameterList_(parameterList)
    , argumentNum_(argumentNum)
    , allowMultipleCalls_(allowMultipleCalls)
    , errorMsg_("")
{}

Command::~Command() {}

const std::string Command::getName() {
    return name_;
}

const std::string Command::getShortName() {
    return shortName_;
}

const std::string Command::getParameterList() {
    return parameterList_;
}

const std::string Command::getInfoText() {
    return infoText_;
}

int Command::getArgumentNumber() {
    return argumentNum_;
}

bool Command::getAllowMultipleCalls() {
    return allowMultipleCalls_;
}

const std::string Command::usage() {
    std::string result = "[";
    if (getShortName() != "")
        result = result + "<" + getShortName() + "|" + getName() + ">";
    else
        result += getName();

    if (getParameterList() != "")
        result = result + " " + getParameterList();

    result += "]";

    return result;
}

const std::string Command::help() {
    std::string result;
    if (getShortName() != "")
        result = getShortName() + "|" + getName() + ": \t" + getInfoText();
    else
        result = getName() + ": \t" + getInfoText();

    return result;
}

const std::string Command::getErrorMessage() {
    return errorMsg_;
}

bool Command::checkParameters(const std::vector<std::string>& parameters) {
    return (parameters.size() == static_cast<size_t>(getArgumentNumber()));
}

bool Command::isValueInSet(const std::string& value, const std::set<std::string>& set) {
    return (set.find(value) != set.end());
}

}   //namespace voreen
