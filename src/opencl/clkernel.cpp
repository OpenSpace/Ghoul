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
#include <ghoul/opencl/clutil.h>

#include <cassert>
#include <type_traits>
#include <ghoul/logging/logmanager.h>

namespace {
    std::string _loggerCat = "CLKernel";
}
namespace ghoul {
namespace opencl {

CLKernel::CLKernel(): _kernel(0) {
    
}
    
CLKernel::CLKernel(CLProgram* program, const std::string& name): _kernel(0) {
    
    if(! initialize(program, name))
        LERROR("Could not create CLKernel '"<< name << "'");
    
}

CLKernel::~CLKernel() {
    if (_kernel != 0 && _kernel.unique()) {
        clReleaseKernel(*_kernel);
    }
}

bool CLKernel::initialize(CLProgram* program, const std::string& name) {
    assert( ! isValidKernel());
    int err = 0;
    _kernel = std::make_shared<cl_kernel>(clCreateKernel(program->operator()(), name.c_str(), &err));
    return true;
}

bool CLKernel::isValidKernel() const {
    return _kernel != 0;
}

int CLKernel::setArgument(unsigned int index, cl_mem* input) {
    assert(isValidKernel());
    return clSetKernelArg(*_kernel, index, sizeof(cl_mem), input);
}

    
int CLKernel::setArgument(unsigned int index, const glm::mat4& matrix) {
    
}
    
CLKernel::AddressQualifier CLKernel::argumentAddressQualifier(size_t argumentIndex) {
    cl_kernel_arg_info arginfo = CL_KERNEL_ARG_ADDRESS_QUALIFIER;
    cl_int err;
    
    cl_kernel_arg_address_qualifier returnval;
    size_t length;
    
    err = clGetKernelArgInfo(*_kernel, argumentIndex, arginfo, sizeof(returnval), &returnval, &length);
    if(err != CL_SUCCESS) {
        LERROR("Error when fetching argument information: " << getErrorString(err));
        return AddressQualifier::ERROR;
    }
    
    switch(returnval) {
        case CL_KERNEL_ARG_ADDRESS_CONSTANT:
            return AddressQualifier::CONSTANT;
        case CL_KERNEL_ARG_ADDRESS_GLOBAL:
            return AddressQualifier::GLOBAL;
        case CL_KERNEL_ARG_ADDRESS_LOCAL:
            return AddressQualifier::LOCAL;
        case CL_KERNEL_ARG_ADDRESS_PRIVATE:
        default:
            return AddressQualifier::PRIVATE;
            break;
    }
}
CLKernel::AccessQualifier CLKernel::argumentAccessQualifier(size_t argumentIndex) {
    cl_kernel_arg_info arginfo = CL_KERNEL_ARG_ACCESS_QUALIFIER;
    cl_int err;
    
    cl_kernel_arg_access_qualifier returnval;
    size_t length;
    
    err = clGetKernelArgInfo(*_kernel, argumentIndex, arginfo, sizeof(returnval), &returnval, &length);
    if(err != CL_SUCCESS) {
        LERROR("Error when fetching argument information: " << getErrorString(err));
        return AccessQualifier::ERROR;
    }
    
    switch(returnval) {
        case CL_KERNEL_ARG_ACCESS_READ_ONLY:
            return AccessQualifier::READ_ONLY;
        case CL_KERNEL_ARG_ACCESS_WRITE_ONLY:
            return AccessQualifier::WRITE_ONLY;
        case CL_KERNEL_ARG_ACCESS_READ_WRITE:
            return AccessQualifier::READ_WRITE;
        case CL_KERNEL_ARG_ACCESS_NONE:
        default:
            return AccessQualifier::NONE;
            break;
    }
}
CLKernel::TypeQualifier CLKernel::argumentTypeQualifier(size_t argumentIndex) {
    cl_kernel_arg_info arginfo = CL_KERNEL_ARG_ACCESS_QUALIFIER;
    cl_int err;
    
    cl_kernel_arg_access_qualifier returnval;
    size_t length;
    
    err = clGetKernelArgInfo(*_kernel, argumentIndex, arginfo, sizeof(returnval), &returnval, &length);
    if(err != CL_SUCCESS) {
        LERROR("Error when fetching argument information: " << getErrorString(err));
        return TypeQualifier::ERROR;
    }
    
    switch(returnval) {
        case CL_KERNEL_ARG_TYPE_CONST:
            return TypeQualifier::CONST;
        case CL_KERNEL_ARG_TYPE_RESTRICT:
            return TypeQualifier::RESTRICT;
        case CL_KERNEL_ARG_TYPE_VOLATILE:
            return TypeQualifier::VOLATILE;
        case CL_KERNEL_ARG_TYPE_NONE:
        default:
            return TypeQualifier::NONE;
            break;
    }
}
std::string CLKernel::argumentTypeName(size_t argumentIndex) {
    cl_kernel_arg_info arginfo = CL_KERNEL_ARG_TYPE_NAME;
    cl_int err;
    
    size_t length;
    std::string returnString;
    char *buffer;
    
    err = clGetKernelArgInfo(*_kernel, argumentIndex, arginfo, sizeof(length), NULL, &length);
    if(err != CL_SUCCESS) {
        LERROR("Error when fetching argument information: " << getErrorString(err));
        return "";
    }
    
    buffer = new char[length];
    err = clGetKernelArgInfo(*_kernel, argumentIndex, arginfo, length, buffer, &length);
    if(err != CL_SUCCESS) {
        LERROR("Error when fetching argument information: " << getErrorString(err));
        delete[] buffer;
        return "";
    }
    
    returnString = buffer;
    delete[] buffer;
    
    return returnString;
}
std::string CLKernel::argumentName(size_t argumentIndex) {
    cl_kernel_arg_info arginfo = CL_KERNEL_ARG_NAME;
    cl_int err;
    
    size_t length;
    std::string returnString;
    char *buffer;
    
    err = clGetKernelArgInfo(*_kernel, argumentIndex, arginfo, NULL, NULL, &length);
    if(err != CL_SUCCESS) {
        LERROR("Error when fetching argument information: " << getErrorString(err));
        return "";
    }
    
    buffer = new char[length];
    err = clGetKernelArgInfo(*_kernel, argumentIndex, arginfo, length, buffer, &length);
    if(err != CL_SUCCESS) {
        LERROR("Error when fetching argument information: " << getErrorString(err));
        delete[] buffer;
        return "";
    }
    
    returnString = buffer;
    delete[] buffer;
    
    return returnString;
}


CLKernel& CLKernel::operator=(const CLKernel& rhs) {
    if (this != &rhs) {
        _kernel = {rhs._kernel};
    }
    return *this;
}
cl_kernel CLKernel::operator()() const {
    return *_kernel;
}
cl_kernel& CLKernel::operator()() {
    return *_kernel;
}
    
std::string CLKernel::AddressQualifierName(CLKernel::AddressQualifier q) {
    switch(q) {
        case CLKernel::AddressQualifier::GLOBAL:
            return "GLOBAL";
        case CLKernel::AddressQualifier::LOCAL:
            return "LOCAL";
        case CLKernel::AddressQualifier::PRIVATE:
            return "PRIVATE";
        case CLKernel::AddressQualifier::CONSTANT:
            return "CONSTANT";
        default:
            return "ERROR";
            break;
    }
}
std::string CLKernel::AccessQualifierName(CLKernel::AccessQualifier q) {
    switch(q) {
        case CLKernel::AccessQualifier::READ_ONLY:
            return "READ_ONLY";
        case CLKernel::AccessQualifier::WRITE_ONLY:
            return "WRITE_ONLY";
        case CLKernel::AccessQualifier::READ_WRITE:
            return "READ_WRITE";
        case CLKernel::AccessQualifier::NONE:
            return "NONE";
        default:
            return "ERROR";
            break;
    }
}
std::string CLKernel::TypeQualifierName(CLKernel::TypeQualifier q) {
    switch(q) {
        case CLKernel::TypeQualifier::CONST:
            return "CONST";
        case CLKernel::TypeQualifier::NONE:
            return "NONE";
        case CLKernel::TypeQualifier::RESTRICT:
            return "RESTRICT";
        case CLKernel::TypeQualifier::VOLATILE:
            return "VOLATILE";
        default:
            return "ERROR";
            break;
    }
}

}
}