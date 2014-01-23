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

#include <ghoul/cmdparser/commandlineparser.h>
#include <ghoul/cmdparser/commandlinecommand.h>
#include <ghoul/logging/logmanager.h>
#include <exception>
#include <map>
#include <stdlib.h>
#include <iostream>

using std::vector;
using std::multimap;
using std::pair;
using std::string;

namespace {

    const std::string _loggerCat = "CommandlineParser";

/**
 * Extracts multiple arguments from a single list. <br>
 * If <code>count</code> is <code>-2</code>, arguments will be extracted, as long as no new commands is found.
 * in[begin] itself will not be extracted if this is the command.<br>
 * If <code>count</code> is <code>-1</code>, the rest of the line will be extracted<br>
 * If <code>count</code> is > 0, that many arguments will be extracted and returned
 */
int extractArguments(const vector<string> in, vector<string>* out, const size_t begin, const int count) {
    int num = 0;
    if (count == -1) {
        for (size_t i = 0; (i < in.size()) && (static_cast<size_t>(begin+1+i) < in.size()) ; ++i, num++)
            out->push_back(in[begin+1+i]);
    }
    else
        if (count == -2) {
            // Extract arguments until a new command is found
            for (size_t i = begin; (i < in.size()) && (in[i][0] != '-'); ++i, num++) {
                out->push_back(in[i]);
            }
        }
    else {
        for (int i = 0; (i < count) && (static_cast<size_t>(begin+1+i) < in.size()) ; ++i, num++) {
            out->push_back(in[begin+1+i]);
        }
    }
    return num;
}

} // namespace

namespace ghoul {
namespace cmdparser {

CommandlineParser::CommandlineParser(const std::string& programName)
  : _commandForNamelessArguments(nullptr)
  , _programName(programName)
{}

CommandlineParser::~CommandlineParser() {
    for (CommandlineCommand* i : _commands)
        delete i;
    _commands.clear();
}

const std::string& CommandlineParser::programPath() const {
    return _programPath;
}

void CommandlineParser::setCommandLine(int argc, char** argv) {
    // argv[0] = program name
    // argv[i] = i-th argument
    if (argc > 0 && argv && argv[0])
        _programPath = argv[0];
    else
        _programPath = "";

    // Might be possible that someone calls us multiple times
    _arguments.clear();

    // Just add the arguments to the vector
    for (int i = 1; i < argc; ++i)
        _arguments.push_back(argv[i]);
}

void CommandlineParser::execute() {
    // There is only one argument and this is either "-h" or "--help" ; so display the help
    if ((_arguments.size() == 1) && ((_arguments[0] == "-h") || (_arguments[0] == "--help")))
        displayHelp();

    vector<string> argumentsForNameless;

    // We store the commands and parameters in a map to be able to execute them without parsing the
    // commandline again
    multimap<CommandlineCommand*, vector<string> > parameterMap;

    for (size_t i = 0 ; i < _arguments.size(); ++i) {
        // In the beginning we assume that we just started the loop or finished reading parameters for
        // one command

        // So test if the next argument is a command or a parameter for a nameless argument
        if (_arguments[i][0] != '-') {
            int number = extractArguments(_arguments, &argumentsForNameless, i, -2);
            i += (number - 1);
        }
        else {
            // We have found a command
            CommandlineCommand* currentCommand = getCommand(_arguments[i]);

            // currentCommand = 0, if there wasn't a command with that specific name or shortName
            if (currentCommand == 0)
                exitWithError(_arguments[i] + " is not a valid command");

            vector<string> parameters;
            int number = extractArguments(_arguments, &parameters, i, currentCommand->argumentNumber());
            i += number;

            // don't insert if the command doesn't allow multiple calls and already is in the map
            if (!currentCommand->allowsMultipleCalls() && parameterMap.find(currentCommand) != parameterMap.end())
                exitWithError(currentCommand->name() + " doesn't allow multiple calls in a single line",
                              currentCommand->name());

            parameterMap.insert(pair<CommandlineCommand*, vector<string> >(currentCommand, parameters));
        }
    }
    // We now have all the commands with the respective parameters stored in the map and all the parameters for
    // the nameless command is avaiable as well.

    // First step: Test, if we have nameless arguments even if we don't have a nameless command. Otherwise bail out
    if (!argumentsForNameless.empty() && (_commandForNamelessArguments == 0))
        exitWithError("No appropriate command avaiable for nameless parameters");

    // Second step: Check if every command is happy with the parameters assigned to it
    for (multimap<CommandlineCommand*, vector<string> >::iterator it = parameterMap.begin(); it != parameterMap.end(); ++it) {
        bool correct = (*it).first->checkParameters((*it).second);
        if (!correct)
            exitWithError("Failed to parse arguments for " + (*it).first->name() + ": " + (*it).first->errorMessage(),
                          (*it).first->name());
    }

    // Second-and-a-halfs step: Display pairs for (command,argument) if verbosity is wanted
    // TODO replace by LDEBUG
    /*
    if (_verbosity) {
        std::cout << "Verbosity output:" << std::endl;
        // First the nameless command
        std::cout << "(Nameless command,";
        for (vector<string>::iterator iter = argumentsForNameless.begin();
            iter != argumentsForNameless.end();
            ++iter)
            std::cout << " " << (*iter);
        std::cout << ")" << std::endl;

        // Then the rest
        for (multimap<CommandlineCommand*, vector<string> >::iterator it = parameterMap.begin(); it != parameterMap.end(); ++it) {
            std::cout << "(" << (*it).first->name() << ",";

            for (vector<string>::iterator iter = (*it).second.begin();
                iter != (*it).second.end();
                ++iter)
                std::cout << " " << (*iter);
            std::cout << ")" << std::endl;
        }
        std::cout << std::endl;
    }
    */

    // Third step: Execute the nameless command if there are any arguments available
    if (!argumentsForNameless.empty()) {
        bool correct = _commandForNamelessArguments->checkParameters(argumentsForNameless);

        if (correct)
            _commandForNamelessArguments->execute(argumentsForNameless);
        else
            exitWithError("One of the parameters for the nameless command was not correct");
    }

    // Fourth step: Execute the commands (this step is only done if everyone is happy up until now)
    for (multimap<CommandlineCommand*, vector<string> >::iterator it = parameterMap.begin(); it != parameterMap.end(); ++it) {
        bool correct = (*it).first->execute((*it).second);
        if (!correct)
            exitWithError("The execution for " + (*it).first->name() + " failed", (*it).first->name());
    }
}

bool CommandlineParser::addCommand(CommandlineCommand* cmd) {
    // Check, if either the name or the shortname is already present in the parser
    const bool nameValid = getCommand(cmd->name()) == nullptr;
    if (!nameValid) {
        LERROR("The name for the command '" << cmd->name() << "' already existed");
        return false;
    }
    const bool hasShortname = cmd->shortName() != "";
    const bool shortNameValid = getCommand(cmd->shortName()) != nullptr;
    if (hasShortname && !shortNameValid) {
        LERROR("The short name for the command '" << cmd->name() << "' already existed");
        return false;
    }

    // If we got this far, the names are valid
    // We don't need to check for duplicate entries as the names would be duplicate, too
    _commands.push_back(cmd);
    return true;
}

void CommandlineParser::addCommandForNamelessArguments(CommandlineCommand* cmd) {
    if (_commandForNamelessArguments != nullptr)
        delete _commandForNamelessArguments;
    _commandForNamelessArguments = cmd;
}

void CommandlineParser::displayUsage(const std::string& command, std::ostream& stream) {
    string usageString = "Usage: ";

    if (command.empty()) {
        if (_programName != "")
            usageString += _programName + " ";

        if (_commandForNamelessArguments != 0)
            usageString += _commandForNamelessArguments->usage() + " ";

        for (CommandlineCommand* it : _commands) {
            if (it)
                usageString += "\n" + it->usage() + " ";
        }
    } else {
        for (CommandlineCommand* it : _commands) {
            if (it && (it->name() == command || it->shortName() == command)) {
                usageString += "\n" + it->usage() + " ";
            }
        }
    }

    // Display via the std-out because no Logger-Prefix is wanted with the output
    stream << usageString << std::endl;
}

void CommandlineParser::displayHelp(std::ostream& stream) {
    displayUsage();
    stream << std::endl << std::endl << "Help:" << std::endl << "-----" << std::endl;

    for (CommandlineCommand* it : _commands)
        stream << it->help() << std::endl;

    exit(EXIT_FAILURE);
}

CommandlineCommand* CommandlineParser::getCommand(const std::string& shortOrLongName) {
    if (shortOrLongName == "")
        return nullptr;
    for (CommandlineCommand* it : _commands) {
        if ((it->name() == shortOrLongName) || (it->shortName() == shortOrLongName))
            return it;
    }
    return nullptr;
}

void CommandlineParser::exitWithError(const std::string& msg, const std::string& command) {
    std::cout << msg << std::endl << std::endl;
    displayUsage(command);
    exit(EXIT_FAILURE);
}

} // namespace cmdparser
} // namespace voreen
