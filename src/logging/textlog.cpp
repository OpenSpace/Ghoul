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

#include "logging/textlog.h"

#include <cstdio>

namespace {
	const std::string keyFileName = "FileName";
	const std::string keyWriteToAppend = "Append";
}

namespace ghoul {
namespace logging {


TextLog::TextLog(const std::string& filename, bool writeToAppend, bool timeStamping,
                 bool dateStamping, bool categoryStamping, bool logLevelStamping)
    : Log(timeStamping, dateStamping, categoryStamping, logLevelStamping)
    , _printFooter(writeToAppend)
{
	initialize(filename, writeToAppend);
}

TextLog::TextLog(const Dictionary& dictionary)
	: Log(dictionary)
{
	std::string fileName;
	if (dictionary.hasKeyAndValue<std::string>(keyFileName))
		dictionary.getValue(keyFileName, fileName);
	else {
		LERRORC("TextLog", "TextLog constructor did not contain a file name");
		return;
	}

	if (dictionary.hasKeyAndValue<bool>(keyWriteToAppend))
		dictionary.getValue(keyWriteToAppend, _printFooter);

	initialize(fileName, _printFooter);
}

TextLog::~TextLog() {
    if (_file != nullptr) {
        if (_printFooter)
            fputs("--------\n", _file);
        fclose(_file);
    }
}

void TextLog::initialize(const std::string& filename, bool writeToAppend) {
	const char* writeMode;
	if (writeToAppend)
		writeMode = "a";
	else
		writeMode = "w";
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
	errno_t error = fopen_s(&_file, filename.c_str(), writeMode);
	if (error != 0)
		_file = 0;
#else
	_file = fopen(filename.c_str(), writeMode);
#endif
}

void TextLog::log(LogManager::LogLevel level, const std::string& category,
                  const std::string& message)
{
    if (_file != nullptr) {
        std::string output;
        if (isDateStamping())
            output += "[" + getDateString();
        if (isTimeStamping())
            output += " | " + getTimeString() + "] ";
        else
            output += "] ";
        if (isCategoryStamping())
            output += category + " ";
        if (isLogLevelStamping())
            output += "(" + LogManager::stringFromLevel(level) + ") ";
        if (output != "")
            output += ":\t";
        output += message + '\n';
        writeLine(output);
    }
}

void TextLog::flush() {
    if (_file != nullptr)
        fflush(_file);
}

void TextLog::writeLine(const std::string& line) {
    if (_file != nullptr)
        fputs(line.c_str(), _file);
}

bool TextLog::hasValidFile() const {
    return (_file != nullptr);
}

} // namespace logging
} // namespace ghoul
