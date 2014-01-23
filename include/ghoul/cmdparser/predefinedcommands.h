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

#ifndef VRN_PREDEFINEDCOMMAND_H
#define VRN_PREDEFINEDCOMMAND_H

#include "voreen/core/utils/cmdparser/singlecommand.h"

namespace voreen {

/**
 * This command is used for toggling a benchmark in the application.
 * It simply calls the SingleCommandZeroArguments ctor and sets the values:<br>
 * name          = --benchmark<br>
 * shortName     = -b<br>
 * \sa Command, SingleCommandZeroArguments, CommandlineParser
 */
class Command_Benchmark : public SingleCommandZeroArguments {
public:
    Command_Benchmark(bool* ptr) : SingleCommandZeroArguments(ptr, "--benchmark", "-b") {}
};

//-----------------------------------------------------------------------------

/**
 * This command is used for setting a width.
 * It simply calls the SingleCommandOneArgument ctor and sets the values:<br>
 * name          = --width<br>
 * shortName     = -w<br>
 * parameterList = &lt;value&gt;
 * \sa Command, SingleCommandOneArgument, CommandlineParser
 */
class Command_Width : public SingleCommand<int> {
public:
    Command_Width(int* ptr) : SingleCommand<int>(ptr, "--width", "-w", "Set the width of a window") {}
};

//-----------------------------------------------------------------------------

/**
 * This command is used for setting a height.
 * It simply calls the SingleCommandOneArgument ctor and sets the values:<br>
 * name          = --height<br>
 * shortName     = -h<br>
 * parameterList = &lt;value&gt;
 * \sa Command, SingleCommandOneArgument, CommandlineParser
 */
class Command_Height : public SingleCommand<int> {
public:
    Command_Height(int* ptr) : SingleCommand<int>(ptr, "--height", "-h", "Set the height of a window") {}
};

//-----------------------------------------------------------------------------

/**
 * This command is used for setting a size.
 * It simply calls the SingleCommandTwoArguments ctor and sets the values:
 * name          = --size<br>
 * shortName     = -s<br>
 * parameterList = &lt;value1 value2&gt;
 * \sa Command, SingleCommandTwoArguments, CommandlineParser
 */
class Command_Size : public SingleCommand<int> {
public:
    Command_Size(int* ptr1, int* ptr2) : SingleCommand<int>(ptr1, ptr2, "--size", "-s", "Set the size of a window") {}
};

//-----------------------------------------------------------------------------

/**
 * This command is used for loading a network file.
 * It simply calls the SingleCommandOneArgument ctor and sets the values:<br>
 * name          = --network<br>
 * shortName     = -n<br>
 * parameterList = &lt;network file&gt;
 * \sa Command, SingleCommandOneArgument, CommandlineParser
 */
class Command_LoadNetworkSingle : public SingleCommand<std::string> {
public:
    Command_LoadNetworkSingle(std::string* ptr) : SingleCommand<std::string>(ptr, "--network", "-n", "Loads a single network", "<network file>") {}
};

//-----------------------------------------------------------------------------

/**
 * This command is used for loading a network file but can be called multiple times
 * It simply calls the SingleCommandOneArgument ctor and sets the values:<br>
 * name          = --network<br>
 * shortName     = -n<br>
 * parameterList = &lt;network file&gt;
 * \sa Command, SingleCommandOneArgument, CommandlineParser
 */
class Command_LoadNetworkMultiple : public MultipleCommand<std::string> {
public:
    Command_LoadNetworkMultiple(std::vector<std::string>* ptr) : MultipleCommand<std::string>(ptr, "--network", "-n", "Loads a single network", "<network file>") {}
};

//-----------------------------------------------------------------------------

/**
 * This command is used for loading a dataset file.
 * It simply calls the SingleCommandOneArgument ctor and sets the values:<br>
 * name          = --dataset<br>
 * shortName     = -d<br>
 * parameterList = &lt;dataset file&gt;
 * \sa Command, SingleCommandOneArgument, CommandlineParser
 */
class Command_LoadDatasetSingle : public SingleCommand<std::string> {
public:
    Command_LoadDatasetSingle(std::string* ptr) : SingleCommand<std::string>(ptr, "--dataset", "-d", "Loads a single dataset", "<dataset file>") {}
};

//-----------------------------------------------------------------------------

/**
 * This command is used for loading a dataset file.
 * It simply calls the SingleCommandOneArgument ctor and sets the values:<br>
 * name          = --dataset<br>
 * shortName     = -d<br>
 * parameterList = &lt;dataset file&gt;
 * \sa Command, SingleCommandOneArgument, CommandlineParser
 */
class Command_LoadDatasetMultiple : public MultipleCommand<std::string> {
public:
    Command_LoadDatasetMultiple(std::vector<std::string>* ptr) : MultipleCommand<std::string>(ptr, "--dataset", "-d", "Loads a single dataset", "<dataset file>") {}
};

//-----------------------------------------------------------------------------

/**
 * This command is used for loading a transfer function.
 * It simply calls the SingleCommandOneArgument ctor and sets the values:<br>
 * name          = --transferfunction<br>
 * shortName     = -tf<br>
 * parameterList = &lt;transfer function&gt;
 * \sa Command, SingleCommandOneArgument, CommandlineParser
 */
class Command_LoadTransferFunction : public SingleCommand<std::string> {
public:
    Command_LoadTransferFunction(std::string* ptr) : SingleCommand<std::string>(ptr, "--transferfunction", "-tf", "Loads a single transfer function", "<transfer function>") {}
};

//-----------------------------------------------------------------------------

/**
 * This command is used for loading a segmentation.
 * It simply calls the SingleCommandOneArgument ctor and sets the values:<br>
 * name          = --segmentation<br>
 * shortName     = -s<br>
 * parameterList = &lt;segmentation file&gt;
 * \sa Command, SingleCommandOneArgument, CommandlineParser
 */
class Command_LoadSegmentation : public SingleCommand<std::string> {
public:
    Command_LoadSegmentation(std::string* ptr) : SingleCommand<std::string>(ptr, "--segmentation", "-s", "Loads a segmentation file", "<segmentation file>") {}
};

//-----------------------------------------------------------------------------

/**
 * This command is used for pointing out a logfile for writing.
 * It simply calls the SingleCommandOneArgument ctor and sets the values:<br>
 * name = --logfile
 * parameterList = &lt;filename&gt;<br>
 * infoText = enable logging to a html file
 * \sa Command, SingleCommandOneArgument, CommandlineParser
 */
class Command_LogFile : public SingleCommand<std::string> {
public:
    Command_LogFile(std::string* ptr) : SingleCommand<std::string>(ptr, "--logfile", "", "enable logging to a html file", "<filename>") {}
};

} // namespace

#endif // VRN_PREDEFINEDCOMMAND_H
