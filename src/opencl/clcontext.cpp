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

#include <ghoul/opencl/ghoul_cl.hpp>
#include <ghoul/opencl/platform.h>
#include <ghoul/opencl/device.h>
#include <ghoul/opencl/clcommandqueue.h>
#include <ghoul/opencl/clprogram.h>
#include <ghoul/opencl/clutil.h>

#include <ghoul/logging/logmanager.h>
#include <ghoul/filesystem/filesystem.h>
#include <ghoul/opengl/texture.h>

// Windows
#ifdef WIN32
    #include <Windows.h>
    #include <WinUser.h>

// OS X
#elif __APPLE__

// Linux
#else
    #include <GL/glx.h>
#endif


namespace {
    std::string _loggerCat = "CLContext";
}

namespace ghoul {
namespace opencl {
    
CLContext::CLContext(): _context(0), _platform(0), _device(0) {}
CLContext::~CLContext() {
    LDEBUG("Destructing object");
    if (_context != 0 && _context.unique()) {
        LDEBUG("Releasing context");
        clReleaseContext(*_context);
    }
}
    
bool CLContext::createContextFromDevice(Device* device) {
    assert(_context == 0);
    
    int err = 0;
    _context = std::make_shared<cl_context>(clCreateContext(0, 1, &device->operator()(), NULL, NULL, &err));
    
    if (err == 0) {
        return true;
    }
    
    // TODO: HANDLE AND PRINT ERROR
    _context = 0;
    
    return false;
}

bool CLContext::createContextFromGLContext() {
    assert(_context == 0);
    
    //LDEBUG("From start " << _platform << " " << _device);
    std::vector<cl::Platform> platforms;
    if(cl::Platform::get(&platforms) != CL_SUCCESS)
        return false;
   
    std::vector<cl::Device> allDevices;
    bool successCreateContext = false;
    for (auto clplatform: platforms) {
        ghoul::opencl::Platform platform(&clplatform);
        platform.fetchInformation();
        
        std::vector<cl::Device> devices;
        clplatform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
        allDevices.insert(allDevices.end(), devices.begin(), devices.end());
        
        for (auto cldevice: devices) {
            successCreateContext = false;
            ghoul::opencl::Device device(&cldevice);
            device.fetchInformation();
            
            int err = 0;
            cl_context context = 0;
            cl_device_id did = device.operator()();
            cl_platform_id pid = platform.operator()();
            
            // Windows
#ifdef WIN32
            cl_context_properties contextProperties[] = {
                CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
                CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
                CL_CONTEXT_PLATFORM, (cl_context_properties)pid,
                0
            };
            context = clCreateContext(contextProperties, 1, &did, NULL, NULL, &err);
            
            // OS X
#elif __APPLE__
            CGLContextObj kCGLContext = CGLGetCurrentContext();
            CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
            cl_context_properties contextProperties[] = {
                CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup,
                0
            };
            context = clCreateContext(contextProperties, 1, &did, NULL, NULL, &err);
            
            // Linux
#else
            cl_context_properties contextProperties[] = {
                CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
                CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
                CL_CONTEXT_PLATFORM, (cl_context_properties)pid,
                0
            };
            context = clCreateContext(contextProperties, 1, &did, NULL, NULL, &err);
            
#endif
            
            if (err == CL_SUCCESS) {
                ghoul::opengl::Texture* t = new ghoul::opengl::Texture(glm::size3_t(128,128,1),
                    ghoul::opengl::Texture::Format::RGBA, GL_RGBA, GL_FLOAT);
                if(t) {
                    t->uploadTexture();
                    err = 0;
                    cl_mem m;
#ifdef CL_VERSION_1_2
                    m = clCreateFromGLTexture(context, CL_MEM_READ_WRITE, t->type(), 0, *t, &err);
#else
                    if(t->type() == GL_TEXTURE_2D) {
                        m = clCreateFromGLTexture2D(context, CL_MEM_READ_WRITE, t->type(), 0, *t, &err);
                    } else if(t->type() == GL_TEXTURE_3D) {
                        m = clCreateFromGLTexture3D(context, CL_MEM_READ_WRITE, t->type(), 0, *t, &err);
                    }else {
                        LERROR("Texture is not a supported format for '"<< device.vendor() << "'");
                        m = 0;
                        err = -1;
                    }
#endif
                    clReleaseMemObject(m);
                    delete t;
                    
                    if (err != CL_SUCCESS) {
                        LDEBUG("Could not create texture for '"<< device.vendor() << "'");
                    }
                    
                    CLCommandQueue commands;
                    if(commands.initialize(context, did)) {
                        LDEBUG("Could initialize commands for '"<< device.vendor() << "'");
                        _context = std::make_shared<cl_context>(context);
                        _device = did;
                        _platform = pid;
                        successCreateContext = true;
                        break;
                    } else {
                        LDEBUG("Could not initialize commands for '"<< device.vendor() << "'");
                    }
                }
                
            } else {
                LDEBUG("Could not create context: " << getErrorString(err));
            }
            
            if( ! successCreateContext) {
                clReleaseContext(context);
            }
        }
        
    }
    
    if ( ! successCreateContext) {
        LFATAL("Could not find suitable devices");
        return false;
    }
    
    LDEBUG("Successfully created CL context from GL context");
    return true;
}

bool CLContext::isValidContext() const {
    return _context != 0;
}

cl_platform_id CLContext::platform() const {
    return _platform;
}
cl_device_id CLContext::device() const {
    return _device;
}

CLCommandQueue CLContext::createCommandQueue() {
    return CLCommandQueue(*_context,_device);
}

CLProgram CLContext::createProgram(const std::string& filename) {
    return CLProgram(this,filename);
}

cl_mem CLContext::createBuffer(cl_mem_flags memFlags, size_t size, void *data) {
    int err = 0;
    cl_mem mem = clCreateBuffer(*_context, memFlags, size, data, &err);
    if (err != 0) {
        LERROR("Could not create buffer: " << getErrorString(err));
    }
    return mem;
}

cl_mem CLContext::createTextureFromGLTexture(cl_mem_flags memFlags, ghoul::opengl::Texture& texture) {
    int err = 0;
    cl_mem mem = 0;
#ifdef CL_VERSION_1_2
    mem = clCreateFromGLTexture(*_context, memFlags, texture.type(), 0, texture, &err);
#else
    if(texture.type() == GL_TEXTURE_1D) { // OpenCL <1.2 doesn't support 1D textures,
    									  // trick it with   this hack   (sorry).
        mem = clCreateFromGLTexture2D(*_context, memFlags, GL_TEXTURE_2D, 0, texture, &err);
    } else if(texture.type() == GL_TEXTURE_2D) {
        mem = clCreateFromGLTexture2D(*_context, memFlags, texture.type(), 0, texture, &err);
    } else if(texture.type() == GL_TEXTURE_3D) {
        mem = clCreateFromGLTexture3D(*_context, memFlags, texture.type(), 0, texture, &err);
    }else {
        LERROR("Texture is not a supported format");
        mem = 0;
        err = -1;
    }
#endif
    if (err != CL_SUCCESS) {
        LERROR("Could not create texture: " << getErrorString(err));
        mem = 0;
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
    return *_context;
}
    
cl_context CLContext::operator()() const {
    return *_context;
}
cl_context& CLContext::operator()() {
    return *_context;
}

}
}
