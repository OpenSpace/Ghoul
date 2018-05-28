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

#ifndef __GHOUL___SHADERPREPROCESSOR___H__
#define __GHOUL___SHADERPREPROCESSOR___H__

#include <ghoul/filesystem/file.h>
#include <ghoul/misc/boolean.h>
#include <ghoul/misc/dictionary.h>
#include <ghoul/misc/exception.h>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

// @TODO(abock): This class needs cleanup

namespace ghoul::opengl {

class ShaderPreprocessor {
public:
    BooleanType(TrackChanges);

    using ShaderChangedCallback = std::function<void ()>;

    struct ShaderPreprocessorError : public RuntimeError {
        explicit ShaderPreprocessorError(std::string msg);
    };

    struct SubstitutionError : public ShaderPreprocessorError {
        explicit SubstitutionError(std::string msg);
    };

    struct ParserError : public ShaderPreprocessorError {
        explicit ParserError(std::string msg);
    };

    struct IncludeError : public ShaderPreprocessorError {
        explicit IncludeError(std::string f);
        std::string file;
    };

    ShaderPreprocessor(std::string shaderPath = "",
        Dictionary dictionary = Dictionary());

    std::string filename();
    Dictionary dictionary();
    void setDictionary(Dictionary dictionary);
    void setFilename(const std::string& shaderPath);
    void setCallback(ShaderChangedCallback changeCallback);
    void process(std::string& output);
    std::string getFileIdentifiersString();

    /**
     * Adds the passed folder to the list of include paths that are checked when a shader
     * includes a file. The list of include paths is traversed in the order in which they
     * where added to this class. The folder in which the shader is located will always be
     * treated as if being on the top of the list.
     *
     * \param folderPath The folder that should be added to the list of include paths
     *
     * \pre \p folderPath must not be empty
     * \pre \p folderPath must be an existing directory
     * \pre \p folderPath must be a path without FileSystem tokens
     */
    static void addIncludePath(std::string folderPath);


private:
    struct Input {
        Input(std::ifstream& str, ghoul::filesystem::File& f, std::string indent);

        std::ifstream& stream;
        ghoul::filesystem::File& file;
        std::string indentation;
        unsigned int lineNumber = 1;
    };

    struct ForStatement {
        unsigned int inputIndex;
        unsigned int lineNumber;
        unsigned int streamPos;

        std::string keyName;
        std::string valueName;
        std::string dictionaryReference;

        int keyIndex;
    };

    struct Env {
        using Scope = std::set<std::string>;

        Env(std::stringstream& out, std::string l = "",
            std::string indent = "");

        std::stringstream& output;
        std::string line;
        std::vector<Input> inputs;
        std::vector<Scope> scopes;
        std::vector<ForStatement> forStatements;
        std::map<std::string, std::vector<std::string> > aliases;
        std::string indentation;
        bool success = true;
    };

    // pre path exists
    // pre path not empty
    // pre path must not contain path tokens
    // throws std::ios_base::failure if error opening file
    void includeFile(const std::string& path, TrackChanges trackChanges,
        ShaderPreprocessor::Env& environment);
    bool parseLine(ShaderPreprocessor::Env& env);
    bool parseFor(ShaderPreprocessor::Env& env);
    bool parseEndFor(ShaderPreprocessor::Env& env);
    bool parseInclude(ShaderPreprocessor::Env& env);
    bool parseVersion(ShaderPreprocessor::Env& env);
    bool parseOs(ShaderPreprocessor::Env& env);

    bool substituteLine(ShaderPreprocessor::Env& env);
    std::string substitute(const std::string& in, ShaderPreprocessor::Env& env);
    bool resolveAlias(const std::string& in, std::string& out,
        ShaderPreprocessor::Env& env);

    void pushScope(std::map<std::string, std::string> map, ShaderPreprocessor::Env& env);

    void popScope(ShaderPreprocessor::Env& env);

    bool tokenizeFor(const std::string& line, std::string& keyName,
        std::string& valueName, std::string& dictionaryName,
        ShaderPreprocessor::Env& env);
    bool parseRange(const std::string& dictionaryName, Dictionary& dictionary, int& min,
        int& max);
    void addLineNumber(ShaderPreprocessor::Env& env);
    bool isInsideEmptyForStatement(ShaderPreprocessor::Env& env);

    std::string debugString(ShaderPreprocessor::Env& env);

    struct FileStruct {
        ghoul::filesystem::File file;
        size_t fileIdentifier;
        bool isTracked;
    };

    std::map<std::string, FileStruct> _includedFiles;
    static std::vector<std::string> _includePaths;
    std::string _shaderPath;
    Dictionary _dictionary;
    ShaderChangedCallback _onChangeCallback;
};

} // namespace ghoul::opengl

#endif // __GHOUL___SHADERPREPROCESSOR___H__
