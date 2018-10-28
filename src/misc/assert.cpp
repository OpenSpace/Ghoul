/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2018                                                               *
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

#include <ghoul/fmt.h>
#include <ghoul/misc/stacktrace.h>
#include <algorithm>
#include <iostream>
#include <vector>

namespace {
    constexpr const bool AlwaysAssert = false;

    std::vector<std::string> PermanentlyIgnoredAsserts;

    std::string hashing(const std::string& file, int line) {
        return file + "||" + std::to_string(line);
    }

    void addPermanentlyIgnoredAssert(const std::string& file, int line) {
        PermanentlyIgnoredAsserts.emplace_back(hashing(file, line));
    }

    bool isPermanentlyIgnored(const std::string& file, int line) {
        auto it = std::find(
            PermanentlyIgnoredAsserts.begin(),
            PermanentlyIgnoredAsserts.end(),
            hashing(file, line)
        );
        return it != PermanentlyIgnoredAsserts.end();
    }
} // namespace

namespace ghoul {

AssertionException::AssertionException(std::string exp, std::string msg,
                                         std::string file, std::string func, int line)
    : std::runtime_error(fmt::format("{}, \"{}\" ({}:{} ({})",
        std::move(exp), std::move(msg), std::move(file), line, std::move(func)
    ))
{}

void internal_assert(std::string expression, std::string message, std::string file,
                                                           std::string function, int line)
{
    if (!isPermanentlyIgnored(file, line)) {
        const std::string padding = "    ";

        std::cerr << std::endl
            << padding << "File:       " << file << ", line " << line << std::endl
            << padding << "Function:   " << function << std::endl
            << padding << "Assertion:  " << expression
            << padding << message << std::endl;

        if (AlwaysAssert) {
#ifdef _MSC_VER
            __debugbreak();
#endif // _MSC_VER

            throw AssertionException(
                std::move(expression),
                std::move(message),
                std::move(file),
                std::move(function),
                line
            );
        }

        while (true) {
            std::cerr << "(I)gnore / Ignore (P)ermanently / (A)ssertion / (S)tacktrace "
                         "/ (E)xit: ";
            std::string inputLine;
            std::getline(std::cin, inputLine);

            // Transform to lower case
            std::transform(
                inputLine.begin(),
                inputLine.end(),
                inputLine.begin(),
                [](char v) { return static_cast<char>(tolower(v)); }
            );

            if (inputLine == "i") {
                break;
            }
            else if (inputLine == "p") {
                addPermanentlyIgnoredAssert(file, line);
                break;
            }
            else if (inputLine == "a") {
#ifdef _MSC_VER
                __debugbreak();
#endif // _MSC_VER

                throw AssertionException(
                    std::move(expression),
                    std::move(message),
                    std::move(file),
                    std::move(function),
                    line
                );
            }
            else if (inputLine == "s") {
                std::vector<std::string> stackTrace = ghoul::stackTrace();

                std::cerr << '\n';
                for (size_t i = 0; i < stackTrace.size(); ++i) {
                    std::cerr << i << ": " << stackTrace[i] << '\n';
                }
                std::cerr << '\n';

                continue;
            }
            else if (inputLine == "e") {
                exit(EXIT_FAILURE);
            }
        }
    }
}

} // namespace ghoul
