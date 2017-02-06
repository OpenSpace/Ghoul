/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2017                                                               *
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

#ifndef __GHOUL___SINGLECOMMAND___H__
#define __GHOUL___SINGLECOMMAND___H__

#include <ghoul/cmdparser/commandlinecommand.h>

namespace ghoul {
namespace cmdparser {

/**
 * This class represents a command that can occur only once in a given commandline and can
 * have up to 4 arguments of respective types \p T, \p U, \p V, and \p U. The concrete
 * amount of variables is determined by the constructor used. The command tries to convert
 * the parameters to the appropriate types and stores them. The template classes \p T,
 * \p U, \p V, and \p U must be convertable using an <code>std::stringstream</code>.
 * \tparam T The typename of the first argument type
 * \tparam U The typename of the second argument type, defaulting to \p T
 * \tparam V The typename of the third argument type, defaulting to \p U
 * \tparam W The typename of the fourth argument type, defaulting to \p V
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
     * \pre \p ptr1 must not be a nullptr
     */
    SingleCommand(T* ptr1,
        std::string name, std::string shortName = "", std::string infoText = "",
        std::string parameterList = "");

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
     * \pre \p ptr1 must not be a nullptr
     * \pre \p ptr2 must not be a nullptr
     */
    SingleCommand(T* ptr1, U* ptr2,
        std::string name, std::string shortName = "", std::string infoText = "",
        std::string parameterList = "");

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
     * \pre \p ptr1 must not be a nullptr
     * \pre \p ptr2 must not be a nullptr
     * \pre \p ptr3 must not be a nullptr
     */
    SingleCommand(T* ptr1, U* ptr2, V* ptr3,
        std::string name, std::string shortName = "", std::string infoText = "",
        std::string parameterList = "");

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
     * \pre \p ptr1 must not be a nullptr
     * \pre \p ptr2 must not be a nullptr
     * \pre \p ptr3 must not be a nullptr
     * \pre \p ptr4 must not be a nullptr
     */
    SingleCommand(T* ptr1, U* ptr2, V* ptr3, W* ptr4,
        std::string name, std::string shortName = "", std::string infoText = "",
        std::string parameterList = "");

    /**
     * Executes this SingleCommand and stores the values passed as \p parameters into the
     * pointers passed through the constructor.
     * \param parameters The parameters for this SingleCommand
     * \throws CommandExecutionException If one parameter has the wrong type that was not
     * detected in the checkParameters method
     */
    void execute(const std::vector<std::string>& parameters) override;

    /**
     * Checks whether all of the \p parameters have the correct types.
     * \param parameters The list of parameters that are to be checked
     * \throw CommandParameterException If any of the parameters have the wrong type
     */
    void checkParameters(const std::vector<std::string>& parameters) const override;

protected:
    T* _ptr1 = nullptr;
    U* _ptr2 = nullptr;
    V* _ptr3 = nullptr;
    W* _ptr4 = nullptr;
};


/**
 * This class represents a command with zero arguments that can only occur once in a given
 * commandline. The bool pointer will be set to <code>true</code>, if the command is
 * executed
 */
class SingleCommandZeroArguments : public CommandlineCommand {
public:
    /**
     * This constructor uses one <code>bool</code> parameter. The command does not take
     * ownership of the passed value.
     * \param ptr The pointer to the bool that will be set to <code>true</code> on
     * execution
     * \param name The full name for this command. Has to start with a <code>-</code> in
     * order to be valid
     * \param shortName The (optional) short name for this command. If it is provided, it
     * has to start with a <code>-</code> in order to be valid
     * \param infoText The info text that will be presented to the user if it is requested
     * by the CommandlineParser
     */
    SingleCommandZeroArguments(bool* ptr, std::string name, std::string shortName = "",
        std::string infoText = "");

    /**
     * Sets the <code>bool</code> value passed in the constructor to <code>true</code>
     */
    void execute(const std::vector<std::string>& /*parameters*/);

protected:
    bool* _ptr = nullptr;
};

} // namespace cmdparser
} // namespace ghoul

#include "singlecommand.inl"

#endif // __GHOUL___SINGLECOMMAND___H__
