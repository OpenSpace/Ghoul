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

#include <ghoul/opencl/clcommandqueue.h>

#include <ghoul/opencl/ghoul_cl.hpp>
#include <ghoul/opencl/platform.h>
#include <ghoul/opencl/device.h>
#include <ghoul/opencl/clkernel.h>
#include <ghoul/opencl/clutil.h>
#include <ghoul/opencl/clworksize.h>

#include <ghoul/logging/logmanager.h>
#include <ghoul/filesystem/filesystem.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>

namespace {
    std::string _loggerCat = "CLCommands";
}

namespace ghoul {
namespace opencl {

CLCommandQueue::CLCommandQueue(): _commands(0) {}

CLCommandQueue::CLCommandQueue(cl_context context, cl_device_id device): _commands(0) {
    if( ! initialize(context, device))
        LWARNING("Command queue not initialized");
}

bool CLCommandQueue::initialize(cl_context context, cl_device_id device) {
    int err = 0;
    _commands = std::make_shared<cl_command_queue>(clCreateCommandQueue(context, device, 0, &err));
    
    if (err != CL_SUCCESS) {
        LFATAL("Could not create program queue: " << getErrorString(err));
        _commands = 0;
        return false;
    }
    return true;
}

CLCommandQueue::~CLCommandQueue() {
    if (_commands != 0 && _commands.unique()) {
        clReleaseCommandQueue(*_commands);
    }
}

void CLCommandQueue::enqueueKernelBlocking(const CLKernel& kernel, const CLWorkSize& ws)  {
    int err = 0;
    err = clEnqueueNDRangeKernel(*_commands, kernel(), ws.dimensions(), NULL, ws.global(), ws.local(), 0, NULL, NULL);
    if (err != 0) {
        LFATAL("Could not run kernel: " << getErrorString(err));
    }
}

void CLCommandQueue::enqueueReadBufferBlocking(cl_mem buffer, size_t size, void* data) {
    int err = 0;
    err = clEnqueueReadBuffer(*_commands, buffer, CL_TRUE, 0, size, data, 0, NULL, NULL );
    if (err != 0) {
        LFATAL("Could not read buffer: " << getErrorString(err));
    }
}
    
void CLCommandQueue::enqueueWriteBufferBlocking(cl_mem buffer, size_t size, void* data) {
    int err = 0;
    err = clEnqueueWriteBuffer(*_commands, buffer, CL_TRUE, 0, size, data, 0, NULL, NULL);
    if (err != 0) {
        LFATAL("Could not write buffer: " << getErrorString(err));
    }
}

cl_event CLCommandQueue::enqueueKernelNonBlocking(const CLKernel& kernel, const CLWorkSize& ws)  {
    int err = 0;
    cl_event event = 0;
    err = clEnqueueNDRangeKernel(*_commands, kernel(), ws.dimensions(), NULL, ws.global(), ws.local(), 0, NULL, &event);
    if (err != 0) {
        LFATAL("Could not run kernel: " << getErrorString(err));
    }
    return event;
}

cl_event CLCommandQueue::enqueueReadBufferNonBlocking(cl_mem buffer, size_t size, void* data) {
    int err = 0;
    cl_event event = 0;
    err = clEnqueueReadBuffer(*_commands, buffer, CL_FALSE, 0, size, data, 0, NULL, &event );
    if (err != 0) {
        LFATAL("Could not read buffer: " << getErrorString(err));
    }
    return event;
}

cl_event CLCommandQueue::enqueueAcquireGLObjects(cl_mem glObject) {
	int err = 0;
	cl_event event = 0;
	err = clEnqueueAcquireGLObjects(*_commands, 1, &glObject, 0, NULL, &event);
	if (err != 0) {
		LFATAL("Could not aquire GL object: " << getErrorString(err));
	}
	return event;
}

cl_event CLCommandQueue::enqueueReleaseGLObjects(cl_mem glObject) {
	int err = 0;
	cl_event event = 0;
	err = clEnqueueReleaseGLObjects(*_commands, 1, &glObject, 0, NULL, &event);
	if (err != 0) {
		LFATAL("Could not release GL object: " << getErrorString(err));
	}
	return event;
}

cl_event CLCommandQueue::enqueueAcquireGLObjects(std::vector<cl_mem> glObjects) {
	int err = 0;
	cl_event event = 0;

	if (glObjects.size() > 0) {
		err = clEnqueueAcquireGLObjects(*_commands, glObjects.size(), &glObjects[0], 0, NULL, &event);
		if (err != 0) {
			LFATAL("Could not aquire GL object: " << getErrorString(err));
		}
	}
	return event;
}

cl_event CLCommandQueue::enqueueReleaseGLObjects(std::vector<cl_mem> glObjects) {
	int err = 0;
	cl_event event = 0;

	if (glObjects.size() > 0) {
		err = clEnqueueReleaseGLObjects(*_commands, glObjects.size(), &glObjects[0], 0, NULL, &event);
		if (err != 0) {
			LFATAL("Could not aquire GL object: " << getErrorString(err));
		}
	}
	return event;
}

void CLCommandQueue::finish() {
    clFinish(*_commands);
}

CLCommandQueue::operator cl_command_queue() {
    return *_commands;
}


}
}
