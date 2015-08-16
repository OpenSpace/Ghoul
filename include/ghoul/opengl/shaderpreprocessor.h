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
//#include <ghoul/filesystem/file.h>
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
  namespace filesystem {
    class File;
  }

  namespace opengl {
    class ShaderPreprocessor {
    public:
      typedef std::function<void (const ShaderPreprocessor&)> ShaderChangedCallback;
      ShaderPreprocessor(const std::string& shaderPath, Dictionary dict);
      ~ShaderPreprocessor();
      void setDictionary(Dictionary dict);
      void setShaderPath(const std::string& shaderPath);
      void setCallback(ShaderChangedCallback cb);
      bool process(std::string& output);

      struct Input {
        Input(std::ifstream* s, ghoul::filesystem::File* f, std::string indent = "")
          : stream(s)
          , file(f)
          , lineNumber(0)
          , indentation(indent) {}
        std::ifstream* stream;
        ghoul::filesystem::File* file;
        int lineNumber;
        std::string indentation;
      };

      struct ForStatement {
        ForStatement(int input,
                     int lineNum,
                     int pos,
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
          , keyIndex(keyI)
        {}
        
        int inputIndex;
        int lineNumber;
        int streamPos;

        std::string dictionaryRef;
        std::string keyName;
        std::string valueName;
        int keyIndex;
      };

      typedef std::set<std::string> Scope;

      struct Env {
        Env(std::stringstream& o)
          : line("")
          , output(o)
          , indentation("")
          , success(true) {}

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
      bool includeFile(const std::string& path, ShaderPreprocessor::Env& env);
      bool parseLine(ShaderPreprocessor::Env& env);
      bool parseFor(ShaderPreprocessor::Env& env);
      bool parseEndFor(ShaderPreprocessor::Env& env);
      bool parseInclude(ShaderPreprocessor::Env& env);

      bool substituteLine(ShaderPreprocessor::Env& env);
      bool substitute(const std::string& in, std::string& out, ShaderPreprocessor::Env& env);
      bool resolveAlias(const std::string& in, std::string& out, ShaderPreprocessor::Env& env);

      bool pushScope(std::map<std::string, std::string> map, ShaderPreprocessor::Env& env);
      bool popScope(ShaderPreprocessor::Env& env);
      
      bool tokenizeFor(const std::string& line, std::string& keyName, std::string& valueName, std::string& dictionaryName, ShaderPreprocessor::Env& env);
      void addLineNumber(ShaderPreprocessor::Env& env);
      bool isInsideEmptyForStatement(ShaderPreprocessor::Env& env);

      bool trackPath(const std::string& path);
      void clearTrackedPaths();

      std::string debugString(ShaderPreprocessor::Env& env);
      

      std::map<std::string, ghoul::filesystem::File*> _trackedFiles;
      std::vector<std::string> _includePaths;
      std::string _shaderPath;
      Dictionary _dictionary;
      ShaderChangedCallback _onChangeCallback;

    };
  } // namespace opengl
} // namespace ghoul

#endif
