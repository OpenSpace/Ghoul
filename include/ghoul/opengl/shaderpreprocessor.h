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

#ifndef __SHADERPREPROCESSOR_H__
#define __SHADERPREPROCESSOR_H__

#include <ghoul/opengl/ghoul_gl.h>
#include <ghoul/filesystem/file.h>
#include <ghoul/filesystem/filesystem.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <ghoul/misc/dictionary.h>
#include <sstream>
#include <fstream>

namespace ghoul {
namespace opengl {

class ShaderPreprocessor {
public:
  typedef std::function<void (const filesystem::File&)> ShaderChangedCallback;

    ShaderPreprocessor(const std::string& shaderPath = "", Dictionary dict = Dictionary());
    ~ShaderPreprocessor();
    void setDictionary(Dictionary dict);
    void setShaderPath(std::string shaderPath);
    void setCallback(ShaderChangedCallback cb);
    bool process(std::string& output);
    std::string getFileIdentifiersString();

    /**
    * Adds the passed folder to the list of include paths that are checked when a shader
    * includes a file. The list of include paths is traversed in the order in which they
    * where added to this class. If the folder does not exist, an error is logged, the
    * list of include paths is unchanged and <code>false</code> is returned. The folder
    * in which the shader is located will always be treated as if being on the top of the
    * list.
    * \param folderPath The folder that should be added to the list of include paths
    * \return <code>true</code> if the <code>folderPath</code> was added successfully,
    * <code>false</code> otherwise
    */
    static bool addIncludePath(std::string folderPath);

    struct Input {
    Input(std::ifstream& s, ghoul::filesystem::File& f, std::string indent = "")
      : stream(s)
      , file(f)
      , lineNumber(1)
      , indentation(indent) {}
    std::ifstream& stream;
    ghoul::filesystem::File& file;
    unsigned int lineNumber;
    std::string indentation;
    };

    struct ForStatement {
    ForStatement(unsigned int input,
                 unsigned int lineNum,
                 unsigned int pos,
                 const std::string& kName,
                 const std::string& vName,
                 const std::string& dRef,
                 int keyI)
      : inputIndex(input)
      , lineNumber(lineNum)
      , streamPos(pos)
      , keyName(kName)
      , valueName(vName)
      , dictionaryRef(dRef)
      , keyIndex(keyI) {}

    unsigned int inputIndex;
    unsigned int lineNumber;
    unsigned int streamPos;

    std::string keyName;
    std::string valueName;
    std::string dictionaryRef;

    int keyIndex;
    };

    typedef std::set<std::string> Scope;

    struct Env {
        Env(std::stringstream& o)
            : line("")
            , output(o)
            , indentation("")
            , success(true)
        {}

        std::string line;
        std::vector<Input> inputs;
        std::vector<Scope> scopes;
        std::vector<ForStatement> forStatements;
        std::map<std::string, std::vector<std::string> > aliases;
        std::stringstream& output;
        std::string indentation;
        bool success;
    };

private:
    bool includeFile(const std::string& path, bool track, ShaderPreprocessor::Env& env);
    bool parseLine(ShaderPreprocessor::Env& env);
    bool parseFor(ShaderPreprocessor::Env& env);
    bool parseEndFor(ShaderPreprocessor::Env& env);
    bool parseInclude(ShaderPreprocessor::Env& env);
    bool parseVersion(ShaderPreprocessor::Env& env);

    bool substituteLine(ShaderPreprocessor::Env& env);
    bool substitute(const std::string& in, std::string& out, ShaderPreprocessor::Env& env);
    bool resolveAlias(const std::string& in, std::string& out, ShaderPreprocessor::Env& env);

    bool pushScope(std::map<std::string, std::string> map, ShaderPreprocessor::Env& env);
    bool popScope(ShaderPreprocessor::Env& env);

    bool tokenizeFor(const std::string& line, std::string& keyName, std::string& valueName, std::string& dictionaryName, ShaderPreprocessor::Env& env);
    void addLineNumber(ShaderPreprocessor::Env& env);
    bool isInsideEmptyForStatement(ShaderPreprocessor::Env& env);

    bool addIncludePath(const std::string& path, bool track);
    void clearIncludedPaths();

    std::string debugString(ShaderPreprocessor::Env& env);

    std::map<std::string, ghoul::filesystem::File> _includedFiles;
    std::map<std::string, int> _fileIdentifiers;
    static std::vector<std::string> _includePaths;
    std::string _shaderPath;
    Dictionary _dictionary;
    ShaderChangedCallback _onChangeCallback;
};
    
} // namespace opengl
} // namespace ghoul

#endif // __SHADERPREPROCESSOR_H__
