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

#ifndef VRN_MULTIPLECOMMAND_H
#define VRN_MULTIPLECOMMAND_H

#include <vector>
#include "voreen/core/utils/cmdparser/command.h"

namespace voreen {

/**
 * This class represents a command which can be called multiple times in a given commandline and
 * can have 1-4 arguments of types T, U, V and U.
 * This class simply tries to convert the parameters to the appropriate values and stores them in it
 * The template must be usable within a stringstream (necessary for conversion)
 * \sa MultipleCommandZeroArguments
 */
template<class T, class U = T, class V = U, class W = V>
class MultipleCommand : public Command {
public:
    MultipleCommand(std::vector<T>* ptr1,
        const std::string& name, const std::string& shortName = "", const std::string& infoText = "",
        const std::string parameterList = "")
        : Command(name, shortName, infoText, parameterList, 1, true)
        , ptr1_(ptr1)
        , ptr2_(0)
        , ptr3_(0)
        , ptr4_(0)
    {}

    MultipleCommand(std::vector<T>* ptr1, std::vector<U>* ptr2,
        const std::string& name, const std::string& shortName = "", const std::string& infoText = "",
        const std::string parameterList = "")
        : Command(name, shortName, infoText, parameterList, 2, true)
        , ptr1_(ptr1)
        , ptr2_(ptr2)
        , ptr3_(0)
        , ptr4_(0)
    {}

    MultipleCommand(std::vector<T>* ptr1, std::vector<U>* ptr2, std::vector<V>* ptr3,
        const std::string& name, const std::string& shortName = "", const std::string& infoText = "",
        const std::string parameterList = "")
        : Command(name, shortName, infoText, parameterList, 3, true)
        , ptr1_(ptr1)
        , ptr2_(ptr2)
        , ptr3_(ptr3)
        , ptr4_(0)
    {}

    MultipleCommand(std::vector<T>* ptr1, std::vector<U>* ptr2, std::vector<V>* ptr3, std::vector<W>* ptr4,
        const std::string& name, const std::string& shortName = "", const std::string& infoText = "",
        const std::string parameterList = "")
        : Command(name, shortName, infoText, parameterList, 4, true)
        , ptr1_(ptr1)
        , ptr2_(ptr2)
        , ptr3_(ptr3)
        , ptr4_(ptr4)
    {}

    bool execute(const std::vector<std::string>& parameters) {
        ptr1_->push_back(cast<T>(parameters[0]));
        if (ptr2_ != 0)
            ptr2_->push_back(cast<U>(parameters[1]));
        if (ptr3_ != 0)
            ptr3_->push_back(cast<V>(parameters[2]));
        if (ptr4_ != 0)
            ptr4_->push_back(cast<W>(parameters[3]));

        return true;
    }

    bool checkParameters(const std::vector<std::string>& parameters) {
        bool result = parameters.size() == static_cast<size_t>(argumentNum_);

        result &= is<T>(parameters[0]);
        if (ptr2_ != 0)
            result &= is<U>(parameters[1]);
        if (ptr3_ != 0)
            result &= is<V>(parameters[2]);
        if (ptr4_ != 0)
            result &= is<W>(parameters[3]);

        return result;
    }

protected:
    std::vector<T>* ptr1_;
    std::vector<U>* ptr2_;
    std::vector<V>* ptr3_;
    std::vector<W>* ptr4_;
};

/**
 * This class represents a command with zero arguments which can be called multiple times in a given commandline.
 * The pointer will be set to "true", if the command is executed
 * The template must be usable within a stringstream (necessary for conversion)
 * \sa MultipleCommand
 */
class MultipleCommandZeroArguments : public Command {
public:
    MultipleCommandZeroArguments(std::vector<bool>* ptr, const std::string& name, const std::string& shortName = "",
        const std::string& infoText = "")
        : Command(name, shortName, infoText, "", 0, true)
        , ptr_(ptr)
    {}

    bool execute(const std::vector<std::string>& /*parameters*/) {
        ptr_->push_back(true);
        return true;
    }

protected:
    std::vector<bool>* ptr_;
};

} // namespace

#endif // VRN_MULTIPLECOMMAND_H
