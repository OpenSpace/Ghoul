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

#ifndef __CMDLINEPARSER_H__
#define __CMDLINEPARSER_H__

#include <iostream>
#include <string>
#include <vector>

namespace ghoul {
namespace cmdparser {

class CommandlineCommand;

class CommandlineParser {
public:
    /**
     * Default constructor.
     * \param programName The name of the program. Used in the #displayUsage and
     * #displayHelp methods
     */
    CommandlineParser(const std::string& programName = "");

    /**
     * The destructor will also delete all the contained commands within.
     */
    ~CommandlineParser();

    /**
     * Sets the command-line given from the main function.
     * \TODO Make variables into references so the commandline parser can change them
     * \param argc The number of arguments
     * \param argv The arguments themselves
     */
    void setCommandLine(int argc, char** argv);

    /**
     * Parses the command-line (#setCommandLine), evaluates all the commands
     * (#CommandlineCommand::checkParameters) and executes (#CommandlineCommand::execute)
     * them. The nameless command (#addCommandForNamelessArguments) will be checked last,
     * but executed first. The execution order of other CommandlineCommands is undefined.
     * \return <code>true</code> if the execution passed successfully; <code>false</code>
     * is returned if either the execution failed or the user requested the help. If the
     * execution fails, parts of the commands may have been already executed, thus leaving
     * the application in an half-initialized state
     */
    bool execute();

    /**
     * Add a new command to the parser. This method transfers ownership of the
     * CommandlineCommand from the caller to the CommandlineParser, which will delete the
     * command upon destruction.
     * \param cmd The new command that is added to the CommandlineParser. The
     * #CommandlineCommand::name and #CommandlineCommand::shortName have to unique to the
     * CommandlineParser, must not be <code>Nameless</code> and have to start with at
     * least one <code>-</code> or the addition will fail.
     * Furthermore, if the <code>cmd</code> already has been added to the
     * CommandlineParser, this method will fail. In both cases, <code>false</code> is
     * returned.
     * \param cmd The command that is to be added. The ownership of the CommandlineCommand
     * will be transferred to the CommandlineParser
     * \return <code>true</code> if the command was added successfully; <code>false</code>
     * if it either already existed or the #CommandlineCommand::name or
     * #CommandlineCommand::shortName were not unique.
     */
    bool addCommand(CommandlineCommand* cmd);

    /**
     * Add a new command to take care of the nameless arguments. Nameless arguments do not
     * have a name or a short name. This method transfers ownership of the
     * CommandlineCommand from the caller to the CommandlineParser, which will delete the
     * command upon destruction. If there already is an CommandlineCommand present for
     * nameless argument, it will be replaced by <code>cmd</code>.
     * \param cmd The command responsible for those arguments
     */
    void addCommandForNamelessArguments(CommandlineCommand* cmd);

    /// Returns the first commandline argument containing the path and the executable name
    /**
     * Returns the first command-line argument containing the path and the executable of
     * the running program.
     * \return The full path to the running program
     */
    const std::string& programPath() const;

    /**
     * Prints usage information to the provided <code>stream</code>. If the argument
     * <code>command</code> is empty, the usage information for all registered commands
     * is printed. Otherwise only the information for the CommandlineCommand with the
     * #CommandlineCommand::name or #CommandlineCommand::shortName equal to 
     * <code>command</code> will be printed. <code>Nameless</code> is a placeholder name
     * for the nameless command (#addCommandForNamelessArguments).
     * \param command Show information for the command with the #CommandlineCommand::name
     * or #CommandlineCommand::shortName only. If <code>command</code> is empty, the
     * usage information for all commands is printed; if <code>command = "Nameless"</code>
     * the usage information for the nameless argument is logged
     * \param stream The stream to which the usage information should be printed
     */
    void displayUsage(const std::string& command = "", std::ostream& stream = std::cout);

    /**
     * Print the full help test to the provided <code>stream</code>. It consists of the
     * usage information (#displayUsage) followed by the help text for each
     * CommandlineCommand (#CommandlineCommand::help).
     * \param stream The stream to which the help information should be printed
     */
    void displayHelp(std::ostream& stream = std::cout);


protected:
    /** Returns the CommandlineCommand with a specific #CommandlineCommand::shortName or
     * #CommandlineCommand::name from the list of stored commands. If no such command
     * exists <code>nullptr</code> will be returned.
     * \param shortOrLongName Either the #CommandlineCommand::name or
     * #CommandlineCommand::shortName of the command that should be fetched
     * \return The command that respond to the given name or short name, or
     * <code>nullptr</code> if no such CommandlineCommand exists
     */
    CommandlineCommand* getCommand(const std::string& shortOrLongName);

    /// The stored commands
    std::vector<CommandlineCommand*> _commands;

    /// The command we want to use for nameless arguments
    CommandlineCommand* _commandForNamelessArguments;

    /// All the arguments passed onto this parser
    std::vector<std::string> _arguments;

    /// The path to the program + filename
    std::string _programPath;

    /// The name of the program used in the \sa usage method
    std::string _programName;
};

} // namespace cmdparser
} // namespace ghoul

#endif // __CMDLINEPARSER_H__
