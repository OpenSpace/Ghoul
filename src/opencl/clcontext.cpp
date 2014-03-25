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

#include <ghoul/opencl/clcontext.h>

#include <ghoul/logging/logmanager.h>
#include <ghoul/filesystem/filesystem.h>
#include <sgct.h>

#include <ghoul/opengl/ghoul_gl.h>
#include <ghoul/opencl/ghoul_cl.h>
#include <ghoul/opencl/ghoul_cl.hpp>
#include <ghoul/opencl/platform.h>
#include <ghoul/opencl/device.h>
#include <ghoul/opencl/clcommandqueue.h>
#include <ghoul/opencl/clprogram.h>
#include <ghoul/opencl/clutil.h>

#include <ghoul/logging/logmanager.h>
#include <ghoul/filesystem/filesystem.h>

namespace {
    std::string _loggerCat = "CLContext";
}

namespace ghoul {
namespace opencl {
    
CLContext::CLContext(): _context(0), _platform(0), _device(0) {}
CLContext::~CLContext() {
    if (_context != 0) {
        clReleaseContext(_context);
    }
}
    
bool CLContext::createContextFromDevice(Device* device) {
    
    int err = 0;
    _context = clCreateContext(0, 1, &device->operator()(), NULL, NULL, &err);
    
    if (err == 0) {
        return true;
    }
    
    // TODO: HANDLE AND PRINT ERROR
    _context = 0;
    
    return false;
}

bool CLContext::createContextFromGLContext() {
    
    LDEBUG("From start " << _platform << " " << _device);
    std::vector<cl::Platform> platforms;
    if(cl::Platform::get(&platforms) != CL_SUCCESS)
        return false;
   
	std::string vendor("NVIDIA");
    for (auto clplatform: platforms) {
        ghoul::opencl::Platform platform(&clplatform);
        platform.fetchInformation();
        
        std::vector<cl::Device> devices;
        clplatform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
        for (auto cldevice: devices) {
            ghoul::opencl::Device device(&cldevice);
            device.fetchInformation();
            
            
            if (device.vendor().compare(0,vendor.length(), vendor) == 0) {
                _platform = platform.operator()();
                _device = device.operator()();
                LDEBUG("Choosing " << _platform << " " << _device);
            }
        }
    }
    
    if (_platform == 0 || _device == 0) {
        LFATAL("Could not find suitable devices");
        return false;
    }

// Windows
#ifdef __WIN32__
    cl_context_properties contextProperties[] = {
        CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
        CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
        CL_CONTEXT_PLATFORM, (cl_context_properties)_platform,
        0
    };
    
// OS X
#elif __APPLE__
    CGLContextObj kCGLContext = CGLGetCurrentContext();
    CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
    cl_context_properties contextProperties[] = {
        CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup,
        0
    };
    
// Linux
#else
    cl_context_properties contextProperties[] = {
        CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
        CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
        CL_CONTEXT_PLATFORM, (cl_context_properties)_platform,
        0
    };
#endif

    int err = 0;
    _context = clCreateContext(contextProperties, 1, &_device, NULL, NULL, &err);

    if (err == 0) {
        return true;
    }
    
    // TODO: HANDLE AND PRINT ERROR
    _context = 0;
    
    return false;
}

bool CLContext::isValidContext() const {
    return _context != 0;
}

CLCommandQueue CLContext::createCommandQueue() {
    return CLCommandQueue(_context,_device);
}

CLProgram CLContext::createProgram(const std::string& filename) {
    return CLProgram(this,filename);
}

cl_mem CLContext::createBuffer(cl_mem_flags memFlags, size_t size, void *data) {
    int err = 0;
    cl_mem mem = clCreateBuffer(_context, memFlags, size, data, &err);
    if (err != 0) {
        LERROR("Could not create buffer: " << getErrorString(err));
    }
    return mem;
}

cl_mem CLContext::createTextureFromGLTexture(cl_mem_flags memFlags, ghoul::opengl::Texture& texture) {
    int err = 0;
    cl_mem mem = clCreateFromGLTexture(_context, memFlags, texture.type(), 0, texture, &err);
    if (err != 0) {
        LERROR("Could not create texture: " << getErrorString(err));
    }
    return mem;
}

CLContext& CLContext::operator=(const CLContext& rhs) {
    if (this != &rhs) {
        _context = rhs._context;
        _device = rhs._device;
        _platform = rhs._platform;
    }
    return *this;
}

CLContext::operator cl_context() const {
    return _context;
}
    
cl_context CLContext::operator()() const {
    return _context;
}
cl_context& CLContext::operator()() {
    return _context;
}

}
}
