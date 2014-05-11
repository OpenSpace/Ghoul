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

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <ghoul/opencl/ghoul_cl.h>

#include <string>

namespace ghoul {
namespace opencl {

class Platform {
public:
    Platform(cl::Platform* platform);
    ~Platform();
    
    bool isInitialized() const;
    
    void fetchInformation();
    void clearInformation();
    
    Platform& operator=(const Platform& rhs);
    Platform& operator=(const cl::Platform& rhs);
    cl_platform_id operator()() const;
    cl_platform_id& operator()();
    
    std::string profile() const;
    std::string version() const;
    std::string name() const;
    std::string vendor() const;
    std::string extensions() const;
    
protected:
    
private:
    cl::Platform* _platform;
    
    bool _isInitialized;
    
    std::string _profile;
	std::string _version;
	std::string _name;
	std::string _vendor;
	std::string _extensions;

};

} // namespace opencl
} // namespace ghoul

#endif // __PLATFORM_H__
