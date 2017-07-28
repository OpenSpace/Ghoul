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

#ifndef __GHOUL___COMMANDLINEPARSER___H__
#define __GHOUL___COMMANDLINEPARSER___H__

#include <ghoul/misc/boolean.h>
#include <ghoul/misc/exception.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace ghoul::cmdparser {

class CommandlineCommand;

class CommandlineParser {
public:
    using AllowUnknownCommands = ghoul::Boolean;

    /**
     * The exception that is thrown in the CommandlineParser::execute method when an
     * error occurs.
     */
    struct CommandlineException : public RuntimeError {
        explicit CommandlineException(const std::string& msg);
    };
    
    /**
     * Default constructor.
     * \param programName The name of the program. Used in the #displayUsage and
     * #displayHelp methods
     * \param allowUnknownCommands If this is set to <code>true</code> the
     * CommandlineParser will ignore commands it did not recognize and extract only the
     * used commands from the passed command-line, leaving the unknown commands untouched.
     * This allows other parts of the program to deal with command-line arguments as well.
     * \param shortHelpCommand The short version of the command used to request help
     * information
     * \param longHelpCommand The long version of the command used to request help
     * information
     */
    CommandlineParser(std::string programName = "",
        AllowUnknownCommands allowUnknownCommands = AllowUnknownCommands::No,
        std::string shortHelpCommand = "-h",
        std::string longHelpCommand = "--help");

    /**
     * The destructor will also delete all the contained commands within.
     */
    ~CommandlineParser();
    
    /**
     * Sets if this CommandlineParser allows command-line arguments that do not belong to
     * any registered CommandlineCommand. If this is set to <code>true</code>, a receiving
     * <code>std::vector</code> has to be provided in the setCommandline method which
     * will contain all command-line arguments that were not consumed by this parser.
     * Warning: This method is only valid to be called before setCommmandLine. Calling it
     * after the command-line has been set results in undefined behavior.
     * \param allowUnknownCommands If set to <code>true</code> this parser allows unknown
     * arguments without a corresponding CommandlineCommand. If <code>false</code> this
     * will lead to logging of errors and #execute will return <code>false</code> if any
     * unknown commands are encountered during execution
     */
    void setAllowUnknownCommands(AllowUnknownCommands allowUnknownCommands);
    
    /**
     * Returns if this CommandlineParser allows unknown commands, which do not have a
     * CommandlineCommand associated with them, in the command-line set by
     * setCommandline.
     * \return <code>true</code> if unknown command-line arguments are allowed by this
     * CommandlineParser; <code>false</code> otherwise
     */
    AllowUnknownCommands allowsUnknownCommands() const;

    /**
     * Sets the command-line given from the main function.
     * \param arguments The commandline arguments, beginning with the name of the
     * application
     * \return The storage for the commands that have not been consumed by the
     * CommandlineParser. The <code>vector</code> will be cleared by this function and
     * will be filled by the execute method
     * \pre \p arguments must not be empty
     */
    std::shared_ptr<const std::vector<std::string>> setCommandLine(
        std::vector<std::string> arguments
    );

    /**
     * Parses the command-line (setCommandLine), evaluates all the commands
     * (CommandlineCommand::checkParameters) and executes (CommandlineCommand::execute)
     * them. The nameless command (addCommandForNamelessArguments) will be checked last,
     * but executed first. The execution order of other CommandlineCommands occurs in the
     * order they appear in the command-line passed in setCommandLine. If the method
     * throws a CommandExecutionException, only some of the commands may have been
     * executed, thus leaving the CommandlineCommands in an undefined state. In the case
     * of any other exception, none of the CommandlineCommands have been executed.
     * \return <code>true</code> if the help message should be displayed by the caller
     * using the displayHelp method.
     * \throws CommandlineException If there are malformed commandline parameters that
     * were passed in the setCommandLine method. Malformed parameters may be due to
     * invalid commands, multiple commands for commands that do not allow for multiple
     * executions, wrong parameter types, error with unnamed arguments
     * \throws CommandExecutionException If the execution of a CommandlineCommand failed
     */
    bool execute();

    /**
     * Add a new command to the parser. This method transfers ownership of the
     * CommandlineCommand from the caller to the CommandlineParser, which will delete the
     * command upon destruction.
     * \param cmd The command that is to be added. The ownership of the CommandlineCommand
     * will be transferred to the CommandlineParser
     * \return <code>true</code> if the command was added successfully; <code>false</code>
     * if it either already existed or the CommandlineCommand::name or
     * CommandlineCommand::shortName were not unique.
     * \pre \p cmd must not be <code>nullptr</code>
     * \pre The name of \p cmd may not have been used in a previous registered
     * CommandlineCommand's name or short name
     * \pre If \p cmd has a short name, it may not have been used in a previous registered
     * CommandlineCommand's name or short name
     */
    bool addCommand(std::unique_ptr<CommandlineCommand> cmd);

    /**
     * Add a new command to take care of the nameless arguments. Nameless arguments do not
     * have a name or a short name. This method transfers ownership of the
     * CommandlineCommand from the caller to the CommandlineParser, which will delete the
     * command upon destruction. If there already is an CommandlineCommand present for
     * nameless argument, it will be replaced by <code>cmd</code>.
     * \param cmd The command responsible for those arguments
     * \pre \p cmd must not be <code>nullptr</code>
     */
    void addCommandForNamelessArguments(std::unique_ptr<CommandlineCommand> cmd);

    /**
     * Returns if a command with the provided \p name has previously been registered.
     * \return <code>true</code> if a CommandlineCommand with the provided \p name has
     * previously been registered; <code>false</code> otherwise
     */
    bool hasCommandForName(const std::string& name) const;

    /**
     * Returns if a command with the provided \p shortName has previously been registered.
     * \return <code>true</code> if a CommandlineCommand with the provided \p shortName
     * has previously been registered; <code>false</code> otherwise
     */
    bool hasCommandForShortName(const std::string& shortName) const;
    
    /**
     * Returns the first command-line argument containing the path and the executable of
     * the running program.
     * \return The full path to the running program
     */
    const std::string& programPath() const;

    /**
     * Prints usage information to the provided \p stream. If the argument \p command is
     * empty, the usage information for all registered commands is printed. Otherwise only
     * the information for the CommandlineCommand with the CommandlineCommand::name or
     * CommandlineCommand::shortName equal to \p command will be printed.
     * <code>Nameless</code> is a placeholder name for the nameless command
     * (#addCommandForNamelessArguments).
     * \param command Show information for the command with the CommandlineCommand::name
     * or CommandlineCommand::shortName only. If <code>command</code> is empty, the
     * usage information for all commands is printed; if <code>command = "Nameless"</code>
     * the usage information for the nameless argument is logged
     * \param stream The stream to which the usage information is printed
     */
    void displayUsage(const std::string& command = "",
        std::ostream& stream = std::cout) const;

    /**
     * Print the full help test to the provided \p stream. It consists of the usage
     * information (#displayUsage) followed by the help text for each CommandlineCommand
     * (CommandlineCommand::help).
     * \param stream The stream to which the help information is printed
     */
    void displayHelp(std::ostream& stream = std::cout) const;

protected:
    /** Returns the CommandlineCommand with a specific CommandlineCommand::shortName or
     * CommandlineCommand::name from the list of stored commands. If no such command
     * exists <code>nullptr</code> will be returned.
     * \param shortOrLongName Either the CommandlineCommand::name or
     * CommandlineCommand::shortName of the command that should be fetched
     * \return The command that respond to the given name or short name, or
     * <code>nullptr</code> if no such CommandlineCommand exists
     */
    CommandlineCommand* getCommand(const std::string& shortOrLongName);
    
    /**
     * Returns <code>true</code> if the \a _arguments contains only a single help
     * argument.
     * \return Whether the \a _arguments only contain a single help argument
     */
    bool hasOnlyHelpCommand() const;

    /// The stored commands
    std::vector<std::unique_ptr<CommandlineCommand>> _commands;

    /// The command we want to use for nameless arguments
    std::unique_ptr<CommandlineCommand> _commandForNamelessArguments;

    /// All the arguments passed onto this parser
    std::vector<std::string> _arguments;
    
    /// The pointer to the vector that will store all the arguments which have not been
    /// consumed by the CommandlineParser
    std::shared_ptr<std::vector<std::string>> _remainingArguments;

    /// The path to the program + filename
    std::string _programPath;

    /// The name of the program used in the \sa usage method
    std::string _programName;
    
    /// Should the CommandlineParser allow unknown commands or throw an error in that case
    AllowUnknownCommands _allowUnknownCommands;
    
    /// Short version of the command to request help information
    std::string _shortHelpCommand;
    
    /// Long version of the command to request help information
    std::string _longHelpCommand;
};

} // namespace ghoul::cmdparser

#endif // __GHOUL___COMMANDLINEPARSER___H__
