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

#include "opencl/platform.h"
//#include "opencl/cl.hpp"
#include <CL/cl.hpp>

namespace ghoul {
namespace opencl {

Platform::Platform(cl::Platform* platform) {
    _platform = platform;
}
Platform::~Platform() {}

bool Platform::isInitialized() const {
    return _isInitialized;
}

void Platform::fetchInformation() {
    if (isInitialized()) {
        clearInformation();
    }
    std::string tmp_string;

    if(_platform->getInfo(CL_PLATFORM_PROFILE, &tmp_string) == CL_SUCCESS)
        _profile = tmp_string;
    if(_platform->getInfo(CL_PLATFORM_VERSION, &tmp_string) == CL_SUCCESS)
        _version = tmp_string;
    if(_platform->getInfo(CL_PLATFORM_NAME, &tmp_string) == CL_SUCCESS)
        _name = tmp_string;
    if(_platform->getInfo(CL_PLATFORM_VENDOR, &tmp_string) == CL_SUCCESS)
        _vendor = tmp_string;
    if(_platform->getInfo(CL_PLATFORM_EXTENSIONS, &tmp_string) == CL_SUCCESS)
        _extensions = tmp_string;

    _isInitialized = true;
}
void Platform::clearInformation() {
    _profile = "";
    _version = "";
    _name = "";
    _vendor = "";
    _extensions = "";
    
    _isInitialized = false;
}

//
// operators
//
Platform& Platform::operator=(const Platform& rhs) {
    if (this != &rhs) // protect against invalid self-assignment
    {
        *_platform = *rhs._platform;
        
        // Ugly version that refetches all information
        clearInformation();
        if(rhs.isInitialized())
            fetchInformation();
    }
    return *this;
}

Platform& Platform::operator=(const cl::Platform& rhs) {
    *_platform = rhs;
    
    // Ugly version that refetches all information
    clearInformation();
    if(isInitialized())
        fetchInformation();
    
    return *this;
}

cl_platform_id Platform::operator()() const {
    return _platform->operator()();
}

cl_platform_id& Platform::operator()() {
    return _platform->operator()();
}

//
// GET
//
std::string Platform::profile() const {
    return _profile;
}
std::string Platform::version() const {
    return _version;
}
std::string Platform::name() const {
    return _name;
}
std::string Platform::vendor() const {
    return _vendor;
}
std::string Platform::extensions() const {
    return _extensions;
}

}
}
