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

#ifndef __GHOUL___SHADERPREPROCESSOR___H__
#define __GHOUL___SHADERPREPROCESSOR___H__

#include <ghoul/filesystem/file.h>
#include <ghoul/misc/boolean.h>
#include <ghoul/misc/dictionary.h>
#include <ghoul/misc/exception.h>
#include <filesystem>
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

    explicit ShaderPreprocessor(std::filesystem::path shaderPath = "",
        Dictionary dictionary = Dictionary());

    const std::filesystem::path& filename() const;
    const Dictionary& dictionary() const;
    void setDictionary(Dictionary dictionary);
    void setFilename(std::filesystem::path shaderPath);
    void setCallback(ShaderChangedCallback changeCallback);
    std::string process();
    std::string includedFiles() const;

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
    static void addIncludePath(const std::filesystem::path& folderPath);

private:
    struct FileStruct {
        filesystem::File file;
        size_t fileIdentifier;
    };

    struct Env {
    public:
        using Scope = std::set<std::string>;

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
            std::string dictionaryReference;

            int keyIndex;
        };

        explicit Env(std::map<std::filesystem::path, FileStruct>& includedFiles, ShaderChangedCallback& onChangeCallback, Dictionary& dictionary);

        bool parseFor(const std::string& line);
        bool parseEndFor(const std::string& line);
        bool parseInclude(std::string_view line);
        bool parseVersion(const std::string& line);
        bool parseOs(const std::string& line);

        // pre path exists
        // pre path not empty
        // pre path must not contain path tokens
        // throws std::ios_base::failure if error opening file
        void processFile(const std::filesystem::path& path);

        void substituteLine(std::string& line) const;
        std::string substitute(std::string_view in) const;
        std::string resolveAlias(std::string_view in) const;

        void pushScope(const std::map<std::string, std::string>& map);

        void popScope();

        void addLineNumber();

        std::string debugString() const;

        std::string output() const;

        std::stringstream _output;
        std::vector<Input> _inputs;
        std::vector<Scope> _scopes;
        std::vector<ForStatement> _forStatements;
        std::map<std::string, std::vector<std::string>> _aliases;
        std::string _indentation;

        // Points towards ShaderPreprocessor::_includedFiles
        std::map<std::filesystem::path, FileStruct>& _includedFiles;

        // Points towards ShaderPreprocessor::_onChangeCallback
        ShaderChangedCallback& _onChangeCallback;

        // Points towards ShaderPreprocessor::_dictionary
        Dictionary& _dictionary;
    };


    std::map<std::filesystem::path, FileStruct> _includedFiles;
    static std::vector<std::filesystem::path> _includePaths;
    std::filesystem::path _shaderPath;
    Dictionary _dictionary;
    ShaderChangedCallback _onChangeCallback;
};

} // namespace ghoul::opengl

#endif // __GHOUL___SHADERPREPROCESSOR___H__
