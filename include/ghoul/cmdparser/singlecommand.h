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

#ifndef __SINGLECOMMAND_H__
#define __SINGLECOMMAND_H__

#include <ghoul/cmdparser/commandlinecommand.h>
#include <ghoul/logging/logmanager.h>

namespace ghoul {
namespace cmdparser {

/**
 * This class represents a command that can occur only once in a given commandline and can
 * have up to 4 arguments of respective types <code>T</code>, <code>U</code>,
 * <code>V</code>, and <code>U</code>. The concrete amount of variables is determined by
 * the constructor used. The command tries to convert the parameters to the appropriate
 * types and stores them. The template classes <code>T</code>, <code>U</code>,
 * <code>V</code>, and <code>U</code> must be convertable using an
 * <code>std::stringstream</code>.
 * \tparam T The typename of the first argument type
 * \tparam U The typename of the second argument type, defaulting to <code>T</code>
 * \tparam V The typename of the third argument type, defaulting to <code>U</code>
 * \tparam W The typename of the fourth argument type, defaulting to <code>V>/code>
 * \sa SingleCommandZeroArguments
 */
template<typename T, typename U = T, typename V = U, typename W = V>
class SingleCommand : public CommandlineCommand {
public:
    /**
     * This constructor uses the one parameter. The command does not take ownership of the
     * passed value.
     * \param ptr1 The pointer to the parameter that will be set when this command is
     * executed
     * \param name The full name for this command. Has to start with a <code>-</code> in
     * order to be valid
     * \param shortName The (optional) short name for this command. If it is provided, it 
     * has to start with a <code>-</code> in order to be valid
     * \param infoText The info text that will be presented to the user if it is requested
     * by the CommandlineParser
     * \param parameterList The explanation of the parameters that this command expects.
     * Is presented to the user upon request by the CommandlineParser
     */
    SingleCommand(T* ptr1,
                  const std::string& name, const std::string& shortName = "",
                  const std::string& infoText = "", const std::string parameterList = "")
        : CommandlineCommand(name, shortName, infoText, parameterList, 1, false)
        , _ptr1(ptr1)
        , _ptr2(nullptr)
        , _ptr3(nullptr)
        , _ptr4(nullptr)
    {}

    /**
     * This constructor uses two parameters. These can be of the same or different types.
     * The command does not take ownership of the value.
     * \param ptr1 The pointer to the parameter that will be set when this command is
     * executed
     * \param ptr2 The pointer to the second parameter that will be set when this command
     * is executed
     * \param name The full name for this command. Has to start with a <code>-</code> in
     * order to be valid
     * \param shortName The (optional) short name for this command. If it is provided, it
     * has to start with a <code>-</code> in order to be valid
     * \param infoText The info text that will be presented to the user if it is requested
     * by the CommandlineParser
     * \param parameterList The explanation of the parameters that this command expects.
     * Is presented to the user upon request by the CommandlineParser
     */
    SingleCommand(T* ptr1, U* ptr2,
                  const std::string& name, const std::string& shortName = "",
                  const std::string& infoText = "", const std::string parameterList = "")
        : CommandlineCommand(name, shortName, infoText, parameterList, 2, false)
        , _ptr1(ptr1)
        , _ptr2(ptr2)
        , _ptr3(nullptr)
        , _ptr4(nullptr)
    {}

    /**
     * This constructor uses three parameters. These can be of the same or different
     * types. The command does not take ownership of the value.
     * \param ptr1 The pointer to the parameter that will be set when this command is
     * executed
     * \param ptr2 The pointer to the second parameter that will be set when this command
     * is executed
     * \param ptr3 The pointer to the third parameter that will be set when this command
     * is executed
     * \param name The full name for this command. Has to start with a <code>-</code> in
     * order to be valid
     * \param shortName The (optional) short name for this command. If it is provided, it
     * has to start with a <code>-</code> in order to be valid
     * \param infoText The info text that will be presented to the user if it is requested
     * by the CommandlineParser
     * \param parameterList The explanation of the parameters that this command expects.
     * Is presented to the user upon request by the CommandlineParser
     */
    SingleCommand(T* ptr1, U* ptr2, V* ptr3,
                  const std::string& name, const std::string& shortName = "",
                  const std::string& infoText = "", const std::string parameterList = "")
        : CommandlineCommand(name, shortName, infoText, parameterList, 3, false)
        , _ptr1(ptr1)
        , _ptr2(ptr2)
        , _ptr3(ptr3)
        , _ptr4(nullptr)
    {}

    /**
     * This constructor uses all four parameters. These can be of the same or different
     * types. The command does not take ownership of the value.
     * \param ptr1 The pointer to the parameter that will be set when this command is
     * executed
     * \param ptr2 The pointer to the second parameter that will be set when this command
     * is executed
     * \param ptr3 The pointer to the third parameter that will be set when this command
     * is executed
     * \param ptr4 The pointer to the fourth parameter that will be set when this command
     * is executed
     * \param name The full name for this command. Has to start with a <code>-</code> in
     * order to be valid
     * \param shortName The (optional) short name for this command. If it is provided, it
     * has to start with a <code>-</code> in order to be valid
     * \param infoText The info text that will be presented to the user if it is requested
     * by the CommandlineParser
     * \param parameterList The explanation of the parameters that this command expects.
     * Is presented to the user upon request by the CommandlineParser
     */
    SingleCommand(T* ptr1, U* ptr2, V* ptr3, W* ptr4,
                  const std::string& name, const std::string& shortName = "",
                  const std::string& infoText = "", const std::string parameterList = "")
        : CommandlineCommand(name, shortName, infoText, parameterList, 4, false)
        , _ptr1(ptr1)
        , _ptr2(ptr2)
        , _ptr3(ptr3)
        , _ptr4(ptr4)
    {}

    
    bool execute(const std::vector<std::string>& parameters) {
        cast(parameters[0], *_ptr1);
        if (_ptr2 != nullptr)
            cast(parameters[1], *_ptr2);
        if (_ptr3 != nullptr)
            cast(parameters[2], *_ptr3);
        if (_ptr4 != nullptr)
            cast(parameters[3], *_ptr4);

        return true;
    }

    bool checkParameters(const std::vector<std::string>& parameters) {
        std::ostringstream errorStr;

        bool result = parameters.size() == static_cast<size_t>(_argumentNum);
        if (!result) {
            errorStr << "Invalid number of parameters: " << parameters.size();
            errorStr << ", expected: " << _argumentNum;
            _errorMsg = errorStr.str();
            return false;
        }

        result &= is<T>(parameters[0]);
        if (!result)
            errorStr << "First parameter invalid";

        if (result && (_ptr2 != nullptr)) {
            result &= is<U>(parameters[1]);
            if (!result)
                errorStr << "Second parameter invalid";
        }

        if (result && (_ptr3 != nullptr)) {
            result &= is<V>(parameters[2]);
            if (!result)
                errorStr << "Third parameter invalid";
        }

        if (result && (_ptr4 != nullptr)) {
            result &= is<W>(parameters[3]);
            if (!result)
                errorStr << "Fourth parameter invalid";
        }

        if (!result)
            _errorMsg = errorStr.str();

        return result;
    }

protected:
    T* _ptr1;
    U* _ptr2;
    V* _ptr3;
    W* _ptr4;
};


/**
 * This class represents a command with zero arguments that can only occur once in a given
 * commandline. The bool pointer will be set to <code>true</code>, if the command is
 * executed
 */
class SingleCommandZeroArguments : public CommandlineCommand {
public:
    SingleCommandZeroArguments(bool* ptr, std::string name,
                               std::string shortName = "",
                               std::string infoText = "")
        : CommandlineCommand(std::move(name), std::move(shortName), std::move(infoText), "", 0, false)
        , _ptr(ptr)
    {}

    bool execute(const std::vector<std::string>& /*parameters*/) {
        *_ptr = true;
        return true;
    }

protected:
    bool* _ptr;
};

} // namespace cmdparser
} // namespace ghoul

#endif // VRN_SINGLECOMMAND_H
