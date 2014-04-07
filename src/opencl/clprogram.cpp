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

CLProgram::CLProgram(CLContext* context, const std::string& filename): _program(0), _warningLevel(Warnings::Default) {
    
    clearOptions();
    
    int err = 0;
    _context = context;
    
    if (ghoul::filesystem::FileSystem::ref().fileExists(filename)) {
        
        std::string contents = readFile(filename);
        const char* constContents = contents.c_str();
        _program = clCreateProgramWithSource(_context->operator()(), 1, (const char **) &constContents, NULL, &err);
    }
    
    if (err != 0) {
        LFATAL("Could not load program source: " << getErrorString(err));
        _program = 0;
    }
}
CLProgram::~CLProgram() {}

void CLProgram::addDefinition(const std::string& definition) {
    if (definition != "") {
        _options += "-D " + definition + " ";
    }
}
void CLProgram::addDefinition(const std::string& definition, const std::string& value) {
    if (definition != "" && value != "") {
        _options += "-D " + definition + "="+value + " ";
    }
    
}
void CLProgram::addDefinition(const std::string& definition, int value) {
    if (definition != "") {
        std::stringstream ss;
        ss << value;
        _options += "-D " + definition + "="+ss.str() + " ";
    }
}
void CLProgram::addIncludeDirectory(const std::string& directory) {
    if (ghoul::filesystem::FileSystem::ref().directoryExists(directory)) {
        _options += "-I " + directory + " ";
    }
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
}
    
CLProgram::Warnings CLProgram::warningLevel() const {
    return _warningLevel;
}

void CLProgram::setWarningLevel(Warnings w) {
    _warningLevel = w;
}

bool CLProgram::build() {
    assert(_program != 0);
    
    std::string options = _options;
    
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
    
    switch (_warningLevel) {
        case Warnings::None:
            options += " -w";
            break;
        case Warnings::WarningsIntoErrors:
            options += " -Werror";
        default:
            break;
    }
    
    int err = clBuildProgram(_program, 0, NULL, options.c_str(), NULL, NULL);
    
    if (err != 0) {
        LFATAL("Could not build program: " << getErrorString(err));
        _program = 0;
        return false;
    }
    
    return true;
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
        _program = rhs._program;
    }
    return *this;
}

cl_program CLProgram::operator()() const {
    return _program;
}

cl_program& CLProgram::operator()() {
    return _program;
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