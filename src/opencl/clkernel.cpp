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
    
CLKernel::AddressQualifier CLKernel::argumentAddressQualifier(size_t argumentIndex) {
#ifdef CL_VERSION_1_2
    cl_kernel_arg_info arginfo = CL_KERNEL_ARG_ADDRESS_QUALIFIER;
    cl_int err;
    
    cl_kernel_arg_address_qualifier returnval;
    size_t length;
    
    err = clGetKernelArgInfo(*_kernel, argumentIndex, arginfo, sizeof(returnval), &returnval, &length);
    if(err != CL_SUCCESS) {
        LERROR("Error when fetching argument information: " << getErrorString(err));
        return AddressQualifier::Error;
    }
    
    switch(returnval) {
        case CL_KERNEL_ARG_ADDRESS_CONSTANT:
            return AddressQualifier::Constant;
        case CL_KERNEL_ARG_ADDRESS_GLOBAL:
            return AddressQualifier::Global;
        case CL_KERNEL_ARG_ADDRESS_LOCAL:
            return AddressQualifier::Local;
        case CL_KERNEL_ARG_ADDRESS_PRIVATE:
        default:
            return AddressQualifier::Private;
            break;
    }
#else
    LWARNING("CL_KERNEL_ARG_ADDRESS_QUALIFIER not supported in OpenCL <1.2. Skipping option.");
    return AddressQualifier::Error;
#endif
}
CLKernel::AccessQualifier CLKernel::argumentAccessQualifier(size_t argumentIndex) {
#ifdef CL_VERSION_1_2
    cl_kernel_arg_info arginfo = CL_KERNEL_ARG_ACCESS_QUALIFIER;
    cl_int err;
    
    cl_kernel_arg_access_qualifier returnval;
    size_t length;
    
    err = clGetKernelArgInfo(*_kernel, argumentIndex, arginfo, sizeof(returnval), &returnval, &length);
    if(err != CL_SUCCESS) {
        LERROR("Error when fetching argument information: " << getErrorString(err));
        return AccessQualifier::Error;
    }
    
    switch(returnval) {
        case CL_KERNEL_ARG_ACCESS_READ_ONLY:
            return AccessQualifier::ReadOnly;
        case CL_KERNEL_ARG_ACCESS_WRITE_ONLY:
            return AccessQualifier::WriteOnly;
        case CL_KERNEL_ARG_ACCESS_READ_WRITE:
            return AccessQualifier::ReadWrite;
        case CL_KERNEL_ARG_ACCESS_NONE:
        default:
            return AccessQualifier::None;
            break;
    }
#else
    LWARNING("CL_KERNEL_ARG_ACCESS_QUALIFIER not supported in OpenCL <1.2. Skipping option.");
    return AccessQualifier::Error;
#endif
}
CLKernel::TypeQualifier CLKernel::argumentTypeQualifier(size_t argumentIndex) {
#ifdef CL_VERSION_1_2
    cl_kernel_arg_info arginfo = CL_KERNEL_ARG_TYPE_QUALIFIER;
    cl_int err;
    
    cl_kernel_arg_type_qualifier returnval;
    size_t length;
    
    err = clGetKernelArgInfo(*_kernel, argumentIndex, arginfo, sizeof(returnval), &returnval, &length);
    if(err != CL_SUCCESS) {
        LERROR("Error when fetching argument information: " << getErrorString(err));
        return TypeQualifier::Error;
    }
    
    switch(returnval) {
        case CL_KERNEL_ARG_TYPE_CONST:
            return TypeQualifier::Const;
        case CL_KERNEL_ARG_TYPE_RESTRICT:
            return TypeQualifier::Restrict;
        case CL_KERNEL_ARG_TYPE_VOLATILE:
            return TypeQualifier::Volatile;
        case CL_KERNEL_ARG_TYPE_NONE:
        default:
            return TypeQualifier::None;
            break;
    }
#else
    LWARNING("CL_KERNEL_ARG_ACCESS_QUALIFIER not supported in OpenCL <1.2. Skipping option.");
    return TypeQualifier::Error;
#endif
}
std::string CLKernel::argumentTypeName(size_t argumentIndex) {
#ifdef CL_VERSION_1_2
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
#else
    LWARNING("CL_KERNEL_ARG_TYPE_NAME not supported in OpenCL <1.2. Skipping option.");
    return "";
#endif
}
std::string CLKernel::argumentName(size_t argumentIndex) {
#ifdef CL_VERSION_1_2
    cl_kernel_arg_info arginfo = CL_KERNEL_ARG_NAME;
    cl_int err;
    
    size_t length;
    std::string returnString;
    char *buffer;
    
    err = clGetKernelArgInfo(*_kernel, argumentIndex, arginfo, 0, NULL, &length);
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
#else
    LWARNING("CL_KERNEL_ARG_NAME not supported in OpenCL <1.2. Skipping option.");
    return "";
#endif
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
        case CLKernel::AddressQualifier::Global:
            return "GLOBAL";
        case CLKernel::AddressQualifier::Local:
            return "LOCAL";
        case CLKernel::AddressQualifier::Private:
            return "PRIVATE";
        case CLKernel::AddressQualifier::Constant:
            return "CONSTANT";
        default:
            return "ERROR";
            break;
    }
}
std::string CLKernel::AccessQualifierName(CLKernel::AccessQualifier q) {
    switch(q) {
        case CLKernel::AccessQualifier::ReadOnly:
            return "READ_ONLY";
        case CLKernel::AccessQualifier::WriteOnly:
            return "WRITE_ONLY";
        case CLKernel::AccessQualifier::ReadWrite:
            return "READ_WRITE";
        case CLKernel::AccessQualifier::None:
            return "NONE";
        default:
            return "ERROR";
            break;
    }
}
std::string CLKernel::TypeQualifierName(CLKernel::TypeQualifier q) {
    switch(q) {
        case CLKernel::TypeQualifier::Const:
            return "CONST";
        case CLKernel::TypeQualifier::None:
            return "NONE";
        case CLKernel::TypeQualifier::Restrict:
            return "RESTRICT";
        case CLKernel::TypeQualifier::Volatile:
            return "VOLATILE";
        default:
            return "ERROR";
            break;
    }
}

}
}
