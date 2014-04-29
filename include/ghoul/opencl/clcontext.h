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

#ifndef __CLCONTEXT_H__
#define __CLCONTEXT_H__

#include <ghoul/opencl/ghoul_cl.h>
#include <ghoul/opencl/device.h>
#include <ghoul/opengl/texture.h>
#include <memory> 

namespace ghoul {
namespace opencl {
    
class CLCommandQueue;
class CLProgram;

class CLContext {
public:
    CLContext();
    ~CLContext();
    
    bool createContextFromDevice(Device* device);
    bool createContextFromGLContext();
    
    bool isValidContext() const;
    
    cl_platform_id platform() const;
    cl_device_id device() const;
    
    CLCommandQueue createCommandQueue();
    CLProgram createProgram(const std::string& filename);
    
    cl_mem createBuffer(cl_mem_flags memFlags, size_t size, void *data = NULL);
    cl_mem createTextureFromGLTexture(cl_mem_flags memFlags, ghoul::opengl::Texture& texture);
    
    CLContext& operator=(const CLContext& rhs);
    operator cl_context() const;
    cl_context operator()() const;
    cl_context& operator()();
    
private:
    std::shared_ptr<cl_context> _context;
    //cl_context _context;
    cl_platform_id _platform;
    cl_device_id _device;
    
}; // class CLContext
    
} // namespace opencl
} // namespace ghoul

#endif