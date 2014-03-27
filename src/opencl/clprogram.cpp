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

CLProgram::CLProgram(CLContext* context, const std::string& filename): _program(0) {
    
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

bool CLProgram::build() {
    assert(_program != 0);
    
    int err = clBuildProgram(_program, 0, NULL, _options.c_str(), NULL, NULL);
    
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