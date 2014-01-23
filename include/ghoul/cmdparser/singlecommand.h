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

#ifndef VRN_SINGLECOMMAND_H
#define VRN_SINGLECOMMAND_H

#include "voreen/core/utils/cmdparser/command.h"

namespace voreen {

/**
 * This class represents a command which can occur only once in a given commandline and can have
 * 1-4 arguments of types T, U, V and U.
 * This class simply tries to convert the parameters to the appropriate values and stores them in it
 * The template must be usable within a stringstream (necessary for conversion)
 * \sa SingleCommandZeroArguments
 */
template<class T, class U = T, class V = U, class W = V>
class SingleCommand : public Command {
public:
    SingleCommand(T* ptr1,
        const std::string& name, const std::string& shortName = "", const std::string& infoText = "",
        const std::string parameterList = "")
        : Command(name, shortName, infoText, parameterList, 1, false)
        , ptr1_(ptr1)
        , ptr2_(0)
        , ptr3_(0)
        , ptr4_(0)
    {}

    SingleCommand(T* ptr1, U* ptr2,
        const std::string& name, const std::string& shortName = "", const std::string& infoText = "",
        const std::string parameterList = "")
        : Command(name, shortName, infoText, parameterList, 2, false)
        , ptr1_(ptr1)
        , ptr2_(ptr2)
        , ptr3_(0)
        , ptr4_(0)
    {}

    SingleCommand(T* ptr1, U* ptr2, V* ptr3,
        const std::string& name, const std::string& shortName = "", const std::string& infoText = "",
        const std::string parameterList = "")
        : Command(name, shortName, infoText, parameterList, 3, false)
        , ptr1_(ptr1)
        , ptr2_(ptr2)
        , ptr3_(ptr3)
        , ptr4_(0)
    {}

    SingleCommand(T* ptr1, U* ptr2, V* ptr3, W* ptr4,
        const std::string& name, const std::string& shortName = "", const std::string& infoText = "",
        const std::string parameterList = "")
        : Command(name, shortName, infoText, parameterList, 4, false)
        , ptr1_(ptr1)
        , ptr2_(ptr2)
        , ptr3_(ptr3)
        , ptr4_(ptr4)
    {}

    ~SingleCommand() {}
    bool execute(const std::vector<std::string>& parameters) {
        *ptr1_ = cast<T>(parameters[0]);
        if (ptr2_ != 0)
            *ptr2_ = cast<U>(parameters[1]);
        if (ptr3_ != 0)
            *ptr3_ = cast<V>(parameters[2]);
        if (ptr4_ != 0)
            *ptr4_ = cast<W>(parameters[3]);

        return true;
    }

    bool checkParameters(const std::vector<std::string>& parameters) {
        std::ostringstream errorStr;

        bool result = parameters.size() == static_cast<size_t>(argumentNum_);
        if (!result) {
            errorStr << "Invalid number of parameters: " << parameters.size();
            errorStr << ", expected: " << argumentNum_;
            errorMsg_ = errorStr.str();
            return false;
        }

        result &= is<T>(parameters[0]);
        if (!result)
            errorStr << "First parameter invalid";

        if (result && ptr2_) {
            result &= is<U>(parameters[1]);
            if (!result)
                errorStr << "Second parameter invalid";
        }

        if (result && ptr3_) {
            result &= is<V>(parameters[2]);
            if (!result)
                errorStr << "Third parameter invalid";
        }

        if (result && ptr4_) {
            result &= is<W>(parameters[3]);
            if (!result)
                errorStr << "Fourth parameter invalid";
        }

        if (!result)
            errorMsg_ = errorStr.str();

        return result;
    }

protected:
    T* ptr1_;
    U* ptr2_;
    V* ptr3_;
    W* ptr4_;
};


/**
 * This class represents a command with zero arguments which can only occur once in a given commandline.
 * The pointer will be set to "true", if the command is executed
 * The template must be usable within a stringstream (necessary for conversion)
 * \sa SingleCommand
 */
class SingleCommandZeroArguments : public Command {
public:
    SingleCommandZeroArguments(bool* ptr, const std::string& name, const std::string& shortName = "",
        const std::string& infoText = "")
        : Command(name, shortName, infoText, "", 0, false)
        , ptr_(ptr)
    {}

    bool execute(const std::vector<std::string>& /*parameters*/) {
        *ptr_ = true;
        return true;
    }

protected:
    bool* ptr_;
};

} // namespace

#endif // VRN_SINGLECOMMAND_H
