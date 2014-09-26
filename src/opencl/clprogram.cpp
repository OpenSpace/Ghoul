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

 #include <ghoul/opencl/clprogram.h>

#include <ghoul/opencl/ghoul_cl.hpp>
#include <ghoul/opencl/platform.h>
#include <ghoul/opencl/device.h>
#include <ghoul/opencl/clkernel.h>
#include <ghoul/opencl/clutil.h>

#include <ghoul/logging/logmanager.h>
#include <ghoul/filesystem/filesystem.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>

namespace {
    std::string _loggerCat = "CLProgram";
}

namespace ghoul {
namespace opencl {

CLProgram::CLProgram(): _program(nullptr), _context(nullptr), _warningLevel(Warnings::DEFAULT) {
    
}

CLProgram::CLProgram(CLContext* context, const std::string& filename): _program(0), _warningLevel(Warnings::DEFAULT) {
    if( ! initialize(context, filename)) {
        LERROR("CLProgram not initialized!");
    }
}
CLProgram::~CLProgram() {
    if(_program != 0 && _program.unique()) {
        clReleaseProgram(*_program);
    }
}

bool CLProgram::initialize(CLContext* context, const std::string& filename) {
    if(context == nullptr) {
        LERROR("Could not initialize program, context is nullptr");
        return false;
    }
    
    if( ! ghoul::filesystem::FileSystem::ref().fileExists(filename)) {
        LERROR("Could not initialize program, file does not exist");
        return false;
    }
    
    clearOptions();
    
    int err = 0;
    _context = context;
    std::string contents = readFile(filename);
    const char* constContents = contents.c_str();
    _program = std::make_shared<cl_program>(
                    clCreateProgramWithSource(_context->operator()(), 1,
                                              (const char **) &constContents,
                                              NULL, &err));
    if (err != 0) {
        LFATAL("Could not load program source: " << getErrorString(err));
        _context = 0;
        _program = 0;
    }
    return true;
}


void CLProgram::addDefinition(const std::string& definition, const std::string& value) {
    if (definition != "") {
        _definitions.push_back(std::make_pair(definition, value));
    }
    
}
void CLProgram::addDefinition(const std::string& definition, int value) {
    if (definition != "") {
        std::stringstream ss;
        ss << value;
        _definitions.push_back(std::make_pair(definition, ss.str()));
    }
}
void CLProgram::addDefinition(const std::string& definition, float value) {
    if (definition != "") {
        std::stringstream ss;
        ss << value;
        _definitions.push_back(std::make_pair(definition, ss.str()));
    }
}
void CLProgram::addIncludeDirectory(const std::string& directory) {
    _includeDirectories.push_back(directory);
}

void CLProgram::addIncludeDirectory(const std::vector<std::string>& directories) {
    for(auto directory: directories)
        _includeDirectories.push_back(directory);
}

bool CLProgram::option(const Option o) const{
    switch (o) {
        case Option::SinglePrecisionConstant:
            return _singlePrecisionConstant;
        case Option::DenormsAreZero:
            return _denormAreZero;
        case Option::OptDisable:
            return _optDisable;
        case Option::StrictAliasing:
            return _strictAliasing;
        case Option::MadEnable:
            return _madEnable;
        case Option::NoSignedZero:
            return _noSignedZero;
        case Option::UnsafeMathOptimizations:
            return _unsafeMathOptimizations;
        case Option::FiniteMathOnly:
            return _finiteMathOnly;
        case Option::FastRelaxedMath:
            return _fastRelaxedMath;
        default:
            LDEBUG("Unrecognized option");
            return false;
    }
}

void CLProgram::setOption(const Option o, bool b) {
    switch (o) {
        case Option::SinglePrecisionConstant:
            _singlePrecisionConstant = b;
            break;
        case Option::DenormsAreZero:
            _denormAreZero = b;
            break;
        case Option::OptDisable:
            _optDisable = b;
            break;
        case Option::StrictAliasing:
            _strictAliasing = b;
            break;
        case Option::MadEnable:
            _madEnable = b;
            break;
        case Option::NoSignedZero:
            _noSignedZero = b;
            break;
        case Option::UnsafeMathOptimizations:
            _unsafeMathOptimizations = b;
            break;
        case Option::FiniteMathOnly:
            _finiteMathOnly = b;
            break;
        case Option::FastRelaxedMath:
            _fastRelaxedMath = b;
            break;
        case Option::KernelArgInfo:
            _kernelArgInfo = b;
#ifndef CL_VERSION_1_2
            LWARNING("OpenCL 1.2 specific option. Some of the following functions might not return expected values.");
#endif
            break;
        default:
            LDEBUG("Unrecognized option");
    }
}

void CLProgram::clearOptions() {
    _singlePrecisionConstant = false;
    _denormAreZero = false;
    _optDisable = false;
    _strictAliasing = false;
    _madEnable = false;
    _noSignedZero = false;
    _unsafeMathOptimizations = false;
    _finiteMathOnly = false;
    _fastRelaxedMath = false;
    _kernelArgInfo = false;
}
    
CLProgram::Warnings CLProgram::warningLevel() const {
    return _warningLevel;
}

void CLProgram::setWarningLevel(Warnings w) {
    _warningLevel = w;
}

bool CLProgram::build() {
    assert(_program != 0);
    
    std::string options;
    
    for(auto definition: _definitions) {
        if (definition.second == "") {
            options += "-D " + definition.first + " ";
        } else {
            options += "-D " + definition.first + "=" + definition.second + " ";
        }
    }
    
    for(auto directory: _includeDirectories)
        options += "-I " + directory + " ";
    
    if (_singlePrecisionConstant)
        options += " -cl-single-precision-constant";
    
    if (_denormAreZero)
        options += " -cl-denorms-are-zero";
    
    if (_optDisable)
        options += " -cl-opt-disable";
    
    if (_strictAliasing)
        options += " -cl-strict-aliasing";
    
    if (_madEnable)
        options += " -cl-mad-enable";
    
    if (_noSignedZero)
        options += " -cl-no-signed-zeros";
    
    if (_unsafeMathOptimizations)
        options += " -cl-unsafe-math-optimizations";
    
    if (_finiteMathOnly)
        options += " -cl-finite-math-only";
    
    if (_fastRelaxedMath)
        options += " -cl-fast-relaxed-math";
    
#ifdef CL_VERSION_1_2
    if(_kernelArgInfo)
        options += " -cl-kernel-arg-info";
#endif
    
    switch (_warningLevel) {
        case Warnings::NONE:
            options += " -w";
            break;
        case Warnings::PEDANTIC:
            options += " -Werror";
        default:
            break;
    }
    
    LDEBUG("Buildning with options: '" << options << "'");
    const char* coptions = options.c_str();
    int err = clBuildProgram(*_program, 0, NULL, coptions, NULL, NULL);
    
    if (err != 0) {
        LFATAL("Could not build program: " << getErrorString(err));
        LDEBUG("Build log: \n" << buildLog());
        //_program = 0;
        return false;
    }
    
    return true;
}

std::string CLProgram::buildLog() {
    size_t len;
    char *buffer;
    clGetProgramBuildInfo(*_program, _context->device(), CL_PROGRAM_BUILD_LOG, 0, NULL, &len);
    buffer = new char[len];
    clGetProgramBuildInfo(*_program, _context->device(), CL_PROGRAM_BUILD_LOG, len*sizeof(char), buffer, &len);
    std::string sb = buffer;
    delete[] buffer;
    return sb;
}

bool CLProgram::isValidProgram() const {
    return _program != 0;
}
    
CLKernel CLProgram::createKernel(const std::string& name) {
    return CLKernel(this, name);
}

CLProgram& CLProgram::operator=(const CLProgram& rhs) {
    if (this != &rhs) // protect against invalid self-assignment
    {
        _program = {rhs._program};
        _context = {rhs._context};
        _includeDirectories = {rhs._includeDirectories};
        _definitions = {rhs._definitions};
        _singlePrecisionConstant = rhs._singlePrecisionConstant;
        _denormAreZero = rhs._denormAreZero;
        _optDisable = rhs._optDisable;
        _strictAliasing = rhs._strictAliasing;
        _madEnable = rhs._madEnable;
        _noSignedZero = rhs._noSignedZero;
        _unsafeMathOptimizations = rhs._unsafeMathOptimizations;
        _finiteMathOnly = rhs._finiteMathOnly;
        _fastRelaxedMath = rhs._fastRelaxedMath;
        _warningLevel = rhs._warningLevel;
    }
    return *this;
}

cl_program CLProgram::operator()() const {
    return *_program;
}

cl_program& CLProgram::operator()() {
    return *_program;
}
    
std::string CLProgram::readFile(const std::string& filename) {
    
    std::string contents = "";
    
    std::ifstream file(absPath(filename));
    std::string temp;
    while(std::getline(file, temp)) {
        contents += temp + "\n";
    }
    
    return contents;
}

}
}