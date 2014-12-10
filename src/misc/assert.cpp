/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014                                                                    *
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

#include <ghoul/misc/assert.h>

#include <iostream>
#include <exception>
#include <ghoul/logging/logmanager.h>
#include <algorithm>

namespace {
	const std::string padding = "    ";
	void printOptions() {
		std::cout << "(I)gnore / (A)ssertException / (E)xit: ";
	}
}

namespace ghoul {

AssertException::AssertException() : std::runtime_error("") {}

const char* AssertException::what() const GHL_NOEXCEPT {
	return "GhoulAssertException";
}

void internal_assert(
	const std::string& expression,
	const std::string& message,
	const std::string& file,
	const std::string& function,
	int line) 
{
	std::stringstream ss;
	ss << "Assertion '" << expression<<"' failed!";
	LFATALC(ss.str(),
		std::endl
		<< padding << "File:       " << file << ", line " << line << std::endl
		<< padding << "Function:   " << function << std::endl 
		<< padding << message);

#ifdef GHL_DEBUG
	std::string inputLine;
	const size_t maxIterations = 3;
	for (size_t i = 0; i < maxIterations; ++i) {

		printOptions();
		std::getline(std::cin, inputLine);

		// Transform to lower case
		std::transform(inputLine.begin(), inputLine.end(), inputLine.begin(), ::tolower);

		if (inputLine == "i") {
			break;
		}
		else if (inputLine == "a") {
			throw AssertException();
			break; // Debuggers can continue from throw
		}
		else if (inputLine == "e") {
			exit(EXIT_FAILURE);
		}

		// Fallthrough if no relevant option is selected
		if(i == maxIterations-1)
			throw AssertException();
	}
#else
	throw AssertException();
#endif
}

} // namespace ghoul

