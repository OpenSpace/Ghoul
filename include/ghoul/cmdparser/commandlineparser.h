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
     * but executed first. The order of other CommandlineCommands are undefined.
     */
    void execute();

    /**
     * Add a new command to the parser.
     * \param cmd The new command
     */
    void addCommand(CommandlineCommand* cmd);

    /**
     * Add a new command to take care of the nameless arguments
     * \param cmd The command responsible for those arguments
     */
    void addCommandForNamelessArguments(CommandlineCommand* cmd);

    /// Returns the first commandline argument containing the path and the executable name
    std::string getProgramPath() const;

    /// Sets the verbosity of the commandline parser. When set, each (command,argument) pair will be
    /// printed to std::cout
    void setVerbosity(const bool verbosity);

    /**
     * Prints the usage information to the std::cout.
     * \param command Show information for this command only, show all if empty.
     */
    void displayUsage(const std::string& command = "");

    /// Print the full help text to the std::cout
    void displayHelp();


protected:
    /// Returns the command with a specific <code>shortName</code> or <code>name</code>. If no such
    /// command exists, 0 is returned
    CommandlineCommand* getCommand(const std::string& shortOrLongName);

    /**
     * Bail out, display the message <code>msg</code> and display the usage
     * \param command Command with which the error happened.
     */
    void exitWithError(const std::string& msg, const std::string& command = "");

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

    /// Controls the verbosity of the commandline parser in regard to execution
    bool _verbosity;
};

} // namespace cmdparser
} // namespace ghoul

#endif // __CMDLINEPARSER_H__
