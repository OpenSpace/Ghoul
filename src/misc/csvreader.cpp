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

#include <ghoul/misc/csvreader.h>

#include <ghoul/fmt.h>
#include <ghoul/misc/assert.h>
#include <ghoul/misc/exception.h>
#include <ghoul/misc/misc.h>
#include <algorithm>
#include <fstream>

namespace {
    std::vector<std::vector<std::string>> internalLoadCSV(std::ifstream& file,
                                     const std::vector<int>& indices = std::vector<int>())
    {
        ghoul_assert(file.good(), "File handle should be good");

        std::vector<std::vector<std::string>> result;

        std::string line;
        while (std::getline(file, line)) {
            std::vector<std::string> lineValues = ghoul::tokenizeString(line, ',');
            // If indices have been specified, we use those to reorganize and filter the
            // line values
            if (!indices.empty()) {
                std::vector<std::string> copy = lineValues;
                lineValues.clear();
                for (int idx : indices) {
                    lineValues.push_back(copy[idx]);
                }
            }
            result.push_back(std::move(lineValues));
        }

        return result;
    }
} // namespace

namespace ghoul {

std::vector<std::vector<std::string>> loadCSVFile(const std::string& fileName,
                                                  bool includeFirstLine)
{
    ghoul_assert(!fileName.empty(), "fileName must not be empty");

    std::ifstream file;
    file.exceptions(std::ifstream::badbit);
    file.open(fileName);

    // Just skip over the first line if we don't want to include it
    if (!includeFirstLine) {
        std::string line;
        std::getline(file, line);
    }

    return internalLoadCSV(file);
}

std::vector<std::vector<std::string>> loadCSVFile(const std::string& fileName,
                                                  const std::vector<std::string>& columns,
                                                  bool includeFirstLine)
{
    ghoul_assert(!fileName.empty(), "fileName must not be empty");
    ghoul_assert(!columns.empty(), "columns must not be empty");

    std::ifstream file;
    file.exceptions(std::ifstream::badbit);
    file.open(fileName);

    // Get the file line that contains the column names
    std::string line;
    std::getline(file, line);
    std::vector<std::string> elements = ghoul::tokenizeString(line, ',');
    if (elements.empty()) {
        throw ghoul::RuntimeError(
            fmt::format("CSV file {} did not contain any lines", fileName)
        );
    }

    std::vector<int> indices(columns.size());

    std::transform(
        columns.begin(),
        columns.end(),
        indices.begin(),
        [elements, &fileName](const std::string& column) {
            auto it = std::find(elements.begin(), elements.end(), column);
            if (it == elements.end()) {
                throw ghoul::RuntimeError(fmt::format(
                    "CSV file {} did not contain the requested key {}", fileName, column
                ));
            }

            return static_cast<int>(std::distance(elements.begin(), it));
        }
    );

    // Reset the file stream if we want to include the first line
    if (includeFirstLine) {
        file.seekg(0);
    }

    return internalLoadCSV(file, indices);
}

std::vector<std::vector<std::string>> loadCSVFile(const std::string& fileName,
                                                  const std::vector<int>& columns,
                                                  bool includeFirstLine)
{
    ghoul_assert(!fileName.empty(), "fileName must not be empty");
    ghoul_assert(!columns.empty(), "columns must not be empty");

    std::ifstream file;
    file.exceptions(std::ifstream::badbit);
    file.open(fileName);

    // Just skip over the first line if we don't want to include it
    if (!includeFirstLine) {
        std::string line;
        std::getline(file, line);
    }

    return internalLoadCSV(file, columns);
}

} // namespace ghoul
