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

#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <ghoul/opencl/ghoul_cl.h>
#include <string>

namespace ghoul {
namespace opencl {

class Device {
public:
    Device(cl::Device* device);
    ~Device();
    
    bool isInitialized() const;
    
    void fetchInformation();
    void clearInformation();
    
    Device& operator=(const Device& rhs);
    Device& operator=(const cl::Device& rhs);
    cl_device_id operator()() const;
    cl_device_id& operator()();
    
    // names
    std::string name() const;
    std::string vendor() const;
    std::string profile() const;
    std::string type() const;
    
    // native
    cl_uint nativeVectorWidthChar() const;
    cl_uint nativeVectorWidthInt() const;
    cl_uint nativeVectorWidthLong() const;
    cl_uint nativeVectorWidthShort() const;
    cl_uint nativeVectorWidthDouble() const;
    cl_uint nativeVectorWidthHalf() const;
    cl_uint nativeVectorWidthFloat() const;
    
    // preferred
    cl_uint preferredVectorWidthChar() const;
    cl_uint preferredVectorWidthInt() const;
    cl_uint preferredVectorWidthLong() const;
    cl_uint preferredVectorWidthShort() const;
    cl_uint preferredVectorWidthDouble() const;
    cl_uint preferredVectorWidthHalf() const;
    cl_uint preferredVectorWidthFloat() const;
    
    cl_uint addressBits() const;
    std::string builtInKernels() const;
    cl_bool compilerAvailable() const;
    cl_device_fp_config doubleFpConfig() const;
    cl_device_fp_config halfFpConfig() const;
    cl_device_fp_config singleFpConfig() const;
    
    cl_bool endianLittle() const;
    std::string extensions() const;
    cl_bool errorCorrectionSupport() const;
    cl_device_exec_capabilities executionCapabilities() const;
    
    cl_ulong globalMemCacheSize() const;
    cl_device_mem_cache_type globalMemCacheType() const;
    cl_ulong globalMemCacheCachelineSize() const;
    cl_ulong globalMemSize() const;
    
    cl_bool hostUnifiedMemory() const;
    
    size_t imageMaxArraySize() const;
    size_t imageMaxBufferSize() const;
    
    cl_bool imageSupport() const;
    size_t image2dMaxWidth() const;
    size_t image2dMaxHeight() const;
    size_t image3dMaxWidth() const;
    size_t image3dMaxHeight() const;
    size_t image3dMaxDepth() const;
    
    cl_device_local_mem_type localMemType() const;
    cl_ulong localMemSize() const;
    
    cl_uint maxReadImageArgs() const;
    cl_uint maxWriteImageArgs() const;
    
    cl_uint maxClockFrequency() const;
    cl_uint maxComputeUnits() const;
    
    cl_uint maxConstantArgs() const;
    cl_ulong maxConstantBufferSize() const;
    
    cl_ulong maxMemAllocSize() const;
    size_t maxParameterSize() const;
    
    cl_uint maxSamplers() const;
    size_t maxWorkGroupSize() const;
    
    cl_uint maxWorkItemDimensions() const;
    size_t maxWorkItemSizes() const;
    
    cl_uint memBaseAddrAlign() const;
    
    std::string openCLCVersion() const;
    cl_device_id parentDevice() const;
    
    cl_platform_id platform() const;
    size_t printfBufferSize() const;
    
    cl_command_queue_properties queueProperties() const;
    
    cl_uint referenceCount() const;
    
    cl_uint vendorId() const;
    
    std::string version() const;

protected:

private:
    cl::Device* _device;
    
    bool _isInitialized;
    
    std::string _name;
	std::string _vendor;
	std::string _profile;
	std::string _type;
    cl_uint _nativeVectorWidthChar;
    cl_uint _nativeVectorWidthInt;
    cl_uint _nativeVectorWidthLong;
    cl_uint _nativeVectorWidthShort;
    cl_uint _nativeVectorWidthDouble;
    cl_uint _nativeVectorWidthHalf;
    cl_uint _nativeVectorWidthFloat;
    
    cl_uint _preferredVectorWidthChar;
    cl_uint _preferredVectorWidthInt;
    cl_uint _preferredVectorWidthLong;
    cl_uint _preferredVectorWidthShort;
    cl_uint _preferredVectorWidthDouble;
    cl_uint _preferredVectorWidthHalf;
    cl_uint _preferredVectorWidthFloat;
    
    cl_uint _addressBits;
    std::string _builtInKernels;
    cl_bool _compilerAvailable;
    cl_device_fp_config _doubleFpConfig;
	cl_device_fp_config _halfFpConfig;
	cl_device_fp_config _singleFpConfig;
    
    cl_bool _endianLittle;
    std::string _extensions;
    cl_bool _errorCorrectionSupport;
    cl_device_exec_capabilities _executionCapabilities;
    
    cl_ulong _globalMemCacheSize;
    cl_device_mem_cache_type _globalMemCacheType;
    cl_ulong _globalMemCacheCachelineSize;
	cl_ulong _globalMemSize;
    
    cl_bool _hostUnifiedMemory;
    
    size_t _imageMaxArraySize;
	size_t _imageMaxBufferSize;
    
    cl_bool _imageSupport;
    size_t  _image2dMaxWidth;
	size_t _image2dMaxHeight;
	size_t _image3dMaxWidth;
	size_t _image3dMaxHeight;
	size_t _image3dMaxDepth;
    
    cl_device_local_mem_type _localMemType;
    cl_ulong _localMemSize;
    
    cl_uint _maxReadImageArgs;
	cl_uint _maxWriteImageArgs;
    
    cl_uint _maxClockFrequency;
	cl_uint _maxComputeUnits;
    
    cl_uint _maxConstantArgs;
    cl_ulong _maxConstantBufferSize;
    
    cl_ulong _maxMemAllocSize;
    size_t _maxParameterSize;
    
    
    cl_uint _maxSamplers;
    size_t _maxWorkGroupSize;
    
    cl_uint _maxWorkItemDimensions;
    size_t _maxWorkItemSizes;
    
    cl_uint _memBaseAddrAlign;
    
    std::string _openCLCVersion;
    cl_device_id _parentDevice;
    
    cl_platform_id _platform;
    size_t _printfBufferSize;
    
    cl_command_queue_properties _queueProperties;
    
    cl_uint _referenceCount;
    
    cl_uint _vendorId;
    
    std::string _version;
    
    /*
    CURRENTLY NOT IMPLEMENTED
    
    CL_DEVICE_PREFERRED_INTEROP_USER_SYNC
    CL_DEVICE_AVAILABLE
    CL_DEVICE_PARTITION_AFFINITY_DOMAIN
    CL_DEVICE_PARTITION_MAX_SUB_DEVICES
    CL_DEVICE_PARTITION_{PROPERTIES, TYPE}
    CL_DEVICE_PROFILING_TIMER_RESOLUTION
    
    CL_DEVICE_MEM_BASE_ADDR_ALIGN, CL_DEVICE_OPENCL_C_VERSION, CL_DEVICE_PARENT_DEVICE, , , , CL_DEVICE_PLATFORM, CL_DEVICE_PRINTF_BUFFER_SIZE, , CL_DEVICE_QUEUE_PROPERTIES, CL_DEVICE_REFERENCE_COUNT,
    CL_DEVICE_VENDOR_ID, CL_{DEVICE, DRIVER}_VERSION
    */
};
    
} // namespace opencl
} // namespace ghoul

#endif // __DEVICE_H__
