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

#include <ghoul/opencl/clkernel.h>

#include <ghoul/opencl/clprogram.h>

#include <cassert>

namespace ghoul {
namespace opencl {
    
CLKernel::CLKernel(CLProgram* program, const std::string& name): _kernel(0) {
    
    int err = 0;
    _kernel = clCreateKernel(program->operator()(), name.c_str(), &err);
    
}

CLKernel::~CLKernel() {}

bool CLKernel::isValidKernel() const {
    return _kernel != 0;
}

int CLKernel::setArgument(unsigned int index, cl_mem* input) {
    assert(isValidKernel());
    return clSetKernelArg(_kernel, index, sizeof(cl_mem), input);
}

CLKernel& CLKernel::operator=(const CLKernel& rhs) {
    if (this != &rhs) {
        _kernel = rhs._kernel;
    }
    return *this;
}
cl_kernel CLKernel::operator()() const {
    return _kernel;
}
cl_kernel& CLKernel::operator()() {
    return _kernel;
}

}
}