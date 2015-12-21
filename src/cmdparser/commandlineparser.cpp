/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2015                                                               *
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

#include <ghoul/misc/assert.h>
#include <format.h>

#include <algorithm>
#include <iostream>
#include <map>

using fmt::format;

namespace {

const std::string _loggerCat = "CommandlineParser";

/**
 * Extracts multiple arguments from a single list. <br>
 * If <code>count</code> is <code>-2</code>, arguments will be extracted, as long as no
 * new commands is found. <code>in[begin]</code> itself will not be extracted if this is
 * the command.<br>
 * If <code>count</code> is <code>-1</code>, the rest of the line will be extracted.<br>
 * If <code>count</code> is <code>> 0</code>, that many arguments will be extracted and
 * returned.
 */
int extractArguments(const std::vector<std::string>& in, std::vector<std::string>& out,
                     const size_t begin, const int count)
{
    int num = 0;
    if (count == -1) {
        for (size_t i = 0; (i < in.size()) && ((begin+1+i) < in.size()) ; ++i, ++num)
            out.push_back(in[begin+1+i]);
    }
    else
        if (count == -2) {
            // Extract arguments until a new command is found
            // The '-' restriction is enforced in the #addCommand method
            for (size_t i = begin; (i < in.size()) && (in[i][0] != '-'); ++i, ++num)
                out.push_back(in[i]);
        }
    else {
        for (int i = 0; (i < count) && ((begin+1+i) < in.size()) ; ++i, ++num)
            out.push_back(in[begin+1+i]);
    }
    return num;
}

} // namespace

namespace ghoul {
namespace cmdparser {
    
CommandlineParser::CommandlineException::CommandlineException(const std::string& msg)
    : RuntimeError(msg, "CommandlineParser")
{}
    
CommandlineParser::CommandlineParser(std::string programName, bool allowUnknownCommands,
                                std::string shortHelpCommand, std::string longHelpCommand)
    : _commandForNamelessArguments(nullptr)
    , _remainingArguments(std::make_shared<std::vector<std::string>>())
    , _programName(std::move(programName))
    , _allowUnknownCommands(allowUnknownCommands)
    , _shortHelpCommand(std::move(shortHelpCommand))
    , _longHelpCommand(std::move(longHelpCommand))
{}

CommandlineParser::~CommandlineParser() {
    _commands.clear();
}
    
void CommandlineParser::setAllowUnknownCommands(bool allowUnknownCommands) {
    _allowUnknownCommands = allowUnknownCommands;
}

bool CommandlineParser::allowsUnknownCommands() const {
    return _allowUnknownCommands;
}

const std::string& CommandlineParser::programPath() const {
    return _programPath;
}

std::shared_ptr<const std::vector<std::string>> CommandlineParser::setCommandLine(
    int argc, char** argv)
{
    ghoul_assert(argv, "Argv cannot be a nullptr");

    // argv[0] = program name
    // argv[i] = i-th argument
    if (argc > 0 && argv && argv[0])
        _programPath = argv[0];
    else
        _programPath = "";
    
    // Might be possible that someone calls us multiple times
    _arguments.clear();
    _remainingArguments->clear();
    
    // Just add the arguments to the vector
    for (int i = 1; i < argc; ++i)
        _arguments.push_back(argv[i]);
    
    return _remainingArguments;
}

bool CommandlineParser::execute() {
    if (_arguments.empty())
        return false;

    // There is only one argument and this is either "-h" or "--help"
    // so display the help
    if (hasOnlyHelpCommand())
        return true;

    std::vector<std::string> argumentsForNameless;

    // We store the commands and parameters in a map to be able to execute them without
    // parsing the commandline again
    std::multimap<CommandlineCommand*, std::vector<std::string>> parameterMap;

    for (size_t i = 0 ; i < _arguments.size(); ++i) {
        // In the beginning we assume that we just started the loop or finished reading
        // parameters for the last command

        // Test if the next argument is a command or a parameter for a nameless argument
        // The restriction for '-' is enforced in the #addCommand method
        if (_arguments[i][0] != '-') {
            // The rest of the commands until the next '-' are for the nameless command
            // if we have one
            if (_commandForNamelessArguments) {
                int number = extractArguments(_arguments, argumentsForNameless, i, -2);
                i += (number - 1);
            }
            else {
                // if we do not have a command for nameless arguments, but we have a place
                // to store them; we do not need to check if the nameless command is
                // available as this is done later
                if (_allowUnknownCommands) {
                    std::vector<std::string> arguments;
                    int number = extractArguments(_arguments, arguments, i, -2);
                    for (const std::string& arg : arguments)
                        _remainingArguments->push_back(arg);                    
                    i += (number - 1);
                }
            }
        }
        else {
            // We have found a command
            CommandlineCommand* currentCommand = getCommand(_arguments[i]);

            // currentCommand = nullptr, if there wasn't a command with that specific
            // name or shortName
            if (!currentCommand) {
                if (_allowUnknownCommands) {
                    // Extract the rest of the arguments
                    std::vector<std::string> arguments;
                    int number = extractArguments(_arguments, arguments, i, -2);
                    _remainingArguments->push_back(_arguments[i]);
                    for (const std::string& arg : arguments)
                        _remainingArguments->push_back(arg);
                    i += number;
                    continue;
                }
                else {
                    throw CommandlineException(format(
                        "{} is not a valid command", _arguments[i]
                    ));
                }
            }

            std::vector<std::string> parameters;
            int number = extractArguments(_arguments, parameters, i,
                                          currentCommand->argumentNumber());
            i += number;

            // don't insert if the command doesn't allow multiple calls and already is in
            // the map
            if (!currentCommand->allowsMultipleCalls() &&
                parameterMap.find(currentCommand) != parameterMap.end())
            {
                throw CommandlineException(format(
                    "{} does not allow multiple calls in a single line",
                    currentCommand->name()
                ));
            }

            parameterMap.emplace(currentCommand, parameters);
        }
    }
    // We now have all the commands with the respective parameters stored in the map and
    // all the parameters for the nameless command is available as well.

    // First step: Test, if we have nameless arguments even if we don't have a nameless
    // command. If so, bail out
    if (!argumentsForNameless.empty() && (!_commandForNamelessArguments)) {
        if (_allowUnknownCommands) {
            for (std::string arg : argumentsForNameless)
                _remainingArguments->push_back(arg);
        }
        else {
            throw CommandlineException(
                "No appropriate command available for nameless parameters"
            );
        }
    }

    // Second step: Check if every command is happy with the parameters assigned to it
	for (const auto& it : parameterMap)
        it.first->checkParameters(it.second);

    // Second-and-a-half step: Display pairs for (command,argument) in debug level
    std::stringstream s;
    for (const std::string& arg : argumentsForNameless)
        s << " " << arg;
    LDEBUG(format("(Nameless argument: {})", s.str()));
    
	for (const auto& it : parameterMap) {
        s.clear();
        for (const std::string& arg : it.second)
            s << " " << arg;
        LDEBUG(format("({}: {})", it.first->name(), s.str()));
    }

    // Third step: Execute the nameless command if there are any arguments available
    if (!argumentsForNameless.empty()) {
        _commandForNamelessArguments->checkParameters(argumentsForNameless);
        _commandForNamelessArguments->execute(argumentsForNameless);
    }

    // Fourth step: Execute the commands (this step is only done if everyone is happy up
    // until now)
	for (const auto& it : parameterMap)
        it.first->execute(it.second);
    
    // If we made it this far it means that all commands have been executed successfully
    return false;
}

bool CommandlineParser::addCommand(std::unique_ptr<CommandlineCommand> cmd) {
    ghoul_assert(cmd, "Command must not be empty");
    ghoul_assert(!getCommand(cmd->name()), "Name was previously registered");
    
    if (!cmd->shortName().empty())
        ghoul_assert(!getCommand(cmd->shortName()), "Shortname was previously registered");
    
    _commands.push_back(std::move(cmd));
    return true;
}

void CommandlineParser::addCommandForNamelessArguments(
    std::unique_ptr<CommandlineCommand> cmd)
{
    ghoul_assert(cmd, "Command must not be empty");
    _commandForNamelessArguments = std::move(cmd);
}
    
bool CommandlineParser::hasCommandForName(const std::string& name) const {
    auto it = std::find_if(
        _commands.begin(),
        _commands.end(),
        [&name](auto& command) { return command->name() == name; }
    );
    return it != _commands.end();
}
    
bool CommandlineParser::hasCommandForShortName(const std::string& shortName) const {
    auto it = std::find_if(
        _commands.begin(),
        _commands.end(),
        [&shortName](auto& command) { return command->shortName() == shortName; }
    );
    return it != _commands.end();
}

void CommandlineParser::displayUsage(const std::string& command,
                                     std::ostream& stream) const
{
    std::string usageString = "Usage: ";

    if (command.empty()) {
        if (!_programName.empty())
            usageString += _programName + " ";

        if (_commandForNamelessArguments)
            usageString += _commandForNamelessArguments->usage() + " ";

        for (auto& it : _commands) {
            if (it)
                usageString += "\n" + it->usage() + " ";
        }
    } else {
        for (auto& it : _commands) {
            if (it && (it->name() == command || it->shortName() == command))
                usageString += "\n" + it->usage() + " ";
        }
    }

    // Display via the std-out because no Logger-Prefix is wanted with the output
    stream << usageString << std::endl;
}

void CommandlineParser::displayHelp(std::ostream& stream) const {
    displayUsage();
    stream << std::endl << std::endl <<
        "Help:" << std::endl << "-----" << std::endl;

    for (auto& it : _commands)
        stream << it->help() << std::endl;
}

CommandlineCommand* CommandlineParser::getCommand(const std::string& shortOrLongName) {
    if (shortOrLongName.empty())
        return nullptr;
    for (auto& it : _commands) {
        if ((it->name() == shortOrLongName) || (it->shortName() == shortOrLongName))
            return it.get();
    }
    return nullptr;
}

bool CommandlineParser::hasOnlyHelpCommand() const {
    return (
        (_arguments.size() == 1) &&
        ((_arguments[0] == _shortHelpCommand) || (_arguments[0] == _longHelpCommand))
    );
    
}
    
} // namespace cmdparser
} // namespace ghoul
