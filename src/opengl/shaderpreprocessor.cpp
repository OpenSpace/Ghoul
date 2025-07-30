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
#include <ghoul/format.h>
#include <ghoul/glm.h>
#include <ghoul/logging/log.h>
#include <ghoul/misc/dictionary.h>
#include <ghoul/misc/exception.h>
#include <ghoul/misc/stringhelper.h>
#include <ghoul/opengl/ghoul_gl.h>
#include <ghoul/systemcapabilities/openglcapabilitiescomponent.h>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <string>
#include <sstream>

namespace {
    struct ShaderPreprocessorError : public ghoul::RuntimeError {
        explicit ShaderPreprocessorError(std::string msg)
            : RuntimeError(std::move(msg), "ShaderPreprocessor")
        {}
    };

    constexpr bool isString(std::string_view str) {
        return str.length() > 1 && str.front() == '"' && str.back() == '"';
    }

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
        if (dotPos != std::string_view::npos) {
            const std::string_view before = key.substr(0, dotPos);
            const std::string_view after = key.substr(dotPos + 1);

            if (dictionary.hasKey(before)) {
                const ghoul::Dictionary d = dictionary.value<ghoul::Dictionary>(before);
                return d.hasKey(after);
            }
            else {
                return false;
            }
        }
        else {
            return dictionary.hasKey(key);
        }
    }

    template <typename T>
    bool hasValueRecursive(const ghoul::Dictionary& dictionary, std::string_view key) {
        const size_t dotPos = key.find('.');
        if (dotPos != std::string_view::npos) {
            const std::string_view before = key.substr(0, dotPos);
            const std::string_view after = key.substr(dotPos + 1);

            if (dictionary.hasValue<ghoul::Dictionary>(before)) {
                const ghoul::Dictionary d = dictionary.value<ghoul::Dictionary>(before);
                return d.hasValue<T>(after);
            }
            else {
                return false;
            }
        }
        else {
            return dictionary.hasValue<T>(key);
        }
    }

    template <typename T>
    T valueRecursive(const ghoul::Dictionary& dictionary, std::string_view key) {
        const size_t dotPos = key.find('.');
        if (dotPos != std::string_view::npos) {
            const std::string_view before = key.substr(0, dotPos);
            const std::string_view after = key.substr(dotPos + 1);

            const ghoul::Dictionary d = dictionary.value<ghoul::Dictionary>(before);
            return d.value<T>(after);
        }
        else {
            return dictionary.value<T>(key);
        }
    }


    bool parseRange(const std::string& dictionaryName,
        ghoul::Dictionary & dictionary, int& min, int& max)
    {
        static const std::string twoDots = "..";
        const size_t minStart = 0;
        const size_t minEnd = dictionaryName.find(twoDots);

        if (minEnd == std::string::npos) {
            throw ShaderPreprocessorError("Expected '..' in range. " + dictionaryName);
        }
        const int minimum = std::stoi(dictionaryName.substr(minStart, minEnd - minStart));

        const size_t maxStart = minEnd + 2;
        const size_t maxEnd = dictionaryName.length();

        const int maximum = std::stoi(dictionaryName.substr(maxStart, maxEnd - maxStart));

        // Create all the elements in the dictionary
        for (int i = 0; i <= maximum - minimum; i++) {
            dictionary.setValue(std::to_string(i + 1), std::to_string(minimum + i));
        }

        // Everything went well. Write over min and max
        min = minimum;
        max = maximum;

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

ShaderPreprocessor::Env::Input::Input(std::ifstream& str, ghoul::filesystem::File& f,
                                 std::string indent)
    : stream(str)
    , file(f)
    , indentation(std::move(indent))
{}

ShaderPreprocessor::Env::Env(std::stringstream& out, std::map<std::filesystem::path, FileStruct>& includedFiles, ShaderChangedCallback& onChangeCallback, Dictionary& dictionary)
    : _output(out)
    , _includedFiles(includedFiles)
    , _onChangeCallback(onChangeCallback)
    , _dictionary(dictionary)
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

void ShaderPreprocessor::setFilename(std::filesystem::path shaderPath) {
    if (_shaderPath != shaderPath) {
        _shaderPath = std::move(shaderPath);
        if (_onChangeCallback) {
            _onChangeCallback();
        }
    }
}

const std::filesystem::path& ShaderPreprocessor::filename() const {
    return _shaderPath;
}

std::string ShaderPreprocessor::process() {
    std::stringstream stream;
    Env env = Env(stream, _includedFiles, _onChangeCallback, _dictionary);

    env.includeFile(_shaderPath, TrackChanges::Yes);

    if (!env._forStatements.empty()) {
        throw ShaderPreprocessorError(
            "Unexpected end of file in the middle of expanding #for statement. " +
            env.debugString()
        );
    }

    if (!env._scopes.empty()) {
        throw ShaderPreprocessorError("Unexpected end of file. " + env.debugString());
    }

    return stream.str();
}

void ShaderPreprocessor::setCallback(ShaderChangedCallback changeCallback) {
    _onChangeCallback = std::move(changeCallback);
    for (std::pair<const std::filesystem::path, FileStruct>& files : _includedFiles) {
        if (files.second.isTracked) {
            files.second.file.setCallback([this]() { _onChangeCallback(); });
        }
    }
}

std::string ShaderPreprocessor::includedFiles() const {
    std::string newStr = std::accumulate(
        _includedFiles.begin(),
        _includedFiles.end(),
        std::string(""),
        [](std::string lhs, const std::pair<const std::filesystem::path, FileStruct>& f) {
            return std::format(
                "{}{}: {}\n", std::move(lhs), f.second.fileIdentifier, f.first
            );
        }
    );
    std::stringstream identifiers;
    for (const std::pair<const std::filesystem::path, FileStruct>& f : _includedFiles) {
        identifiers << f.second.fileIdentifier << ": " << f.first << '\n';
    }
    std::string oldStr = identifiers.str();
    ghoul_assert(newStr == oldStr, "");
    return newStr;
}

void ShaderPreprocessor::addIncludePath(const std::filesystem::path& folderPath) {
    ghoul_assert(!folderPath.empty(), "Folder path must not be empty");
    ghoul_assert(
        std::filesystem::is_directory(folderPath),
        "Folder path must be an existing directory"
    );

    const auto it = std::find(_includePaths.begin(), _includePaths.end(), folderPath);
    if (it == _includePaths.cend()) {
        _includePaths.push_back(folderPath);
    }
}

void ShaderPreprocessor::Env::includeFile(const std::filesystem::path& path,
                                     TrackChanges trackChanges)
{
    ghoul_assert(!path.empty(), "Path must not be empty");
    ghoul_assert(std::filesystem::is_regular_file(path), "Path must be an existing file");

    if (_includedFiles.find(path) == _includedFiles.end()) {
        const auto it = _includedFiles.emplace(
            path,
            FileStruct {
                filesystem::File(path),
                _includedFiles.size(),
                trackChanges
            }
        ).first;
        if (trackChanges) {
            it->second.file.setCallback([this]() { _onChangeCallback(); });
        }
    }

    std::ifstream stream = std::ifstream(path, std::ifstream::binary);
    if (!stream.good()) {
        throw ghoul::RuntimeError(std::format("Error loading include file '{}'", path));
    }
    ghoul_assert(stream.good(), "Input stream is not good");

    ghoul::filesystem::File file = ghoul::filesystem::File(path);

    const std::string prevIndent = !_inputs.empty() ? _inputs.back().indentation : "";

    _inputs.emplace_back(stream, file, prevIndent + _indentation);
    if (_inputs.size() > 1) {
        addLineNumber();
    }

    while (parseLine()) {
        if (!_success) {
            throw ShaderPreprocessorError(std::format(
                "Could not parse line. '{}': {}", path, _inputs.back().lineNumber
            ));
        }
    }

    if (!_forStatements.empty()) {
        const ShaderPreprocessor::Env::ForStatement& forStatement = _forStatements.back();
        if (forStatement.inputIndex + 1 >= _inputs.size()) {
            const int inputIndex = forStatement.inputIndex;
            const ShaderPreprocessor::Env::Input& forInput = _inputs[inputIndex];
            const std::filesystem::path p = forInput.file.path();
            int lineNumber = forStatement.lineNumber;

            throw ShaderPreprocessorError(std::format(
                "Unexpected end of file. Still processing #for loop from '{}': {}. {}",
                p, lineNumber, debugString()
            ));
        }
    }

    _inputs.pop_back();

    if (!_inputs.empty()) {
        addLineNumber();
    }
}

void ShaderPreprocessor::Env::addLineNumber() {
    const std::filesystem::path filename = _inputs.back().file.path();
    ghoul_assert(
        _includedFiles.find(filename) != _includedFiles.end(),
        "File not in included files"
    );
    const size_t fileIdentifier = _includedFiles.at(filename).fileIdentifier;

    std::string includeSeparator;
#ifndef __APPLE__
    // Sofar, only Nvidia on Windows supports empty statements in the middle of the shader
    using Vendor = ghoul::systemcapabilities::OpenGLCapabilitiesComponent::Vendor;
    if (OpenGLCap.gpuVendor() == Vendor::Nvidia) {
        includeSeparator = "; // preprocessor add semicolon to isolate error messages";
    }
#endif // __APPLE__

    _output << std::format(
        "{}\n#line {} {} // {}\n",
        includeSeparator, _inputs.back().lineNumber, fileIdentifier, filename
    );
}

bool ShaderPreprocessor::Env::parseLine() {
    Env::Input& input = _inputs.back();
    if (!ghoul::getline(input.stream, _line)) {
        return false;
    }
    input.lineNumber++;

    // Trim away any whitespaces in the start and end of the line.
    static const std::string ws = " \n\r\t";
    size_t startPos = _line.find_first_not_of(ws);
    _indentation = _line.substr(0, startPos);
    trimWhitespace(_line);

    bool isSpecialLine = parseEndFor(); // #endfor

    const bool isInsideEmptyForStatement =
        !_forStatements.empty() && (_forStatements.back().keyIndex == -1);
    if (isInsideEmptyForStatement) {
        return true;
    }

    // Replace all #{<name>} strings with data from <name> in dictionary.
    if (!substituteLine()) {
        return false;
    }

    if (!isSpecialLine) {
        isSpecialLine |=
            parseVersion() ||    // #version __CONTEXT__
            parseOs() ||         // #define __OS__
            parseInclude() ||    // #include
            parseFor();          // #for <key>, <value> in <dictionary>
    }

    if (!isSpecialLine) {
      // Write GLSL code to output.
        _output << input.indentation << _indentation << _line << '\n';
    }
    return true;
    // Insert all extensions to the preprocessor here.
}

std::string ShaderPreprocessor::Env::debugString() const {
    if (_inputs.empty()) {
        return "";
    }

    const ShaderPreprocessor::Env::Input& input = _inputs.back();
    return std::format("{}: {}", input.file.path(), input.lineNumber);
}

bool ShaderPreprocessor::Env::substituteLine() {
    std::string& line = _line;
    size_t beginOffset = 0;

    while ((beginOffset = line.rfind("#{")) != std::string::npos) {
        const size_t endOffset = line.substr(beginOffset).find('}');
        if (endOffset == std::string::npos) {
            throw ShaderPreprocessorError("Could not parse line. " + debugString());
        }

        const std::string in = line.substr(beginOffset + 2, endOffset - 2);
        const std::string out = substitute(in);

        const std::string first = line.substr(0, beginOffset);
        const std::string last = line.substr(
            beginOffset + endOffset + 1,
            line.length() - 1 - (beginOffset + endOffset)
        );

        line = std::format("{}{}{}", first, out, last);
    }
    return true;
}

bool ShaderPreprocessor::Env::resolveAlias(const std::string& in, std::string& out) {
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
    if (_aliases.find(beforeDot) != _aliases.end()) {
        if (!_aliases[beforeDot].empty()) {
            beforeDot = _aliases[beforeDot].back();
        }
    }

    out = beforeDot + afterDot;
    return ((afterDot.empty() && isString(beforeDot)) ||
           hasKeyRecursive(_dictionary, out));
}

std::string ShaderPreprocessor::Env::substitute(const std::string& in) {
    std::string resolved;
    if (!resolveAlias(in, resolved)) {
        throw ShaderPreprocessorError(std::format(
            "Could not resolve variable '{}'. {}", in, debugString()
        ));
    }

    if (isString(resolved)) {
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
    else if (hasValueRecursive<glm::dvec2>(_dictionary, resolved)) {
        glm::dvec2 vec = valueRecursive<glm::dvec2>(_dictionary, resolved);
        return std::format("dvec2({},{})", vec.x, vec.y);
    }
    else if (hasValueRecursive<glm::dvec3>(_dictionary, resolved)) {
        glm::dvec3 vec = valueRecursive<glm::dvec3>(_dictionary, resolved);
        return std::format("dvec3({},{},{})", vec.x, vec.y, vec.z);
    }
    else {
        throw ShaderPreprocessorError(std::format(
            "'{}' was resolved to '{}' which is a type that is not supported. {}",
            in, resolved, debugString()
        ));
    }
}

void ShaderPreprocessor::Env::pushScope(const std::map<std::string, std::string>& map) {
    Env::Scope scope;
    for (const std::pair<const std::string, std::string>& pair : map) {
        const std::string& key = pair.first;
        const std::string& value = pair.second;
        scope.insert(key);
        if (_aliases.find(key) == _aliases.end()) {
           _aliases[key] = std::vector<std::string>();
        }
        _aliases[key].push_back(value);
    }
    _scopes.push_back(scope);
}

void ShaderPreprocessor::Env::popScope() {
    ghoul_assert(!_scopes.empty(), "Environment must have open scope");
    for (const std::string& key : _scopes.back()) {
        (void)key;
        ghoul_assert(_aliases.find(key) != _aliases.end(), "Key not found");
        ghoul_assert(!_aliases.at(key).empty(), "No aliases for key");
    }

    const Env::Scope& scope = _scopes.back();
    for (const std::string& key : scope) {
        _aliases[key].pop_back();
        if (_aliases[key].empty()) {
            _aliases.erase(key);
        }
    }
    _scopes.pop_back();
}

bool ShaderPreprocessor::Env::parseInclude() {
    static const std::string includeString = "#include";
    static const std::string ws = " \n\r\t";
    static const std::string noTrackString = ":notrack";

    std::string& line = _line;

    const bool tracksInclude = (line.find(noTrackString) == std::string::npos);

    if (line.substr(0, includeString.length()) != includeString) {
        return false;
    }

    const size_t p1 = line.find_first_not_of(ws, includeString.length());
    if (p1 == std::string::npos) {
        throw ShaderPreprocessorError("Expected file path after #include. " + debugString());
    }

    if ((line[p1] != '\"') && (line[p1] != '<')) {
        throw ShaderPreprocessorError("Expected \" or <. " + debugString());
    }

    if (line[p1] == '\"') {
        const size_t p2 = line.find_first_of('\"', p1 + 1);
        if (p2 == std::string::npos) {
            throw ShaderPreprocessorError("Expected \"" + debugString());
        }

        const size_t includeLength = p2 - p1 - 1;
        const std::filesystem::path includeFilename = line.substr(p1 + 1, includeLength);
        std::filesystem::path includeFilepath =
            _inputs.back().file.path().parent_path() / includeFilename;

        bool includeFileWasFound = std::filesystem::is_regular_file(includeFilepath);

        // Resolve the include paths if this default includeFilename does not exist
        if (!includeFileWasFound) {
            for (const std::filesystem::path& path : _includePaths) {
                includeFilepath = path / includeFilename;
                if (std::filesystem::is_regular_file(includeFilepath)) {
                    includeFileWasFound = true;
                    break;
                }
            }
        }

        if (!includeFileWasFound) {
            // Our last chance is that the include file is an absolute path
            const bool found = std::filesystem::is_regular_file(includeFilename);
            if (found) {
                includeFilepath = includeFilename;
                includeFileWasFound = true;
            }
        }

        if (!includeFileWasFound) {
            throw ShaderPreprocessorError(std::format(
                "Could not resolve file path for include file '{}'", includeFilepath
            ));
        }

        includeFile(includeFilepath, TrackChanges(tracksInclude));
    }
    else if (line.at(p1) == '<') {
        const size_t p2 = line.find_first_of('>', p1 + 1);
        if (p2 == std::string::npos) {
            throw ShaderPreprocessorError("Expected >. " + debugString());
        }

        const size_t includeLen = p2 - p1 - 1;
        const std::filesystem::path include = absPath(line.substr(p1 + 1, includeLen));
        includeFile(include, TrackChanges(tracksInclude));
    }
    return true;
}

bool ShaderPreprocessor::Env::parseVersion() const {
    static const std::string versionString = "#version __CONTEXT__";
    if (_line.substr(0, versionString.length()) == versionString) {
        _output << glslVersionString() << '\n';
        return true;
    }
    return false;
}

bool ShaderPreprocessor::Env::parseOs() {
    static const std::string osString = "#define __OS__";
    const std::string& line = _line;
    if (line.length() >= osString.length() &&
        line.substr(0, osString.length()) == osString)
    {
#ifdef WIN32
        constexpr std::string_view os = "WIN32";
#endif
#ifdef __APPLE__
        constexpr std::string_view os = "APPLE";
#endif
#ifdef __linux__
        constexpr std::string_view os = "linux";
#endif
        _output << std::format(
            "#ifndef __OS__\n"
            "#define __OS__ {}\n"
            "#define {}\n"
            "#endif\n",
            os, os
        );
        addLineNumber();
        return true;
    }
    return false;
}

bool ShaderPreprocessor::Env::tokenizeFor(const std::string& line, std::string& keyName,
                                     std::string& valueName, std::string& dictionaryName)
{
    static const std::string forString = "#for";
    static const std::string inString = "in";
    static const std::string ws = " \n\r\t";
    static const std::string comma = ",";

    // parse this:
    // #for <key>, <value> in <dictionary>

    const size_t length = line.length();
    if (length < forString.length() + inString.length() ||
        line.substr(0, forString.length()) != forString)
    {
        return false;
    }

    const size_t firstWsPos = forString.length();
    const size_t keyOffset = line.substr(firstWsPos).find_first_not_of(ws);
    const size_t keyPos = firstWsPos + keyOffset;

    const size_t commaOffset = line.substr(keyPos).find_first_of(comma);

    size_t commaPos = 0;
    if (commaOffset != std::string::npos) { // Found a comma
        commaPos = keyPos + commaOffset;
        keyName = line.substr(keyPos, commaOffset);
        trimWhitespace(keyName);
    }
    else {
        commaPos = keyPos - 1;
        keyName = "";
    }

    const size_t valueOffset = line.substr(commaPos + 1).find_first_not_of(ws);
    const size_t valuePos = commaPos + 1 + valueOffset;

    const size_t wsBeforeInOffset = line.substr(valuePos).find_first_of(ws);
    const size_t wsBeforeInPos = valuePos + wsBeforeInOffset;

    valueName = line.substr(valuePos, wsBeforeInOffset);
    trimWhitespace(valueName);

    const size_t inOffset = line.substr(wsBeforeInPos).find_first_not_of(ws);
    const size_t inPos = wsBeforeInPos + inOffset;

    if (line.substr(inPos).length() < inString.length() + 1 ||
        line.substr(inPos, inString.length()) != inString)
    {
        throw ShaderPreprocessorError("Expected 'in' in #for statement. " + debugString());
    }

    const size_t wsBeforeDictionaryPos = inPos + inString.length();
    const size_t dictionaryOffset =
        line.substr(wsBeforeDictionaryPos).find_first_not_of(ws);
    const size_t dictionaryPos = wsBeforeDictionaryPos + dictionaryOffset;

    const size_t endOffset = line.substr(dictionaryPos).find_first_of(ws);
    dictionaryName = line.substr(dictionaryPos, endOffset);
    trimWhitespace(dictionaryName);

    return true;
}

bool ShaderPreprocessor::Env::parseFor() {
    std::string keyName;
    std::string valueName;
    std::string dictionaryName;
    if (!tokenizeFor(_line, keyName, valueName, dictionaryName)) {
         return false;
     }

    if (keyName.empty()) {
        // No key means that the for statement could possibly be a range.
        Dictionary rangeDictionary;
        int min = 0;
        int max = 0;
        if (!parseRange(dictionaryName, rangeDictionary, min, max)) {
            return false;
        }
        // Previous dictionary name is not valid as a key since it has dots in it.
        dictionaryName = std::format("(Range {} to {})", min, max);
        // Add the inner dictionary
        _dictionary.setValue(dictionaryName, rangeDictionary);
    }

    // The dictionary name can be an alias.
    // Resolve the real dictionary reference.
    std::string dictionaryRef;
    if (!resolveAlias(dictionaryName, dictionaryRef)) {
        throw ShaderPreprocessorError(std::format(
            "Could not resolve variable '{}'. {}", dictionaryName, debugString()
        ));
    }
    // Fetch the dictionary to iterate over.
    const Dictionary innerDictionary = _dictionary.value<Dictionary>(dictionaryRef);

    std::vector<std::string_view> keys = innerDictionary.keys();
    ShaderPreprocessor::Env::Input& input = _inputs.back();
    int keyIndex = 0;

    std::map<std::string, std::string> table;
    if (!keys.empty()) {
        table[keyName] = "\"" + std::string(keys[0]) + "\"";
        table[valueName] = dictionaryRef + "." + std::string(keys[0]);
        keyIndex = 0;

        _output << "//# For loop over " << dictionaryRef << '\n';
        _output << "//# Key " << keys[0] << " in " << dictionaryRef << '\n';
        addLineNumber();
    }
    else {
        keyIndex = -1;
        _output << "//# Empty for loop\n";
    }
    pushScope(table);

    _forStatements.push_back({
        static_cast<unsigned int>(_inputs.size() - 1),
        input.lineNumber,
        static_cast<unsigned int>(input.stream.tellg()),
        keyName,
        valueName,
        dictionaryRef,
        keyIndex
    });

    return true;
}

bool ShaderPreprocessor::Env::parseEndFor() {
    static const std::string endForString = "#endfor";

    const std::string& line = _line;
    const size_t length = line.length();

    if (length <= 6 || line.substr(0, endForString.length()) != endForString) {
        return false;
    }

    if (_forStatements.empty()) {
        throw ShaderPreprocessorError(
            "Unexpected #endfor. No corresponding #for was found" + debugString()
        );
    }

    Env::ForStatement& forStmnt = _forStatements.back();
    // Require #for and #endfor to be in the same input file
    if (forStmnt.inputIndex != _inputs.size() - 1) {
        _success = false;
        const int inputIndex = forStmnt.inputIndex;
        const ShaderPreprocessor::Env::Input& forInput = _inputs[inputIndex];
        std::filesystem::path path = forInput.file.path();
        int lineNumber = forStmnt.lineNumber;

        throw ShaderPreprocessorError(std::format(
            "Unexpected #endfor. Last #for was in {}: {}. {}",
            path, lineNumber, debugString()
        ));
    }

    popScope();
    forStmnt.keyIndex++;

    // Fetch the dictionary to iterate over
    const Dictionary innerDict = _dictionary.value<Dictionary>(
        forStmnt.dictionaryReference
    );
    std::vector<std::string_view> keys = innerDict.keys();

    std::map<std::string, std::string> table;
    if (forStmnt.keyIndex < static_cast<int>(keys.size())) {
        std::string_view key = keys[forStmnt.keyIndex];
        table[forStmnt.keyName] = std::format("\"{}\"", key);
        table[forStmnt.valueName] = std::format(
            "{}.{}", forStmnt.dictionaryReference, key
        );
        pushScope(table);
        _output << std::format("//# Key {} in {}\n", key, forStmnt.dictionaryReference);
        addLineNumber();
        // Restore input to its state from when #for was found
        Env::Input& input = _inputs.back();
        input.stream.seekg(forStmnt.streamPos);
        input.lineNumber = forStmnt.lineNumber;
    }
    else {
        // This was the last iteration (or there ware zero iterations)
        _output <<
            std::format("//# Terminated loop over {}\n", forStmnt.dictionaryReference);
        addLineNumber();
        _forStatements.pop_back();
    }
    return true;
}

} // namespace ghoul::opengl
