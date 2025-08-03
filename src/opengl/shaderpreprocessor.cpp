/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2025                                                               *
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

#include <ghoul/opengl/shaderpreprocessor.h>

#include <ghoul/filesystem/filesystem.h>
#include <ghoul/glm.h>
#include <ghoul/misc/dictionary.h>
#include <ghoul/misc/stringhelper.h>
#include <ghoul/systemcapabilities/openglcapabilitiescomponent.h>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <string>
#include <sstream>
#include <utility>

namespace {
    constexpr std::string_view Ws = " \n\r\t";

    struct ShaderPreprocessorError : public ghoul::RuntimeError {
        explicit ShaderPreprocessorError(std::string msg)
            : RuntimeError(std::move(msg), "ShaderPreprocessor")
        {}
    };

    std::string glslVersionString() {
        int versionMajor = 0;
        int versionMinor = 0;
        int profileMask = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
        glGetIntegerv(GL_MINOR_VERSION, &versionMinor);
        glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profileMask);

        const ContextProfileMask cpm = ContextProfileMask(profileMask);
        const bool isCore = cpm == ContextProfileMask::GL_CONTEXT_CORE_PROFILE_BIT;
        const bool isCompatibility =
            cpm == ContextProfileMask::GL_CONTEXT_COMPATIBILITY_PROFILE_BIT;

        ghoul_assert(
            isCore || isCompatibility,
            "OpenGL context is neither core nor compatibility"
        );

        std::string_view type =
            isCore ? " core" : (isCompatibility ? " compatibility" : "");
        return std::format("#version {}{}0 {}", versionMajor, versionMinor, type);
    }

    bool hasKeyRecursive(const ghoul::Dictionary& dictionary, std::string_view key) {
        const size_t dotPos = key.find('.');
        if (dotPos == std::string_view::npos) {
            return dictionary.hasKey(key);
        }

        const std::string_view before = key.substr(0, dotPos);
        if (dictionary.hasKey(before)) {
            const ghoul::Dictionary d = dictionary.value<ghoul::Dictionary>(before);
            const std::string_view after = key.substr(dotPos + 1);
            return hasKeyRecursive(d, after);
        }
        else {
            return false;
        }
    }

    template <typename T>
    bool hasValueRecursive(const ghoul::Dictionary& dictionary, std::string_view key) {
        const size_t dotPos = key.find('.');
        if (dotPos == std::string_view::npos) {
            return dictionary.hasValue<T>(key);
        }

        const std::string_view before = key.substr(0, dotPos);
        if (dictionary.hasValue<ghoul::Dictionary>(before)) {
            const ghoul::Dictionary d = dictionary.value<ghoul::Dictionary>(before);
            const std::string_view after = key.substr(dotPos + 1);
            return hasValueRecursive<T>(d, after);
        }
        else {
            return false;
        }
    }

    template <typename T>
    T valueRecursive(const ghoul::Dictionary& dictionary, std::string_view key) {
        const size_t dotPos = key.find('.');
        if (dotPos == std::string_view::npos) {
            return dictionary.value<T>(key);
        }

        const std::string_view before = key.substr(0, dotPos);
        const std::string_view after = key.substr(dotPos + 1);
        const ghoul::Dictionary d = dictionary.value<ghoul::Dictionary>(before);
        return valueRecursive<T>(d, after);
    }

    std::tuple<int, int, ghoul::Dictionary> parseRange(const std::string& key) {
        constexpr std::string_view TwoDots = "..";

        const size_t minEnd = key.find(TwoDots);
        if (minEnd == std::string::npos) {
            throw ShaderPreprocessorError(std::format("Expected '..' in range. {}", key));
        }
        const int minimum = std::stoi(key.substr(0, minEnd));

        const size_t maxStart = minEnd + 2;
        const size_t maxEnd = key.size();
        const int maximum = std::stoi(key.substr(maxStart, maxEnd - maxStart));

        // Create all the elements in the dictionary
        ghoul::Dictionary dictionary;
        for (int i = 0; i <= maximum - minimum; i++) {
            dictionary.setValue(std::to_string(i + 1), std::to_string(minimum + i));
        }

        // Everything went well. Write over min and max
        return { minimum, maximum, dictionary };
    }

    class Env {
    public:
        explicit Env(ghoul::Dictionary dictionary);

        void processFile(const std::filesystem::path& path);
        std::vector<std::filesystem::path> includedFiles() const;
        std::string finalize();

    private:
        struct Input {
            std::ifstream& stream;
            const std::filesystem::path file;
            const std::string indentation;
            unsigned int lineNumber = 1;
        };

        struct ForStatement {
            unsigned int inputIndex;
            unsigned int lineNumber;
            unsigned int streamPos;

            std::string keyName;
            std::string valueName;
            std::string dictionary;

            static constexpr size_t EmptyLoop = std::numeric_limits<size_t>::max();
            size_t currentIteration;
        };

        bool parseFor(const std::string& line);
        bool parseEndFor(const std::string& line);
        bool parseInclude(std::string_view line);
        bool parseVersion(const std::string& line);
        bool parseOs(const std::string& line);

        void substituteLine(std::string& line) const;
        std::string substitute(std::string_view in) const;
        std::string resolveAlias(std::string_view in) const;

        void addLineNumber();
        std::string debugString() const;


        std::stringstream _output;
        std::vector<Input> _inputs;
        std::vector<std::map<std::string, std::string>> _scopes;
        std::vector<ForStatement> _forStatements;
        std::string _indentation;

        std::vector<std::filesystem::path> _includedFiles;
        ghoul::Dictionary _dictionary;
    };

    Env::Env(ghoul::Dictionary dictionary)
        : _dictionary(std::move(dictionary))
    {}

    void Env::processFile(const std::filesystem::path& path) {
        ghoul_assert(!path.empty(), "Path must not be empty");
        ghoul_assert(std::filesystem::is_regular_file(path), "Path must exist");

        auto it = std::find(_includedFiles.begin(), _includedFiles.end(), path);
        if (it == _includedFiles.end()) {
            _includedFiles.push_back(path);
        }

        std::ifstream stream = std::ifstream(path, std::ifstream::binary);
        if (!stream.good()) {
            throw ShaderPreprocessorError(std::format("Error loading file '{}'", path));
        }

        const std::string prevIndent = !_inputs.empty() ? _inputs.back().indentation : "";
        _inputs.emplace_back(stream, path, prevIndent + _indentation);
        if (_inputs.size() > 1) {
            addLineNumber();
        }


        // Parse the individual lines
        while (true) {
            Env::Input& input = _inputs.back();
            std::string line;
            if (!ghoul::getline(input.stream, line)) {
                break;
            }
            input.lineNumber++;

            // Trim away any whitespaces in the start and end of the line
            const size_t startPos = line.find_first_not_of(Ws);
            _indentation = line.substr(0, startPos);
            ghoul::trimWhitespace(line);

            bool isSpecialLine = parseEndFor(line); // #endfor

            // Check for an empty for loop. This can happen when the loop variables is
            // evaluated to be never true
            const bool isInsideEmptyForStatement =
                !_forStatements.empty() &&
                (_forStatements.back().currentIteration == ForStatement::EmptyLoop);
            if (isInsideEmptyForStatement) {
                continue;
            }

            // Replace all #{<name>} strings with data from <name> in dictionary
            substituteLine(line);

            if (!isSpecialLine) {
                isSpecialLine |=
                    parseVersion(line) ||    // #version __CONTEXT__
                    parseOs(line) ||         // #define __OS__
                    parseInclude(line) ||    // #include
                    parseFor(line);          // #for <key>, <value> in <dictionary>
            }

            if (!isSpecialLine) {
                // Write GLSL code to output
                _output << std::format("{}{}{}\n", input.indentation, _indentation, line);
            }
        }


        if (!_forStatements.empty()) {
            const ForStatement& forStatement = _forStatements.back();
            if (forStatement.inputIndex + 1 >= _inputs.size()) {
                const int inputIndex = forStatement.inputIndex;

                throw ShaderPreprocessorError(std::format(
                    "Unexpected end of file. Still processing #for loop from {}:{}. {}",
                    _inputs[inputIndex].file, forStatement.lineNumber, debugString()
                ));
            }
        }

        _inputs.pop_back();

        if (!_inputs.empty()) {
            addLineNumber();
        }
    }

    void Env::addLineNumber() {
        const std::filesystem::path filename = _inputs.back().file;
        auto it = std::find(_includedFiles.begin(), _includedFiles.end(), filename);
        ghoul_assert(it != _includedFiles.end(), "File not in included files");

        std::string_view includeSep;
    #ifndef __APPLE__
        // Sofar, only Nvidia on Windows supports empty statements in the shader
        using Vendor = ghoul::systemcapabilities::OpenGLCapabilitiesComponent::Vendor;
        if (OpenGLCap.gpuVendor() == Vendor::Nvidia) {
            includeSep = "; // preprocessor add semicolon to isolate error messages";
        }
    #endif // __APPLE__

        ptrdiff_t number = std::distance(_includedFiles.begin(), it);
        _output << std::format(
            "{}\n#line {} {} // {}\n",
            includeSep, _inputs.back().lineNumber, number, filename
        );
    }

    std::string Env::debugString() const {
        if (_inputs.empty()) {
            return "";
        }

        const Env::Input& input = _inputs.back();
        return std::format("{}: {}", input.file, input.lineNumber);
    }

    std::vector<std::filesystem::path> Env::includedFiles() const {
        return _includedFiles;
    }

    std::string Env::finalize() {
        if (!_forStatements.empty()) {
            throw ShaderPreprocessorError(std::format(
                "Unexpected end of file in the middle of expanding #for statement. {}",
                debugString()
            ));
        }

        if (!_scopes.empty()) {
            throw ShaderPreprocessorError(std::format(
                "Unexpected end of file. {}", debugString()
            ));
        }

        return _output.str();
    }

    void Env::substituteLine(std::string& line) const {
        while (true) {
            const size_t begin = line.rfind("#{");
            if (begin == std::string::npos) {
                break;
            }
            const size_t length = line.substr(begin + 2).find('}');
            if (length == std::string::npos) {
                throw ShaderPreprocessorError(std::format(
                    "Could not parse line. {}", debugString()
                ));
            }

            const std::string in = line.substr(begin + 2, length);
            const std::string out = substitute(in);

            const std::string first = line.substr(0, begin);
            const std::string last = line.substr(begin + 2 + length + 1);

            line = std::format("{}{}{}", first, out, last);
        }
    }

    std::string Env::resolveAlias(std::string_view in) const {
        std::string beforeDot;
        std::string afterDot;
        if (const size_t firstDotPos = in.find('.');  firstDotPos != std::string::npos) {
            beforeDot = in.substr(0, firstDotPos);
            afterDot = in.substr(firstDotPos);
        }
        else {
            beforeDot = in;
            afterDot = "";
        }

        // Resolve only part before dot
        for (const std::map<std::string, std::string>& scope :
             std::ranges::reverse_view(_scopes))
        {
            if (auto it = scope.find(beforeDot);  it != scope.end()) {
                beforeDot = it->second;
                break;
            }
        }

        std::string res = beforeDot + afterDot;
        if (!afterDot.empty() && !hasKeyRecursive(_dictionary, res)) {
            throw ShaderPreprocessorError(std::format(
                "Could not resolve variable '{}'. {}", in, debugString()
            ));
        }

        return res;
    }

    std::string Env::substitute(std::string_view in) const {
        std::string resolved = resolveAlias(in);

        if (resolved.starts_with('"') && resolved.ends_with('"')) {
            return resolved.substr(1, resolved.length() - 2);
        }
        else if (hasValueRecursive<bool>(_dictionary, resolved)) {
            return std::to_string(valueRecursive<bool>(_dictionary, resolved));
        }
        else if (hasValueRecursive<std::string>(_dictionary, resolved)) {
            return valueRecursive<std::string>(_dictionary, resolved);
        }
        else if (hasValueRecursive<int>(_dictionary, resolved)) {
            return std::to_string(valueRecursive<int>(_dictionary, resolved));
        }
        else if (hasValueRecursive<double>(_dictionary, resolved)) {
            return std::to_string(valueRecursive<double>(_dictionary, resolved));
        }
        else if (hasValueRecursive<glm::ivec2>(_dictionary, resolved)) {
            glm::ivec2 vec = valueRecursive<glm::ivec2>(_dictionary, resolved);
            return std::format("ivec2({},{})", vec.x, vec.y);
        }
        else if (hasValueRecursive<glm::ivec3>(_dictionary, resolved)) {
            glm::ivec3 vec = valueRecursive<glm::ivec3>(_dictionary, resolved);
            return std::format("ivec3({},{},{})", vec.x, vec.y, vec.z);
        }
        else if (hasValueRecursive<glm::ivec4>(_dictionary, resolved)) {
            glm::ivec4 vec = valueRecursive<glm::ivec4>(_dictionary, resolved);
            return std::format("ivec4({},{},{},{})", vec.x, vec.y, vec.z, vec.w);
        }
        else if (hasValueRecursive<glm::dvec2>(_dictionary, resolved)) {
            glm::dvec2 vec = valueRecursive<glm::dvec2>(_dictionary, resolved);
            return std::format("dvec2({},{})", vec.x, vec.y);
        }
        else if (hasValueRecursive<glm::dvec3>(_dictionary, resolved)) {
            glm::dvec3 vec = valueRecursive<glm::dvec3>(_dictionary, resolved);
            return std::format("dvec3({},{},{})", vec.x, vec.y, vec.z);
        }
        else if (hasValueRecursive<glm::dvec4>(_dictionary, resolved)) {
            glm::dvec4 vec = valueRecursive<glm::dvec4>(_dictionary, resolved);
            return std::format("dvec4({},{},{},{})", vec.x, vec.y, vec.z, vec.w);
        }

        throw ShaderPreprocessorError(std::format(
            "'{}' was resolved to '{}' which is a type that is not supported. {}",
            in, resolved, debugString()
        ));
    }

    bool Env::parseInclude(std::string_view line) {
        constexpr std::string_view IncludeString = "#include";

        if (!line.starts_with(IncludeString)) {
            return false;
        }

        const size_t p1 = line.find_first_not_of(Ws, IncludeString.size());
        if (p1 == std::string::npos) {
            throw ShaderPreprocessorError(std::format(
                "Expected file path after #include. {}", debugString()
            ));
        }

        if (line[p1] == '\"') {
            const size_t p2 = line.find_first_of('\"', p1 + 1);
            if (p2 == std::string::npos) {
                throw ShaderPreprocessorError(std::format(
                    "Expected \" {}", debugString()
                ));
            }

            const size_t includeLength = p2 - p1 - 1;
            const std::string_view includeFilename = line.substr(p1 + 1, includeLength);
            std::filesystem::path includeFilepath =
                _inputs.back().file.parent_path() / includeFilename;

            // Resolve the include paths if this default includeFilename does not exist
            if (std::filesystem::is_regular_file(includeFilepath)) {
                processFile(includeFilepath);
                return true;
            }

            for (const std::filesystem::path& path :
                 ghoul::opengl::ShaderPreprocessor::includePaths())
            {
                includeFilepath = path / includeFilename;
                if (std::filesystem::is_regular_file(includeFilepath)) {
                    processFile(includeFilepath);
                    return true;
                }
            }

            // Our last chance is that the include file is an absolute path
            const bool found = std::filesystem::is_regular_file(includeFilename);
            if (found) {
                processFile(includeFilename);
                return true;
            }

            throw ShaderPreprocessorError(std::format(
                "Could not resolve file path for include file '{}'", includeFilepath
            ));
        }
        else if (line.at(p1) == '<') {
            const size_t p2 = line.find_first_of('>', p1 + 1);
            if (p2 == std::string::npos) {
                throw ShaderPreprocessorError(std::format(
                    "Expected >. {}", debugString()
                ));
            }

            const size_t includeLen = p2 - p1 - 1;
            const std::filesystem::path inc = absPath(line.substr(p1 + 1, includeLen));

            if (!std::filesystem::is_regular_file(inc)) {
                throw ShaderPreprocessorError(std::format(
                    "Could not resolve file path for include file '{}'", inc
                ));
            }

            processFile(inc);
            return true;
        }
        else {
            throw ShaderPreprocessorError(std::format(
                "Expected \" or <. {}", debugString()
            ));
        }
    }

    bool Env::parseVersion(const std::string& line) {
        constexpr std::string_view VersionString = "#version __CONTEXT__";
        if (line.starts_with(VersionString)) {
            _output << std::format("{}\n", glslVersionString());
            return true;
        }
        return false;
    }

    bool Env::parseOs(const std::string& line) {
        constexpr std::string_view OsString = "#define __OS__";
        if (line.starts_with(OsString)) {
    #ifdef WIN32
            constexpr std::string_view os = "WIN32";
    #endif // WIN32
    #ifdef __APPLE__
            constexpr std::string_view os = "APPLE";
    #endif // __APPLE__
    #ifdef __linux__
            constexpr std::string_view os = "linux";
    #endif // __linux__
            _output << std::format(
                "#ifndef __OS__\n"
                "#define __OS__ {0}\n"
                "#define {0}\n"
                "#endif\n",
                os
            );
            addLineNumber();
            return true;
        }
        return false;
    }

    bool Env::parseFor(const std::string& line) {
        // parse this:
        // #for <key>, <value> in <dictionary>
        // #for <key> in <a>..<b>

        constexpr std::string_view ForString = "#for";
        if (!line.starts_with(ForString)) {
            return false;
        }

        const size_t keyPos = line.find_first_not_of(Ws, ForString.size());

        std::string keyName;
        size_t commaPos = line.find(',');
        const bool hasKey = commaPos != std::string::npos;
        if (hasKey) {
            keyName = line.substr(keyPos, commaPos - keyPos);
            ghoul::trimWhitespace(keyName);
        }
        else {
            commaPos = keyPos - 1;
            keyName = "";
        }

        const size_t valuePos = line.find_first_not_of(Ws, commaPos + 1);
        const size_t valueEnd = line.find_first_of(Ws, valuePos);
        std::string valueName = line.substr(valuePos, valueEnd - valuePos);
        ghoul::trimWhitespace(valueName);

        const size_t inPos = line.find_first_not_of(Ws, valueEnd);
        constexpr std::string_view InString = "in";
        if (!line.substr(inPos).starts_with(InString)) {
            throw ShaderPreprocessorError(std::format(
                "Expected 'in' in #for statement. {}", debugString()
            ));
        }
        const size_t inEnd = line.find_first_of(Ws, inPos);

        const size_t dictPos = line.find_first_not_of(Ws, inEnd);
        const size_t dictEnd = line.find_first_of(Ws, dictPos);
        std::string dictName = line.substr(dictPos, dictEnd - dictPos);
        ghoul::trimWhitespace(dictName);

        // No key means that the for statement could possibly be a range
        if (!hasKey) {
            auto [min, max, rangeDictionary] = parseRange(dictName);
            dictName = std::format("(Range {} to {})", min, max);
            _dictionary.setValue(dictName, rangeDictionary);
        }

        // Fetch the dictionary to iterate over
        std::string dict = resolveAlias(dictName);
        const ghoul::Dictionary innerDict = _dictionary.value<ghoul::Dictionary>(dict);

        std::vector<std::string_view> keys = innerDict.keys();
        int currentIteration = 0;

        std::map<std::string, std::string> table;
        if (!keys.empty()) {
            table[keyName] = std::format("\"{}\"", keys[0]);
            table[valueName] = std::format("{}.{}", dict, keys[0]);
            currentIteration = 0;

            _output << std::format(
                "//# For loop over {0}\n"
                "//# Key {1} in {0}\n",
                dict, keys[0]
            );
        }
        else {
            currentIteration = ForStatement::EmptyLoop;
            _output << "//# Empty for loop\n";
        }
        addLineNumber();
        _scopes.push_back(table);

        Env::Input& input = _inputs.back();
        _forStatements.emplace_back(
            static_cast<unsigned int>(_inputs.size() - 1),
            input.lineNumber,
            static_cast<unsigned int>(input.stream.tellg()),
            keyName,
            valueName,
            dict,
            currentIteration
        );

        return true;
    }

    bool Env::parseEndFor(const std::string& line) {
        constexpr std::string_view EndForString = "#endfor";

        if (!line.starts_with(EndForString)) {
            return false;
        }

        if (_forStatements.empty()) {
            throw ShaderPreprocessorError(std::format(
                "Unexpected #endfor. No corresponding #for was found. {}", debugString()
            ));
        }

        Env::ForStatement& forStmnt = _forStatements.back();
        // Require #for and #endfor to be in the same input file
        if (forStmnt.inputIndex != _inputs.size() - 1) {
            const int inputIndex = forStmnt.inputIndex;
            const Env::Input& forInput = _inputs[inputIndex];

            throw ShaderPreprocessorError(std::format(
                "Unexpected #endfor. Last #for was in {}: {}. {}",
                forInput.file, forStmnt.lineNumber, debugString()
            ));
        }

        // When we get here, we have already processed the first iteration of the for
        // loop. So we can pop the scope of that variable and move to the next key index
        _scopes.pop_back();
        forStmnt.currentIteration++;

        // Fetch the dictionary to iterate over
        const ghoul::Dictionary innerDict = _dictionary.value<ghoul::Dictionary>(
            forStmnt.dictionary
        );
        std::vector<std::string_view> keys = innerDict.keys();

        // This part of the code effectively behaves like the check in a for loop, except
        // that the for loop is extracting all of the lines of the file
        if (forStmnt.currentIteration < keys.size()) {
            std::string_view key = keys[forStmnt.currentIteration];
            std::map<std::string, std::string> table;
            table[forStmnt.keyName] = std::format("\"{}\"", key);
            table[forStmnt.valueName] = std::format("{}.{}", forStmnt.dictionary, key);
            _scopes.push_back(table);
            _output << std::format("//# Key {} in {}\n", key, forStmnt.dictionary);
            addLineNumber();

            // Restore input to its state from when #for was found
            Env::Input& input = _inputs.back();
            input.stream.seekg(forStmnt.streamPos);
            input.lineNumber = forStmnt.lineNumber;
        }
        else {
            // This was the last iteration or there were zero iterations
            _output <<
                std::format("//# Terminated loop over {}\n", forStmnt.dictionary);
            addLineNumber();
            _forStatements.pop_back();
        }
        return true;
    }
} // namespace

namespace ghoul::opengl {

std::vector<std::filesystem::path> ShaderPreprocessor::_includePaths =
    std::vector<std::filesystem::path>();

ShaderPreprocessor::ShaderPreprocessor(std::filesystem::path shaderPath,
                                       Dictionary dictionary)
    : _shaderPath(std::move(shaderPath))
    , _dictionary(std::move(dictionary))
{}

void ShaderPreprocessor::setDictionary(Dictionary dictionary) {
    _dictionary = std::move(dictionary);
    if (_onChangeCallback) {
        _onChangeCallback();
    }
}

const Dictionary& ShaderPreprocessor::dictionary() const {
    return _dictionary;
}

const std::filesystem::path& ShaderPreprocessor::filename() const {
    return _shaderPath;
}

std::string ShaderPreprocessor::process() {
    Env env = Env(_dictionary);
    env.processFile(_shaderPath);
    std::string result = env.finalize();
    for (const std::filesystem::path& file : env.includedFiles()) {
        filesystem::File f = filesystem::File(file);
        f.setCallback([this]() { _onChangeCallback(); });
        _includedFiles.push_back(std::move(f));
    }
    return result;
}

void ShaderPreprocessor::setCallback(ShaderChangedCallback changeCallback) {
    _onChangeCallback = std::move(changeCallback);
    for (filesystem::File& file : _includedFiles) {
        file.setCallback([this]() { _onChangeCallback(); });
    }
}

std::string ShaderPreprocessor::includedFiles() const {
    std::string result;
    for (size_t i = 0; i < _includedFiles.size(); i++) {
        result = std::format("{}{}: {}\n", result, i, _includedFiles[i].path());
    }
    result.pop_back(); // remove the trailing \n
    return result;
}

void ShaderPreprocessor::addIncludePath(const std::filesystem::path& folderPath) {
    ghoul_assert(!folderPath.empty(), "Folder path must not be empty");
    ghoul_assert(
        std::filesystem::is_directory(folderPath),
        "Folder path must be an existing directory"
    );

    const auto it = std::find(_includePaths.begin(), _includePaths.end(), folderPath);
    if (it == _includePaths.end()) {
        _includePaths.push_back(folderPath);
    }
}

const std::vector<std::filesystem::path> ShaderPreprocessor::includePaths() {
    return _includePaths;
}

} // namespace ghoul::opengl
