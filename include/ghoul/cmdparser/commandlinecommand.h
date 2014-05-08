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

#ifndef __COMMANDLINECOMMAND_H__
#define __COMMANDLINECOMMAND_H__

#include <exception>
#include <sstream>
#include <string>
#include <vector>
#include <set>

namespace ghoul {
namespace cmdparser {

/**
 * A command is an operation that can be called via command line arguments on program
 * startup. As this class is virtual, it has to be derived from with the #execute method
 * implemented. The CommandlineCommand are used by adding them to the CommandlineParser,
 * using the method #CommandlineParser::addCommand. The common way of using the commands
 * is to pass a variable of appropriate type to the CommandlineCommand by reference, which
 * gets set to the correct value when the command is executed.
 * Within the parser, the command's <code>name</code> and <code>shortName</code> must be
 * unique.
 *
 * There exist a number of defined convenience classes to use, for example
 * CommandlineCommandBoolean, CommandlineCommandFloat, CommandlineCommandTwoFloat,
 * CommadnlineCommandInteger, CommandlineCommandTwoInteger, CommandlineCommandString,
 * CommandlineCommandTwoString that accept one or two of the listed parameter-types.
 *
 * For generic use, see the templated classes SingleCommandlineCommand and
 * MultipleCommandlineCommand which are capable of setting basic types that are
 * convertible from string using a <code>std::stringstream</code>.
 */
class CommandlineCommand {
public:
    /**
     * The constructor which saves the arguments to own member variables.
     * \param name The (long) name of the parameter. For example <code>--command1</code>
     * \param shortName The abbreviated name of the parameter. For example <code>-c</code>
     * \param infoText A short text (preferably one line) explaining what the command
     * does. Used in the #CommandlineParser::displayHelp method
     * \param parameterList A user-readable description which parameters are used and
     * supported. This is used in the #CommandlineParser::displayUsage and
     * #CommandlineParser::displayHelp methods
     * \param argumentNum The number of arguments this command accepts
     * \param allowMultipleCalls If this argument is <code>true</code> it signals the
     * CommandlineParser that it should allow multiple instances of this
     * CommandlineCommand in a single command line
     */
    CommandlineCommand(std::string name, std::string shortName = "",
                       std::string infoText = "", std::string parameterList = "",
                       int argumentNum = 1, bool allowMultipleCalls = false);
    virtual ~CommandlineCommand();

    /**
     * Return the full name of this command.
     * \return The full name of this command
     */
    const std::string& name() const;

    /**
     * Returns the short name of this command.
     * \return The short name of this command
     */
    const std::string& shortName() const;

    /**
     * Returns the parameter list necessary for the #usage method
     * \return The parameter list this command expects
     */
    const std::string& parameterList() const;

    /**
     * Returns a short description used in the #CommandlineParser::displayHelp and
     * #CommandlineParser::displayUsage methods
     * \return A short description of this command
     */
    const std::string& infoText() const;

    /// Returns the number of accepted arguments for this command
    int argumentNumber() const;

    /// Returns if the command can be called more than once in a single command line
    bool allowsMultipleCalls() const;

    /**
     * Returns a message describing the error reported by #checkParameters, or an empty
     * string, if no error has occurred.
     */
    const std::string& errorMessage() const;

    /**
     * Executes this command with the given parameters. Each subclass must implement this
     * abstract method and perform all actions within it.
     * \param parameters The parameters needed for the execution of this command. By the
     * time this method is called, the parameters have already been verified by the
     * #checkParameters method
     * \return This method should return <code>true</code>, if the execution was
     * successful, <code>false</code> otherwise and log possible errors
     */
    virtual bool execute(const std::vector<std::string>& parameters) = 0;

    /**
     * Checks the parameters for consistency and correct amount. The basic implementation
     * only checks for the correct number of parameters. If you want to test for other
     * conditions (for example type), overwrite this method in the derived class.
     * \param parameters The parameters which should be tested
     * \return <code>true</code>, if the parameters are correct, <code>false</code>
     * otherwise
     */
    virtual bool checkParameters(const std::vector<std::string>& parameters);

    /**
     * Returns the usage part for the help of this CommandlineCommand. Used in the
     * #CommandlineParser::usage method.
     * \return The usage part for the help of this command
     */
    virtual std::string usage() const;

    /// Returns the help-part for a command. Used in the help()-method from the
    /// commandlineparser
    virtual std::string help() const;

protected:
    /**
     * Tries to cast the string value <code>s</code> to the templated parameter T. If this
     * succeeds, <code>true</code> is returned; otherwise <code>false</code>. The
     * conversion is done via an <code>std::stringstream</code> so it can only cast those
     * types supported by the stream.
     * \tparam T The type of the value which should be converted
     * \param s The <code>std::string</code> representation of the value
     * \param t The reference which will store the converted value
     * \return <code>true</code> if the conversion was successful, <code>false</code>
     * otherwise
     */
    template <class T>
    bool cast(const std::string& s, T& t) {
        std::istringstream iss(s);
        iss >> std::dec >> t;
        return !(iss.fail());
    }

    /**
     * Checks if the string value <code>s</code> can be cast into the type <code>T</code>.
     * It only returns <code>true</code> for those values that can be converted using an
     * <code>std::stringstream</code>
     * \tparam T The type of the value which should be converted
     * \param s The <code>std::string</code> representation of the value
     * \return <code>true</code> if the value can be converted, <code>false</code>
     * otherwise
     */
    template <class T>
    bool is(const std::string& s) {
        std::istringstream iss(s);
        T t;
        iss >> std::dec >> t;
        return !(iss.fail());
    }

    /// Name of the command used on command-line level
    std::string _name;
    /// The short name of this command which is also usable (usually an abbreviation)
    std::string _shortName;
    /// A description of the command; used in the #help method
    std::string _infoText;
    /// The parameter list necessary for the #usage method
    std::string _parameterList;
    /// Name used as a prefix for logging
    std::string _loggerCat;
    /// Stores the number of arguments this command accepts
    int _argumentNum;
    /// Stores, if the command can be called multiple times in a single command line
    bool _allowsMultipleCalls;
    /// Error message set by checkParameters().
    std::string _errorMsg;
};

}  // namespace cmdparser
}  // namespace ghoul

#endif  //__COMMANDLINECOMMAND_H__
