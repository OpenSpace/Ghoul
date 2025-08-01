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
#include <utility>

namespace {
    constexpr std::string_view Ws = " \n\r\t";

    struct ShaderPreprocessorError : public ghoul::RuntimeError {
        explicit ShaderPreprocessorError(std::string msg)
            : RuntimeError(std::move(msg), "ShaderPreprocessor")
        {}
    };

    constexpr bool isString(std::string_view str) {
        return str.starts_with('"') && str.ends_with('"');
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
        if (dotPos == std::string_view::npos) {
            return dictionary.hasKey(key);
        }

        const std::string_view before = key.substr(0, dotPos);
        if (dictionary.hasKey(before)) {
            const ghoul::Dictionary d = dictionary.value<ghoul::Dictionary>(before);
            const std::string_view after = key.substr(dotPos + 1);
            return d.hasKey(after);
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
            return d.hasValue<T>(after);
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
        return d.value<T>(after);
    }

    std::pair<int, int> parseRange(const std::string& key, ghoul::Dictionary& dictionary)
    {
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
        for (int i = 0; i <= maximum - minimum; i++) {
            dictionary.setValue(std::to_string(i + 1), std::to_string(minimum + i));
        }

        // Everything went well. Write over min and max
        return { minimum, maximum };
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
    Env env = Env(_includedFiles, _onChangeCallback, _dictionary);
    env.processFile(_shaderPath);

    if (!env._forStatements.empty()) {
        throw ShaderPreprocessorError(std::format(
            "Unexpected end of file in the middle of expanding #for statement. {}",
            env.debugString()
        ));
    }

    if (!env._scopes.empty()) {
        throw ShaderPreprocessorError(std::format(
            "Unexpected end of file. {}", env.debugString()
        ));
    }

    return env.output();
}

void ShaderPreprocessor::setCallback(ShaderChangedCallback changeCallback) {
    _onChangeCallback = std::move(changeCallback);
    for (std::pair<const std::filesystem::path, FileStruct>& files : _includedFiles) {
        files.second.file.setCallback([this]() { _onChangeCallback(); });
    }
}

std::string ShaderPreprocessor::includedFiles() const {
    std::vector<std::pair<std::filesystem::path, size_t>> inc;
    inc.resize(_includedFiles.size());
    std::transform(
        _includedFiles.begin(),
        _includedFiles.end(),
        inc.begin(),
        [](const std::pair<const std::filesystem::path, FileStruct>& p) {
            return std::pair(p.first, p.second.fileIdentifier);
        }
    );

    std::sort(
        inc.begin(),
        inc.end(),
        [](const std::pair<std::filesystem::path, size_t>& lhs,
           const std::pair<std::filesystem::path, size_t>& rhs)
        {
            return lhs.second <= rhs.second;
        }
    );

    std::string result = std::accumulate(
        inc.begin(),
        inc.end(),
        std::string(""),
        [](std::string lhs, const std::pair<const std::filesystem::path, size_t>& f) {
            return std::format("{}{}: {}\n", std::move(lhs), f.second, f.first);
        }
    );
    return result;
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

ShaderPreprocessor::Env::Env(std::map<std::filesystem::path, FileStruct>& includedFiles, ShaderChangedCallback& onChangeCallback, Dictionary& dictionary)
    : _includedFiles(includedFiles)
    , _onChangeCallback(onChangeCallback)
    , _dictionary(dictionary)
{}

void ShaderPreprocessor::Env::processFile(const std::filesystem::path& path) {
    ghoul_assert(!path.empty(), "Path must not be empty");
    ghoul_assert(std::filesystem::is_regular_file(path), "Path must be an existing file");

    if (!_includedFiles.contains(path)) {
        filesystem::File file = filesystem::File(path);
        file.setCallback([this]() { _onChangeCallback(); });

        _includedFiles.emplace(
            path,
            FileStruct {
                std::move(file),
                _includedFiles.size()
            }
        );
    }

    std::ifstream stream = std::ifstream(path, std::ifstream::binary);
    if (!stream.good()) {
        throw RuntimeError(std::format("Error loading include file '{}'", path));
    }
    ghoul_assert(stream.good(), "Input stream is not good");

    const std::string prevIndent = !_inputs.empty() ? _inputs.back().indentation : "";

    _inputs.emplace_back(stream, path, prevIndent + _indentation);
    if (_inputs.size() > 1) {
        addLineNumber();
    }


    while (true) {
        Env::Input& input = _inputs.back();
        std::string line;
        if (!ghoul::getline(input.stream, line)) {
            break;
        }
        input.lineNumber++;

        // Trim away any whitespaces in the start and end of the line
        size_t startPos = line.find_first_not_of(" \n\r\t");
        _indentation = line.substr(0, startPos);
        trimWhitespace(line);

        bool isSpecialLine = parseEndFor(line); // #endfor

        const bool isInsideEmptyForStatement =
            !_forStatements.empty() && (_forStatements.back().keyIndex == -1);
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
        const ShaderPreprocessor::Env::ForStatement& forStatement = _forStatements.back();
        if (forStatement.inputIndex + 1 >= _inputs.size()) {
            const int inputIndex = forStatement.inputIndex;

            throw ShaderPreprocessorError(std::format(
                "Unexpected end of file. Still processing #for loop from '{}': {}. {}",
                _inputs[inputIndex].file, forStatement.lineNumber, debugString()
            ));
        }
    }

    _inputs.pop_back();

    if (!_inputs.empty()) {
        addLineNumber();
    }
}

void ShaderPreprocessor::Env::addLineNumber() {
    const std::filesystem::path filename = _inputs.back().file;
    ghoul_assert(_includedFiles.contains(filename), "File not in included files");
    const size_t fileIdentifier = _includedFiles.at(filename).fileIdentifier;

    std::string_view includeSeparator;
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

std::string ShaderPreprocessor::Env::debugString() const {
    if (_inputs.empty()) {
        return "";
    }

    const ShaderPreprocessor::Env::Input& input = _inputs.back();
    return std::format("{}: {}", input.file, input.lineNumber);
}

std::string ShaderPreprocessor::Env::output() const {
    return _output.str();
}

void ShaderPreprocessor::Env::substituteLine(std::string& line) const {
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

std::string ShaderPreprocessor::Env::resolveAlias(std::string_view in) const {
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
    if (auto it = _aliases.find(beforeDot);  it != _aliases.end() && !it->second.empty())
    {
        beforeDot = it->second.back();
    }

    std::string res = beforeDot + afterDot;
    if (!(((afterDot.empty() && isString(beforeDot)) ||
        hasKeyRecursive(_dictionary, res))))
    {
        throw ShaderPreprocessorError(std::format(
            "Could not resolve variable '{}'. {}", in, debugString()
        ));
    }

    return res;
}

std::string ShaderPreprocessor::Env::substitute(std::string_view in) const {
    std::string resolved = resolveAlias(in);

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
        if (!_aliases.contains(key)) {
           _aliases[key] = std::vector<std::string>();
        }
        _aliases[key].push_back(value);
    }
    _scopes.push_back(scope);
}

void ShaderPreprocessor::Env::popScope() {
    ghoul_assert(!_scopes.empty(), "Environment must have open scope");
    for ([[maybe_unused]] const std::string& key : _scopes.back()) {
        ghoul_assert(_aliases.contains(key), "Key not found");
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

bool ShaderPreprocessor::Env::parseInclude(const std::string& line) {
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

    if ((line[p1] != '\"') && (line[p1] != '<')) {
        throw ShaderPreprocessorError(std::format("Expected \" or <. {}", debugString()));
    }

    if (line[p1] == '\"') {
        const size_t p2 = line.find_first_of('\"', p1 + 1);
        if (p2 == std::string::npos) {
            throw ShaderPreprocessorError(std::format("Expected \" {}", debugString()));
        }

        const size_t includeLength = p2 - p1 - 1;
        const std::filesystem::path includeFilename = line.substr(p1 + 1, includeLength);
        std::filesystem::path includeFilepath =
            _inputs.back().file.parent_path() / includeFilename;

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

        processFile(includeFilepath);
    }
    else if (line.at(p1) == '<') {
        const size_t p2 = line.find_first_of('>', p1 + 1);
        if (p2 == std::string::npos) {
            throw ShaderPreprocessorError(std::format("Expected >. {}", debugString()));
        }

        const size_t includeLen = p2 - p1 - 1;
        const std::filesystem::path include = absPath(line.substr(p1 + 1, includeLen));
        processFile(include);
    }
    return true;
}

bool ShaderPreprocessor::Env::parseVersion(const std::string& line) {
    constexpr std::string_view VersionString = "#version __CONTEXT__";
    if (line.starts_with(VersionString)) {
        _output << std::format("{}\n", glslVersionString());
        return true;
    }
    return false;
}

bool ShaderPreprocessor::Env::parseOs(const std::string& line) {
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

bool ShaderPreprocessor::Env::tokenizeFor(std::string_view line, std::string& keyName,
                                     std::string& valueName, std::string& dictionaryName) const
{

    // parse this:
    // #for <key>, <value> in <dictionary>

    constexpr std::string_view ForString = "#for";
    if (!line.starts_with(ForString)) {
        return false;
    }

    const size_t firstWsPos = ForString.size();
    const size_t keyOffset = line.substr(firstWsPos).find_first_not_of(Ws);
    const size_t keyPos = firstWsPos + keyOffset;

    const size_t commaOffset = line.substr(keyPos).find(',');

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

    const size_t valueOffset = line.substr(commaPos + 1).find_first_not_of(Ws);
    const size_t valuePos = commaPos + 1 + valueOffset;

    const size_t wsBeforeInOffset = line.substr(valuePos).find_first_of(Ws);
    const size_t wsBeforeInPos = valuePos + wsBeforeInOffset;

    valueName = line.substr(valuePos, wsBeforeInOffset);
    trimWhitespace(valueName);

    const size_t inOffset = line.substr(wsBeforeInPos).find_first_not_of(Ws);
    const size_t inPos = wsBeforeInPos + inOffset;

    constexpr std::string_view InString = "in";
    if (line.substr(inPos).length() < InString.length() + 1 ||
        line.substr(inPos, InString.length()) != InString)
    {
        throw ShaderPreprocessorError(std::format(
            "Expected 'in' in #for statement. {}", debugString()
        ));
    }

    const size_t wsBeforeDictionaryPos = inPos + InString.size();
    const size_t dictionaryOffset =
        line.substr(wsBeforeDictionaryPos).find_first_not_of(Ws);
    const size_t dictionaryPos = wsBeforeDictionaryPos + dictionaryOffset;

    const size_t endOffset = line.substr(dictionaryPos).find_first_of(Ws);
    dictionaryName = line.substr(dictionaryPos, endOffset);
    trimWhitespace(dictionaryName);

    return true;
}

bool ShaderPreprocessor::Env::parseFor(const std::string& line) {
    std::string keyName;
    std::string valueName;
    std::string dictionaryName;
    if (!tokenizeFor(line, keyName, valueName, dictionaryName)) {
         return false;
     }

    if (keyName.empty()) {
        // No key means that the for statement could possibly be a range
        Dictionary rangeDictionary;
        auto [min, max] = parseRange(dictionaryName, rangeDictionary);

        // Previous dictionary name is not valid as a key since it has dots in it
        dictionaryName = std::format("(Range {} to {})", min, max);
        // Add the inner dictionary
        _dictionary.setValue(dictionaryName, rangeDictionary);
    }

    // The dictionary name can be an alias
    // Resolve the real dictionary reference
    std::string dictionaryRef = resolveAlias(dictionaryName);

    // Fetch the dictionary to iterate over
    const Dictionary innerDictionary = _dictionary.value<Dictionary>(dictionaryRef);

    std::vector<std::string_view> keys = innerDictionary.keys();
    ShaderPreprocessor::Env::Input& input = _inputs.back();
    int keyIndex = 0;

    std::map<std::string, std::string> table;
    if (!keys.empty()) {
        table[keyName] = std::format("\"{}\"", keys[0]);
        table[valueName] = std::format("{}.{}", dictionaryRef, keys[0]);
        keyIndex = 0;

        _output << std::format(
            "//# For loop over {0}\n"
            "//# Key {1} in {0}\n",
            dictionaryRef, keys[0]
        );
        addLineNumber();
    }
    else {
        keyIndex = -1;
        _output << "//# Empty for loop\n";
    }
    pushScope(table);

    _forStatements.emplace_back(
        static_cast<unsigned int>(_inputs.size() - 1),
        input.lineNumber,
        static_cast<unsigned int>(input.stream.tellg()),
        keyName,
        valueName,
        dictionaryRef,
        keyIndex
    );

    return true;
}

bool ShaderPreprocessor::Env::parseEndFor(const std::string& line) {
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
        const ShaderPreprocessor::Env::Input& forInput = _inputs[inputIndex];

        throw ShaderPreprocessorError(std::format(
            "Unexpected #endfor. Last #for was in {}: {}. {}",
            forInput.file, forStmnt.lineNumber, debugString()
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
        table[forStmnt.valueName] = std::format("{}.{}", forStmnt.dictionaryReference, key);
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
