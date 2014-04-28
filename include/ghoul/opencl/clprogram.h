/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2014                                                               *
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

#ifndef __CLPROGRAM_H__
#define __CLPROGRAM_H__

#include <ghoul/opengl/ghoul_gl.h>
#include <ghoul/opencl/ghoul_cl.h>
#include <ghoul/opencl/clcontext.h>

#include <string>
#include <memory>
#include <vector>

namespace ghoul {
namespace opencl {

class CLKernel;
    
class CLProgram {
public:

    enum class Option {
        SinglePrecisionConstant, DenormsAreZero, OptDisable, StrictAliasing, MadEnable,
        NoSignedZero, UnsafeMathOptimizations, FiniteMathOnly, FastRelaxedMath, KernelArgInfo
    };
    
    enum class Warnings {
        NONE, DEFAULT, PEDANTIC
    };

    CLProgram();
    CLProgram(CLContext* context, const std::string& filename);
    ~CLProgram();
    
    bool initialize(CLContext* context, const std::string& filename);
    
    void addDefinition(const std::string& definition, const std::string& value = "");
    void addDefinition(const std::string& definition, int value);
    void addDefinition(const std::string& definition, float value);
    
    void addIncludeDirectory(const std::string& directory);
    void addIncludeDirectory(const std::vector<std::string>& directories);
    bool option(const Option o) const;
    void setOption(const Option o, bool b);
    Warnings warningLevel() const;
    void setWarningLevel(Warnings w);
    
    void clearOptions();
    
    bool build();
    std::string buildLog();
    
    bool isValidProgram() const;
    
    CLKernel createKernel(const std::string& name);
    
    CLProgram& operator=(const CLProgram& rhs);
    cl_program operator()() const;
    cl_program& operator()();
    
private:
    std::shared_ptr<cl_program> _program;
    CLContext* _context;
    
    std::vector<std::string> _includeDirectories;
    std::vector<std::pair<std::string,std::string> > _definitions;
    
    std::string readFile(const std::string& filename);
    
    bool _singlePrecisionConstant, _denormAreZero, _optDisable, _strictAliasing, _madEnable,
         _noSignedZero, _unsafeMathOptimizations, _finiteMathOnly,_fastRelaxedMath, _kernelArgInfo;
    
    Warnings _warningLevel;
    
}; // class CLProgram
}
    
} // namespace ghoul

#endif