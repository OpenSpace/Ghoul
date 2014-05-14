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

#ifndef __CLKERNEL_H__
#define __CLKERNEL_H__

#include <ghoul/opencl/ghoul_cl.h>
#include <ghoul/glm.h>

#include <string>
#include <memory>

#ifdef __unix__
	#undef None
#endif //__unix__

namespace ghoul {
namespace opencl {
    
class CLProgram;

class CLKernel {
public:
    
    enum class AddressQualifier {Global, Local, Constant, Private, Error};
    enum class AccessQualifier {ReadOnly, WriteOnly, ReadWrite, None, Error};
    enum class TypeQualifier {Const, Restrict, Volatile, None, Error};

    CLKernel();
    CLKernel(CLProgram* program, const std::string& name);
    ~CLKernel();
    
    bool initialize(CLProgram* program, const std::string& name);
    bool isValidKernel() const;
    
    int setArgument(unsigned int index, cl_mem* input);
    
    template<typename T>
    int setArgument(unsigned int index, T input);
    
    AddressQualifier argumentAddressQualifier(size_t argumentIndex);
    AccessQualifier argumentAccessQualifier(size_t argumentIndex);
    TypeQualifier argumentTypeQualifier(size_t argumentIndex);
    std::string argumentTypeName(size_t argumentIndex);
    std::string argumentName(size_t argumentIndex);
    
    CLKernel& operator=(const CLKernel& rhs);
    cl_kernel operator()() const;
    cl_kernel& operator()();
    
    static std::string AddressQualifierName(AddressQualifier q);
    static std::string AccessQualifierName(AccessQualifier q);
    static std::string TypeQualifierName(TypeQualifier q);
    
private:
    std::shared_ptr<cl_kernel> _kernel;
}; // class CLKernel
}
    
} // namespace ghoul

template<typename T>
int ghoul::opencl::CLKernel::setArgument(unsigned int index, T input) {
    static_assert(std::is_fundamental<T>::value, "T must be a fundemental type");
    return clSetKernelArg(_kernel, index, sizeof(T), &input);
}

#endif
