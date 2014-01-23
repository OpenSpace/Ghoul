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

#include <map>
#include <stdlib.h>

using std::vector;
using std::multimap;
using std::pair;
using std::string;

namespace {

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
  : commandForNamelessArguments_(0)
  , programName_(programName)
  , verbosity_(false)
{}

CommandlineParser::~CommandlineParser() {
    for (CommandlineCommand* i : commands_)
        delete *i;
    commands_.clear();
}

std::string CommandlineParser::getProgramPath() const {
    return programPath_;
}

void CommandlineParser::setVerbosity(const bool verbosity) {
    verbosity_ = verbosity;
}

void CommandlineParser::setCommandLine(int argc, char** argv) {
    // argv[0] = program name
    // argv[i] = i-th argument
    if (argc > 0 && argv && argv[0])
        programPath_ = argv[0];
    else
        programPath_ = "";

    // Might be possible that someone calls us multiple times
    arguments_.clear();

    // Just add the arguments to the vector
    for (int i = 1; i < argc; ++i)
        arguments_.push_back(argv[i]);
}

void CommandlineParser::setCommandLine(std::vector<std::string> arguments) {
    // argv[0] = program name
    // argv[i] = i-th argument
    if (arguments.size() > 0)
        programPath_ = arguments[0];
    else
        programPath_ = "";

    // Might be possible that someone calls us multiple times
    arguments_.clear();

    // Just add the arguments to the vector
    for (size_t i = 1; i < arguments.size(); ++i)
        arguments_.push_back(arguments[i]);
}

void CommandlineParser::execute() {
    // There is only one argument and this is either "-h" or "--help" ; so display the help
    if ((arguments_.size() == 1) && ((arguments_[0] == "-h") || (arguments_[0] == "--help")))
        displayHelp();

    vector<string> argumentsForNameless;

    // We store the commands and parameters in a map to be able to execute them without parsing the
    // commandline again
    multimap<CommandlineCommand*, vector<string> > parameterMap;

    for (size_t i = 0 ; i < arguments_.size(); ++i) {
        // In the beginning we assume that we just started the loop or finished reading parameters for
        // one command

        // So test if the next argument is a command or a parameter for a nameless argument
        if (arguments_[i][0] != '-') {
            int number = extractArguments(arguments_, &argumentsForNameless, i, -2);
            i += (number - 1);
        }
        else {
            // We have found a command
            CommandlineCommand* currentCommand = getCommand(arguments_[i]);

            // currentCommand = 0, if there wasn't a command with that specific name or shortName
            if (currentCommand == 0)
                exitWithError(arguments_[i] + " is not a valid command");

            vector<string> parameters;
            int number = extractArguments(arguments_, &parameters, i, currentCommand->getArgumentNumber());
            i += number;

            // don't insert if the command doesn't allow multiple calls and already is in the map
            if (!currentCommand->getAllowMultipleCalls() && parameterMap.find(currentCommand) != parameterMap.end())
                exitWithError(currentCommand->getName() + " doesn't allow multiple calls in a single line",
                              currentCommand->getName());

            parameterMap.insert(pair<Command*, vector<string> >(currentCommand, parameters));
        }
    }
    // We now have all the commands with the respective parameters stored in the map and all the parameters for
    // the nameless command is avaiable as well.

    // First step: Test, if we have nameless arguments even if we don't have a nameless command. Otherwise bail out
    if (!argumentsForNameless.empty() && (commandForNamelessArguments_ == 0))
        exitWithError("No appropriate command avaiable for nameless parameters");

    // Second step: Check if every command is happy with the parameters assigned to it
    for (multimap<CommandlineCommand*, vector<string> >::iterator it = parameterMap.begin(); it != parameterMap.end(); ++it) {
        bool correct = (*it).first->checkParameters((*it).second);
        if (!correct)
            exitWithError("Failed to parse arguments for " + (*it).first->getName() + ": " + (*it).first->getErrorMessage(),
                          (*it).first->getName());
    }

    // Second-and-a-halfs step: Display pairs for (command,argument) if verbosity is wanted
    if (verbosity_) {
        std::cout << "Verbosity output:" << std::endl;
        // First the nameless command
        std::cout << "(Nameless command,";
        for (vector<string>::iterator iter = argumentsForNameless.begin();
            iter != argumentsForNameless.end();
            ++iter)
            std::cout << " " << (*iter);
        std::cout << ")" << std::endl;

        // Then the rest
        for (multimap<Command*, vector<string> >::iterator it = parameterMap.begin(); it != parameterMap.end(); ++it) {
            std::cout << "(" << (*it).first->getName() << ",";

            for (vector<string>::iterator iter = (*it).second.begin();
                iter != (*it).second.end();
                ++iter)
                std::cout << " " << (*iter);
            std::cout << ")" << std::endl;
        }
        std::cout << std::endl;
    }

    // Third step: Execute the nameless command if there are any arguments available
    if (!argumentsForNameless.empty()) {
        bool correct = commandForNamelessArguments_->checkParameters(argumentsForNameless);

        if (correct)
            commandForNamelessArguments_->execute(argumentsForNameless);
        else
            exitWithError("One of the parameters for the nameless command was not correct");
    }

    // Fourth step: Execute the commands (this step is only done if everyone is happy up until now)
    for (multimap<CommandlineCommand*, vector<string> >::iterator it = parameterMap.begin(); it != parameterMap.end(); ++it) {
        bool correct = (*it).first->execute((*it).second);
        if (!correct)
            exitWithError("The execution for " + (*it).first->getName() + " failed", (*it).first->getName());
    }
}

void CommandlineParser::addCommand(CommandlineCommand* cmd) throw (VoreenException) {
    // Check, if either the name or the shortname is already assigned in the parser
    if (getCommand(cmd->getName()) == 0 && ((getCommand(cmd->getShortName()) == 0) || (cmd->getShortName() == "")))
        commands_.push_back(cmd);
    else {
        // One of the names existed, so throw an exception
        throw VoreenException(cmd->getName() + " or " + cmd->getShortName() + " was already assigned in this parser");
    }
}

void CommandlineParser::addCommandForNamelessArguments(CommandlineCommand* cmd) throw (VoreenException) {
    if (commandForNamelessArguments_ == 0) {
        // The command for nameless arguments wasn't already set
        if (cmd->getAllowMultipleCalls())
            throw VoreenException("Nameless command mustn't be allowed to be called multiple times");
        else
            commandForNamelessArguments_ = cmd;
    }
    else
        throw VoreenException("There was already a nameless command assigned to this parser");
}

void CommandlineParser::displayUsage(const std::string& command) {
    string usageString = "Usage: ";

    if (command.empty()) {
        if (programName_ != "")
            usageString += programName_ + " ";

        if (commandForNamelessArguments_ != 0)
            usageString += commandForNamelessArguments_->usage() + " ";

        for (CommandlineCommand* it : commands_)
            if (*it)
                usageString += "\n" + (*it)->usage() + " ";
        }
    } else {
        for (CommandlineCommand* it : commands_) {
            if (*it && ((*it)->getName() == command || (*it)->getShortName() == command)) {
                usageString += "\n" + (*it)->usage() + " ";
            }
        }
    }

    // Display via the std-out because no Logger-Prefix is wanted with the output
    std::cout << usageString << std::endl;
}

void CommandlineParser::displayHelp() {
    displayUsage();
    std::cout << std::endl << std::endl << "Help:" << std::endl << "-----" << std::endl;

    for (CommandlineCommand* it : commands_)
        std::cout << (*it)->help() << std::endl;

    exit(EXIT_FAILURE);
}

CommandlineCommand* CommandlineParser::getCommand(const std::string& shortOrLongName) {
    for (CommandlineCommand* it : commands_) {
        if (((*it)->getName() == shortOrLongName) || ((*it)->getShortName() == shortOrLongName))
            return (*it);
    }
    return 0;
}

void CommandlineParser::exitWithError(const std::string& msg, const std::string& command) {
    std::cout << msg << std::endl << std::endl;
    displayUsage(command);
    exit(EXIT_FAILURE);
}

} // namespace cmdparser
} // namespace voreen
