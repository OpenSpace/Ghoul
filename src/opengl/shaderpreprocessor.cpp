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

#include <ghoul/opengl/shaderpreprocessor.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/misc/dictionary.h>
#include <ghoul/logging/log.h>
#include <string>
#include <fstream>

namespace {
const std::string _loggerCat = "ShaderPreprocessor";

bool isString(std::string str) {
    return str.length() > 1 && str[0] == '"' && str[str.length() - 1] == '"';
}

std::string trim(const std::string& str, std::string* before = nullptr, std::string* after = nullptr) {
    static const std::string ws = " \n\r\t";
    size_t startPos = str.find_first_not_of(ws);
    if (startPos == std::string::npos)
        startPos = 0;
    size_t endPos = str.find_last_not_of(ws);
    if (endPos == std::string::npos)
        endPos = str.length();
    else
        endPos += 1;

    size_t length = endPos - startPos;
    if (before != nullptr) {
        *before = str.substr(0, startPos);
    }
    if (after != nullptr) {
        *after = str.substr(endPos);
    }
    return str.substr(startPos, length);
}

std::string glslVersionString() {
    int versionMajor;
    int versionMinor;
    int profileMask;
    glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
    glGetIntegerv(GL_MINOR_VERSION, &versionMinor);
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profileMask);
    std::stringstream ss;
    ss << "#version " << versionMajor << versionMinor << "0" << // version is set
        (profileMask & GL_CONTEXT_CORE_PROFILE_BIT ? " core" :
         (profileMask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT ? " compatibility" : ""));
    return ss.str();
}

}

namespace ghoul {
namespace opengl {

std::vector<std::string> ShaderPreprocessor::_includePaths = std::vector<std::string>();

ShaderPreprocessor::ShaderPreprocessor(const std::string& shaderPath, Dictionary dictionary)
    : _shaderPath(shaderPath)
    , _dictionary(dictionary) {}

ShaderPreprocessor::~ShaderPreprocessor() {
  clearTrackedPaths();
}

void ShaderPreprocessor::setShaderPath(const std::string& shaderPath) {
    _shaderPath = shaderPath;
}

void ShaderPreprocessor::setDictionary(Dictionary dictionary) {
    _dictionary = std::move(dictionary);
}


bool ShaderPreprocessor::process(std::string& output) {
    std::stringstream stream;
    ShaderPreprocessor::Env env(stream);
    clearTrackedPaths();

    // Start processing.
    includeFile(absPath(_shaderPath), env);

    if (env.forStatements.size() > 0) {
         env.success = false;
         LERROR("Parse error. Unexpected end of file. In the middle of expanding #for statement. " << debugString(env));
    }

    if (env.scopes.size() > 0) {
        env.success = false;
        LERROR("Parse error. Unexpected end of file. " << debugString(env));
    }


    output = stream.str();
    return env.success;
}

bool ShaderPreprocessor::includeFile(const std::string& path, ShaderPreprocessor::Env& env) {
    if (!trackPath(path)) {
        LERROR("Could not track path. Does file exist? " << path);
        return false;
    }

    std::ifstream stream(path);

    if (!stream.is_open()) {
        LERROR("Could not open file. " << path);
        return false;
    }

    ghoul::filesystem::File file(path);

    std::string prevIndentation = "";
    if (env.inputs.size() > 0) {
        prevIndentation = env.inputs.back().indentation;
    }

    env.inputs.push_back(ShaderPreprocessor::Input(stream, file, prevIndentation + env.indentation));
    addLineNumber(env);

    while (parseLine(env)) {
        if (!env.success) {
            LERROR("Parse error. Could not parse line. " << path << ":" << env.inputs.back().lineNumber);
            return false;
        }
    }

    if (env.forStatements.size() > 0) {
        ShaderPreprocessor::ForStatement& forStatement = env.forStatements.back();
        if (forStatement.inputIndex + 1 >= env.inputs.size()) {
            int inputIndex = forStatement.inputIndex;
            ShaderPreprocessor::Input& forInput = env.inputs[inputIndex];
            std::string path = forInput.file.path();
            int lineNumber = forStatement.lineNumber;

            LERROR("Parse error. Unexpected end of file. Still processing #for loop from " << path << ":" << lineNumber << ". " << debugString(env));
          return false;
        }
    }

    env.inputs.pop_back();

    if (env.inputs.size() > 0) {
      addLineNumber(env);
    }

    return true;
}

void ShaderPreprocessor::addLineNumber(ShaderPreprocessor::Env& env) {
    std::string filename = env.inputs.back().file.path();
    int fileIdentifier = -1;
    if (_fileIdentifiers.find(filename) != _fileIdentifiers.end()) {
        fileIdentifier = _fileIdentifiers[filename];
    }
    if (fileIdentifier == -1) {
        LERROR("could not find in tracked files: " << filename);
    }
    env.output << "; // semicolon separator added by preprocessor to isolate error messages. " << std::endl << "#line " << env.inputs.back().lineNumber << " " << fileIdentifier << std::endl;
}

bool ShaderPreprocessor::isInsideEmptyForStatement(ShaderPreprocessor::Env& env) {
    return env.forStatements.size() > 0 && env.forStatements.back().keyIndex == -1;
}

bool ShaderPreprocessor::parseLine(ShaderPreprocessor::Env& env) {
    Input& input = env.inputs.back();
    if (!std::getline(input.stream, env.line)) {
        return false;
    }
    input.lineNumber++;

    // Trim away any whitespaces in the start and end of the line.
    env.line = trim(env.line, &env.indentation);

    bool specialLine = parseEndFor(env); // #endfor

    if (isInsideEmptyForStatement(env)) {
        return true;
    }

    // Replace all #{<name>} strings with data from <name> in dictionary.
    if (!substituteLine(env)) {
        return false;
    }

    if (!specialLine) {
         specialLine
             |= parseVersion(env) // #version __CONTEXT__
             || parseInclude(env) // #include
             || parseFor(env);    // #for <key>, <value> in <dictionary>
    }

    if (!specialLine) {
      // Write GLSL code to output.
        env.output << input.indentation << env.indentation << env.line << std::endl;
    }
    return true;
    // Insert all extensions to the preprocessor here.
}

std::string ShaderPreprocessor::debugString(ShaderPreprocessor::Env& env) {
    std::stringstream s;
    if (env.inputs.size() > 0) {
        ShaderPreprocessor::Input& input = env.inputs.back();
        s << input.file.path() << ":" << input.lineNumber;
    } else {
        s << "";
    }
        return s.str();
}

bool ShaderPreprocessor::substituteLine(ShaderPreprocessor::Env& env) {
    std::string& line = env.line;
    std::stringstream processed;
    int beginOffset, endOffset;
    while ((beginOffset = line.find("#{")) != -1) {
        endOffset = line.substr(beginOffset).find("}");
        if (endOffset == -1) {
            LERROR("Parse error. Could not parse line. " << debugString(env));
            return false;
        }

        std::string in = line.substr(beginOffset + 2, endOffset - 2);
        std::string out;
        bool success = substitute(in, out, env);
        if (!success) {
            return false;
        }
        processed << line.substr(0, beginOffset) << out;
        line = line.substr(beginOffset + endOffset + 1);
    }

    processed << line;
    line = processed.str();
    return true;
}

bool ShaderPreprocessor::resolveAlias(const std::string& in, std::string& out, ShaderPreprocessor::Env& env) {
    size_t firstDotPos;
    std::string beforeDot, afterDot;
    if ((firstDotPos = in.find(".")) != std::string::npos) {
        beforeDot = in.substr(0, firstDotPos);
        afterDot = in.substr(firstDotPos);
    } else {
        beforeDot = in;
        afterDot = "";
    }

    // Resolve only part before dot
    if (env.aliases.find(beforeDot) != env.aliases.end()) {
        if (env.aliases[beforeDot].size() > 0) {
            beforeDot = env.aliases[beforeDot].back();
        }
    }

    out = beforeDot + afterDot;
    return ((afterDot == "" && isString(beforeDot)) || _dictionary.hasKey(out));
}

bool ShaderPreprocessor::substitute(const std::string& in, std::string& out, ShaderPreprocessor::Env& env) {
    std::string resolved;
    if (!resolveAlias(in, resolved, env)) {
        LERROR("Substitution error. Could not resolve variable '" << in << "'. " << debugString(env));
        return false;
    }

    std::stringstream ss;
    if (isString(resolved)) {
        ss << resolved.substr(1, resolved.length() - 2);
    } else if (_dictionary.hasValue<std::string>(resolved)) {
        ss << _dictionary.value<std::string>(resolved);
    } else if (_dictionary.hasValue<long long>(resolved)) {
        ss << _dictionary.value<long long>(resolved);
    } else if (_dictionary.hasValue<unsigned long long>(resolved)) {
        ss << _dictionary.value<unsigned long long>(resolved);
    } else if (_dictionary.hasValue<double>(resolved)) {
        ss << _dictionary.value<double>(resolved);
    } else if (_dictionary.hasValue<glm::ivec2>(resolved)) {
        glm::ivec2 vec = _dictionary.value<glm::ivec2>(resolved);
        ss << "ivec2(" << vec.x << ", " << vec.y << ")";
    } else if (_dictionary.hasValue<glm::ivec3>(resolved)) {
        glm::ivec3 vec = _dictionary.value<glm::ivec3>(resolved);
        ss << "ivec3(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    } else if (_dictionary.hasValue<glm::uvec2>(resolved)) {
        glm::uvec2 vec = _dictionary.value<glm::uvec2>(resolved);
        ss << "uvec2(" << vec.x << ", " << vec.y << ")";
    } else if (_dictionary.hasValue<glm::uvec3>(resolved)) {
        glm::uvec3 vec = _dictionary.value<glm::uvec3>(resolved);
        ss << "uvec3(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    } else if (_dictionary.hasValue<glm::dvec2>(resolved)) {
        glm::dvec2 vec = _dictionary.value<glm::dvec2>(resolved);
        ss << "dvec2(" << vec.x << ", " << vec.y << ")";
    } else if (_dictionary.hasValue<glm::dvec3>(resolved)) {
        glm::dvec3 vec = _dictionary.value<glm::dvec3>(resolved);
        ss << "dvec3(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    } else {
        LERROR("Substitution error. '" << in << "' was resolved to '" << resolved << "' which has a type that is not supported by the preprocessor. " << debugString(env));
        return false;
    }
    out = ss.str();
    return true;
}

bool ShaderPreprocessor::pushScope(std::map<std::string, std::string> map, ShaderPreprocessor::Env& env) {
    Scope scope;
    for (const auto& pair : map) {
        std::string key = pair.first;
        std::string value = pair.second;
        scope.insert(key);
        if (env.aliases.find(key) == env.aliases.end()) {
           std::vector<std::string> vec;
           env.aliases[key] = vec;
        }
        env.aliases[key].push_back(value);
    }
    env.scopes.push_back(scope);
    return true;
}

bool ShaderPreprocessor::popScope(ShaderPreprocessor::Env& env) {
    if (env.scopes.size() == 0) {
        return false;
    }

    Scope& scope = env.scopes.back();
    for (const auto& key : scope) {
        if (env.aliases.find(key) == env.aliases.end()) {
            // should not happen.
            return false;
         }
        if (env.aliases[key].size() == 0) {
            // should not happen
            return false;
         }
        env.aliases[key].pop_back();
        if (env.aliases[key].size() == 0) {
            env.aliases.erase(key);
        }
    }

    env.scopes.pop_back();
    return true;
}

bool ShaderPreprocessor::parseInclude(ShaderPreprocessor::Env& env) {
    static const std::string includeString = "#include";
    static const std::string ws = " \n\r\t";

    std::string& line = env.line;
    size_t length = line.length();
    if (length > 11 && line.substr(0, includeString.length()) == includeString) {
        size_t p1 = line.find_first_not_of(ws, includeString.length());
        size_t p2 = std::string::npos;
        if (p1 != std::string::npos) {
            if (line.at(p1) == '\"') {
                p2 = line.find_first_of("\"", p1 + 1);
                if (p2 != std::string::npos) {
                     std::string includeFilename = env.inputs.back().file.directoryName() + ghoul::filesystem::FileSystem::PathSeparator + line.substr(p1 + 1, p2 - p1 - 1);

                     bool includeFileWasFound = FileSys.fileExists(includeFilename);

                     // Resolve the include paths if this default includeFilename does
                     // not exist
                     if (!includeFileWasFound) {
                         for (const std::string& includePath : _includePaths) {
                             includeFilename = includePath + ghoul::filesystem::FileSystem::PathSeparator + line.substr(p1 + 1, p2 - p1 - 1);

                             if (FileSys.fileExists(includeFilename)) {
                                 includeFileWasFound = true;
                                 break;
                             }
                         }
                     }

                     if (includeFileWasFound) {
                         includeFile(absPath(includeFilename), env);
                     } else {
                         LERROR("Could not resolve file path for include file '" <<
                             line.substr(p1 + 1, p2 - p1 - 1) << "'");

                         env.success = false;
                         return true;
                     }
                }
           } else if (line.at(p1) == '<') {
                p2 = line.find_first_of(">", p1 + 1);
                if (p2 != std::string::npos) {
                    std::string includeFilename = absPath(line.substr(p1 + 1, p2 - p1 - 1));

                    includeFile(includeFilename, env);
                } else {
                    LERROR("Parse error. Expected >. " << debugString(env));
                    env.success = false;
                    return true;
                }
            } else {
                 LERROR("Parse error. Expected \" or <. " << debugString(env));
                 env.success = false;
                 return true;
            }
        } else {
            LERROR("Parse error. Expected file path. " << debugString(env));
            env.success = false;
            return true;
        }
        return true;
    }
    return false;
}

bool ShaderPreprocessor::parseVersion(ShaderPreprocessor::Env& env) {
    static const std::string versionString = "#version __CONTEXT__";
    std::string& line = env.line;
    if  (line.length() >= versionString.length() && line.substr(0, versionString.length()) == versionString) {
        env.output << glslVersionString();
        return true;
    }
    return false;
}


bool ShaderPreprocessor::tokenizeFor(const std::string& line, std::string& keyName, std::string& valueName, std::string& dictionaryName, ShaderPreprocessor::Env& env) {
    static const std::string forString = "#for";
    static const std::string inString = "in";
    static const std::string ws = " \n\r\t";
    static const std::string comma = ",";

    // parse this:
    // #for <key>, <value> in <dictionary>

    size_t length = line.length();
    if (length < forString.length() + inString.length() || line.substr(0, forString.length()) != forString) {
        return false;
    }

    size_t firstWsPos = forString.length();
    size_t keyOffset = line.substr(firstWsPos).find_first_not_of(ws);
    size_t keyPos = firstWsPos + keyOffset;

    size_t commaOffset = line.substr(keyPos).find_first_of(comma);
    size_t commaPos = keyPos + commaOffset;

    keyName = trim(line.substr(keyPos, commaOffset));

    size_t valueOffset = line.substr(commaPos + 1).find_first_not_of(ws);
    size_t valuePos = commaPos + 1 + valueOffset;

    size_t wsBeforeInOffset = line.substr(valuePos).find_first_of(ws);
    size_t wsBeforeInPos = valuePos + wsBeforeInOffset;

    valueName = trim(line.substr(valuePos, wsBeforeInOffset));

    size_t inOffset = line.substr(wsBeforeInPos).find_first_not_of(ws);
    size_t inPos = wsBeforeInPos + inOffset;

    if (line.substr(inPos).length() < inString.length() + 1 || line.substr(inPos, inString.length()) != inString) {
      LERROR("Parse error. Expected 'in' in #for statement. " << debugString(env));
      return false;
    }

    size_t wsBeforeDictionaryPos = inPos + inString.length();
    size_t dictionaryOffset = line.substr(wsBeforeDictionaryPos).find_first_not_of(ws);
    size_t dictionaryPos = wsBeforeDictionaryPos + dictionaryOffset;

    size_t endOffset = line.substr(dictionaryPos).find_first_of(ws);
    dictionaryName = trim(line.substr(dictionaryPos, endOffset));

    return true;
}

bool ShaderPreprocessor::parseFor(ShaderPreprocessor::Env& env) {
    std::string keyName, valueName, dictionaryName;
    if (!tokenizeFor(env.line, keyName, valueName, dictionaryName, env)) {
         return false;
    }

    // The dictionary name can be an alias.
    // Resolve the real dictionary reference.
    std::string dictionaryRef;
    if (!resolveAlias(dictionaryName, dictionaryRef, env)) {
        LERROR("Substitution error. Could not resolve variable '" << dictionaryName << "'. " << debugString(env));
        env.success = false;
        return true;
    }

    // Fetch the dictionary to iterate over.
    Dictionary innerDictionary = _dictionary.value<Dictionary>(dictionaryRef);

    std::vector<std::string> keys = innerDictionary.keys();
    ShaderPreprocessor::Input& input = env.inputs.back();
    int keyIndex;

    std::map<std::string, std::string> table;
    if (keys.size() > 0) {
        table[keyName] = "\"" + keys[0] + "\"";
        table[valueName] = dictionaryRef + "." + keys[0];
        keyIndex = 0;

        env.output << "//# For loop over " << dictionaryRef << std::endl;
        env.output << "//# Key " << keys[0] << " in " << dictionaryRef << std::endl;
        addLineNumber(env);
    } else {
        keyIndex = -1;
        env.output << "//# Empty for loop" << std::endl;
    }
    pushScope(table, env);

    ShaderPreprocessor::ForStatement forStatement(env.inputs.size() - 1,
        input.lineNumber,
        input.stream.tellg(),
        keyName,
        valueName,
        dictionaryRef,
        keyIndex);
    env.forStatements.push_back(forStatement);
    return true;
}


bool ShaderPreprocessor::parseEndFor(ShaderPreprocessor::Env& env) {
    static const std::string endForString = "#endfor";

    std::string& line = env.line;
    size_t length = line.length();

    if (length > 6 && line.substr(0, endForString.length()) == endForString) {
        if (env.forStatements.size() == 0) {
            LERROR("Parse error. Unexpected #endfor. No corresponing #for was found." << debugString(env));
            env.success = false;
            return true;
        }

        ForStatement& forStatement = env.forStatements.back();
        // Require #for and #endfor to be in the same input file
        if (forStatement.inputIndex != env.inputs.size() - 1) {
            env.success = false;
            int inputIndex = forStatement.inputIndex;
            ShaderPreprocessor::Input& forInput = env.inputs[inputIndex];
            std::string path = forInput.file.path();
            int lineNumber = forStatement.lineNumber;

            LERROR("Parse error. Unexpected #endfor. Last seen #for was in " << path << ":" << lineNumber << ". " << debugString(env));
            env.success = false;
            return true;
        }

        if (!popScope(env)) {
            LERROR("Preprocessor internal error. Failed to pop scope from stack. " << debugString(env));
        }
        forStatement.keyIndex++;

        // Fetch the dictionary to iterate over
        Dictionary innerDictionary = _dictionary.value<Dictionary>(forStatement.dictionaryRef);
        std::vector<std::string> keys = innerDictionary.keys();

        std::map<std::string, std::string> table;
        if (forStatement.keyIndex < static_cast<int>(keys.size())) {
            std::string key = keys[forStatement.keyIndex];
            table[forStatement.keyName] = "\"" + key + "\"";
            table[forStatement.valueName] = forStatement.dictionaryRef + "." + key;
            pushScope(table, env);
            env.output << "//# Key " << key << " in " << forStatement.dictionaryRef << std::endl;
            addLineNumber(env);
            // Restore input to its state from when #for was found
            Input& input = env.inputs.back();
            input.stream.seekg(forStatement.streamPos);
            input.lineNumber = forStatement.lineNumber;
        } else {
            // This was the last iteration (or there ware zero iterations)
            env.output << "//# Terminated loop over " << forStatement.dictionaryRef << std::endl;
            addLineNumber(env);
            env.forStatements.pop_back();
        }
    return true;
    }
    return false;
}

std::string ShaderPreprocessor::getFileIdentifiersString() {
    std::stringstream identifiers;
    for (const auto& f : _fileIdentifiers) {
        identifiers << f.second << ": " << f.first << std::endl;
    }
    return identifiers.str();
}

bool ShaderPreprocessor::trackPath(const std::string& path) {
    if (_trackedFiles.find(path) == _trackedFiles.end()) {
        // Todo, what happens if file does not exist?
        _trackedFiles.emplace(std::pair<std::string, ghoul::filesystem::File>(path, ghoul::filesystem::File(path)));
        const auto& filePtr = _trackedFiles.find(path);
        filePtr->second.setCallback([this](const filesystem::File& file) {
                if (_onChangeCallback) {
                    _onChangeCallback(file);
                }
            });
    }

    if (_fileIdentifiers.find(path) == _fileIdentifiers.end()) {
        _fileIdentifiers[path] = _fileIdentifiers.size();
    }
    return true;
}


void ShaderPreprocessor::setCallback(ShaderChangedCallback changeCallback) {
    _onChangeCallback = changeCallback;
}


void ShaderPreprocessor::clearTrackedPaths() {
     _trackedFiles.clear();
     _fileIdentifiers.clear();
}


bool ShaderPreprocessor::addIncludePath(std::string folderPath) {
    folderPath = absPath(folderPath);

    // We only want unique values in this list
    if (std::find(_includePaths.begin(), _includePaths.end(), folderPath)
        != _includePaths.end()) {
        LERROR("Include path '" << folderPath << "' was already registered");
        return false;
    }
    // We only want valid folders in this folder
    if (!FileSys.directoryExists(folderPath)) {
        LERROR("Include path '" << folderPath << "' is not a valid folder");
        return false;
    }
    // If we managed to get to this place, we have a valid directory
    _includePaths.push_back(std::move(folderPath));
    return true;
}

} // namespace opengl
} // namespace ghoul
