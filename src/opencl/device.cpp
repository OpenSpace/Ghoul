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

#include "opencl/device.h"

#include <ghoul/opencl/ghoul_cl.hpp>

namespace ghoul {
namespace opencl {

Device::Device(cl::Device* device): _device(device) {
    clearInformation();
}
Device::~Device() {}
    
bool Device::isInitialized() const {
    return _isInitialized;
}
    
void Device::fetchInformation() {
    if (isInitialized()) {
        clearInformation();
    }
    
    std::string         tmp_string;
    cl_uint             tmp_uint;
    cl_ulong            tmp_ulong;
    cl_device_fp_config tmp_cl_device_fp_config;
    cl_bool             tmp_bool;
    size_t              tmp_size_t;
    cl_device_exec_capabilities tmp_device_exec_capabilities;
    cl_device_mem_cache_type tmp_device_mem_cache_type;
    cl_device_local_mem_type tmp_device_local_mem_type;
    cl_device_id        tmp_device_id;
    cl_platform_id        tmp_platform_id;
    cl_command_queue_properties tmp_command_queue_properties;
    
    if(_device->getInfo(CL_DEVICE_NAME, &tmp_string) == CL_SUCCESS)
        _name = tmp_string;
    if(_device->getInfo(CL_DEVICE_VENDOR, &tmp_string) == CL_SUCCESS)
        _vendor = tmp_string;
    if(_device->getInfo(CL_DEVICE_PROFILE, &tmp_string) == CL_SUCCESS)
        _profile = tmp_string;
    if(_device->getInfo(CL_DEVICE_TYPE, &tmp_string) == CL_SUCCESS)
        _type = tmp_string;
    
    // native
    if(_device->getInfo(CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, &tmp_uint) == CL_SUCCESS)
        _nativeVectorWidthChar = tmp_uint;
    if(_device->getInfo(CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, &tmp_uint) == CL_SUCCESS)
        _nativeVectorWidthInt = tmp_uint;
    if(_device->getInfo(CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, &tmp_uint) == CL_SUCCESS)
        _nativeVectorWidthLong = tmp_uint;
    if(_device->getInfo(CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, &tmp_uint) == CL_SUCCESS)
        _nativeVectorWidthShort = tmp_uint;
    if(_device->getInfo(CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, &tmp_uint) == CL_SUCCESS)
        _nativeVectorWidthDouble = tmp_uint;
    if(_device->getInfo(CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, &tmp_uint) == CL_SUCCESS)
        _nativeVectorWidthHalf = tmp_uint;
    if(_device->getInfo(CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, &tmp_uint) == CL_SUCCESS)
        _nativeVectorWidthFloat = tmp_uint;

    // preferred
    if(_device->getInfo(CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, &tmp_uint) == CL_SUCCESS)
        _preferredVectorWidthChar = tmp_uint;
    if(_device->getInfo(CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, &tmp_uint) == CL_SUCCESS)
        _preferredVectorWidthInt = tmp_uint;
    if(_device->getInfo(CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, &tmp_uint) == CL_SUCCESS)
        _preferredVectorWidthLong = tmp_uint;
    if(_device->getInfo(CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, &tmp_uint) == CL_SUCCESS)
        _preferredVectorWidthShort = tmp_uint;
    if(_device->getInfo(CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, &tmp_uint) == CL_SUCCESS)
        _preferredVectorWidthDouble = tmp_uint;
    if(_device->getInfo(CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, &tmp_uint) == CL_SUCCESS)
        _preferredVectorWidthHalf = tmp_uint;
    if(_device->getInfo(CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, &tmp_uint) == CL_SUCCESS)
        _preferredVectorWidthFloat = tmp_uint;
    
    
    if(_device->getInfo(CL_DEVICE_ADDRESS_BITS, &tmp_uint) == CL_SUCCESS)
        _addressBits = tmp_uint;
    if(_device->getInfo(CL_DEVICE_ADDRESS_BITS, &tmp_string) == CL_SUCCESS)
        _builtInKernels = tmp_string;
    if(_device->getInfo(CL_DEVICE_COMPILER_AVAILABLE, &tmp_bool) == CL_SUCCESS)
        _compilerAvailable = tmp_bool;
    if(_device->getInfo(CL_DEVICE_DOUBLE_FP_CONFIG, &tmp_cl_device_fp_config) == CL_SUCCESS)
        _doubleFpConfig = tmp_cl_device_fp_config;
    if(_device->getInfo(CL_DEVICE_HALF_FP_CONFIG, &tmp_cl_device_fp_config) == CL_SUCCESS)
        _halfFpConfig = tmp_cl_device_fp_config;
    if(_device->getInfo(CL_DEVICE_SINGLE_FP_CONFIG, &tmp_cl_device_fp_config) == CL_SUCCESS)
        _singleFpConfig = tmp_cl_device_fp_config;
    
    
    if(_device->getInfo(CL_DEVICE_ENDIAN_LITTLE, &tmp_bool) == CL_SUCCESS)
        _endianLittle = tmp_bool;
    
    if(_device->getInfo(CL_DEVICE_EXTENSIONS, &tmp_string) == CL_SUCCESS)
        _extensions = tmp_string;
    
    if(_device->getInfo(CL_DEVICE_ERROR_CORRECTION_SUPPORT, &tmp_bool) == CL_SUCCESS)
        _errorCorrectionSupport = tmp_bool;
    if(_device->getInfo(CL_DEVICE_EXECUTION_CAPABILITIES, &tmp_device_exec_capabilities) == CL_SUCCESS)
        _executionCapabilities = tmp_device_exec_capabilities;

    if(_device->getInfo(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, &tmp_ulong) == CL_SUCCESS)
        _globalMemCacheSize = tmp_ulong;
    if(_device->getInfo(CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, &tmp_device_mem_cache_type) == CL_SUCCESS)
        _globalMemCacheType = tmp_device_mem_cache_type;
    
    
    if(_device->getInfo(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, &tmp_ulong) == CL_SUCCESS)
        _globalMemCacheCachelineSize = tmp_ulong;
    if(_device->getInfo(CL_DEVICE_GLOBAL_MEM_SIZE, &tmp_ulong) == CL_SUCCESS)
        _globalMemSize = tmp_ulong;
    
    
    if(_device->getInfo(CL_DEVICE_HOST_UNIFIED_MEMORY, &tmp_bool) == CL_SUCCESS)
        _hostUnifiedMemory = tmp_bool;
    
    
    if(_device->getInfo(CL_DEVICE_IMAGE_SUPPORT, &tmp_bool) == CL_SUCCESS)
        _imageSupport = tmp_bool;
    if(_device->getInfo(CL_DEVICE_IMAGE2D_MAX_WIDTH, &tmp_size_t) == CL_SUCCESS)
        _image2dMaxWidth = tmp_size_t;
    if(_device->getInfo(CL_DEVICE_IMAGE2D_MAX_HEIGHT, &tmp_size_t) == CL_SUCCESS)
        _image2dMaxHeight = tmp_size_t;
    if(_device->getInfo(CL_DEVICE_IMAGE3D_MAX_WIDTH, &tmp_size_t) == CL_SUCCESS)
        _image3dMaxWidth = tmp_size_t;
    if(_device->getInfo(CL_DEVICE_IMAGE3D_MAX_HEIGHT, &tmp_size_t) == CL_SUCCESS)
        _image3dMaxHeight = tmp_size_t;
    if(_device->getInfo(CL_DEVICE_IMAGE3D_MAX_DEPTH, &tmp_size_t) == CL_SUCCESS)
        _image3dMaxDepth = tmp_size_t;
    
    if(_device->getInfo(CL_DEVICE_LOCAL_MEM_TYPE, &tmp_device_local_mem_type) == CL_SUCCESS)
        _localMemType = tmp_device_local_mem_type;
    if(_device->getInfo(CL_DEVICE_LOCAL_MEM_SIZE, &tmp_ulong) == CL_SUCCESS)
        _localMemSize = tmp_ulong;
    
    if(_device->getInfo(CL_DEVICE_MAX_READ_IMAGE_ARGS, &tmp_uint) == CL_SUCCESS)
        _maxReadImageArgs = tmp_uint;
    if(_device->getInfo(CL_DEVICE_MAX_WRITE_IMAGE_ARGS, &tmp_uint) == CL_SUCCESS)
        _maxWriteImageArgs = tmp_uint;
    
    if(_device->getInfo(CL_DEVICE_MAX_CLOCK_FREQUENCY, &tmp_uint) == CL_SUCCESS)
        _maxClockFrequency = tmp_uint;
    if(_device->getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &tmp_uint) == CL_SUCCESS)
        _maxComputeUnits = tmp_uint;
    
    if(_device->getInfo(CL_DEVICE_MAX_CONSTANT_ARGS, &tmp_uint) == CL_SUCCESS)
        _maxConstantArgs = tmp_uint;
    if(_device->getInfo(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, &tmp_uint) == CL_SUCCESS)
        _maxConstantBufferSize = tmp_uint;
    
    if(_device->getInfo(CL_DEVICE_MAX_MEM_ALLOC_SIZE, &tmp_ulong) == CL_SUCCESS)
        _maxMemAllocSize = tmp_ulong;
    if(_device->getInfo(CL_DEVICE_MAX_PARAMETER_SIZE, &tmp_size_t) == CL_SUCCESS)
        _maxParameterSize = tmp_size_t;
    
    if(_device->getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, &tmp_size_t) == CL_SUCCESS)
        _maxWorkGroupSize = tmp_size_t;
    if(_device->getInfo(CL_DEVICE_MAX_SAMPLERS, &tmp_uint) == CL_SUCCESS)
        _maxSamplers = tmp_uint;
    
    if(_device->getInfo(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, &tmp_uint) == CL_SUCCESS)
        _maxWorkItemDimensions = tmp_uint;
    if(_device->getInfo(CL_DEVICE_MAX_WORK_ITEM_SIZES, &tmp_uint) == CL_SUCCESS)
        _maxWorkItemSizes = tmp_uint;
    
    if(_device->getInfo(CL_DEVICE_MEM_BASE_ADDR_ALIGN, &tmp_uint) == CL_SUCCESS)
        _memBaseAddrAlign = tmp_uint;
    
    
    if(_device->getInfo(CL_DEVICE_OPENCL_C_VERSION, &tmp_string) == CL_SUCCESS)
        _openCLCVersion = tmp_string;
    
    if(_device->getInfo(CL_DEVICE_PLATFORM, &tmp_platform_id) == CL_SUCCESS)
        _platform = tmp_platform_id;
    
    
    if(_device->getInfo(CL_DEVICE_QUEUE_PROPERTIES, &tmp_command_queue_properties) == CL_SUCCESS)
        _queueProperties = tmp_command_queue_properties;
    
    
    if(_device->getInfo(CL_DEVICE_VENDOR_ID, &tmp_uint) == CL_SUCCESS)
        _vendorId = tmp_uint;
    
    if(_device->getInfo(CL_DEVICE_VERSION, &tmp_string) == CL_SUCCESS)
        _version = tmp_string;

	// OpenCL 1.2 specifics
#ifdef CL_VERSION_1_2
    if(_device->getInfo(CL_DEVICE_IMAGE_MAX_ARRAY_SIZE, &tmp_size_t) == CL_SUCCESS)
        _imageMaxArraySize = tmp_size_t;
	if(_device->getInfo(CL_DEVICE_IMAGE_MAX_BUFFER_SIZE, &tmp_size_t) == CL_SUCCESS)
        _imageMaxBufferSize = tmp_size_t;
	if(_device->getInfo(CL_DEVICE_PARENT_DEVICE, &tmp_device_id) == CL_SUCCESS)
        _parentDevice = tmp_device_id;
	if(_device->getInfo(CL_DEVICE_PRINTF_BUFFER_SIZE, &tmp_size_t) == CL_SUCCESS)
        _printfBufferSize = tmp_size_t;
    if(_device->getInfo(CL_DEVICE_REFERENCE_COUNT, &tmp_uint) == CL_SUCCESS)
        _referenceCount = tmp_uint;
#endif	

    _isInitialized = true;
}
void Device::clearInformation() {
    
    _name = "";
    _vendor = "";
    _profile = "";
    _type = "";
    
    _nativeVectorWidthChar = 0;
    _nativeVectorWidthInt = 0;
    _nativeVectorWidthLong = 0;
    _nativeVectorWidthShort = 0;
    _nativeVectorWidthDouble = 0;
    _nativeVectorWidthHalf = 0;
    _nativeVectorWidthFloat = 0;
    
    _preferredVectorWidthChar = 0;
    _preferredVectorWidthInt = 0;
    _preferredVectorWidthLong = 0;
    _preferredVectorWidthShort = 0;
    _preferredVectorWidthDouble = 0;
    _preferredVectorWidthHalf = 0;
    _preferredVectorWidthFloat = 0;
    
    _addressBits = 0;
    _builtInKernels = "";
    _compilerAvailable = false;
    _doubleFpConfig = 0;
    _halfFpConfig = 0;
    _singleFpConfig = 0;
    
    _endianLittle = false;
    _extensions = "";
    _errorCorrectionSupport = false;
    _executionCapabilities = 0;
    
    _globalMemCacheSize = 0;
    _globalMemCacheType = 0;
    _globalMemCacheCachelineSize = 0;
    _globalMemSize = 0;
    
    _hostUnifiedMemory = 0;
    
    _imageMaxArraySize = 0;
    _imageMaxBufferSize = 0;
    
    _imageSupport = false;
    _image2dMaxWidth = 0;
    _image2dMaxHeight = 0;
    _image3dMaxWidth = 0;
    _image3dMaxHeight = 0;
    _image3dMaxDepth = 0;
    
    _localMemType = 0;
    _localMemSize = 0;
    
    _maxReadImageArgs = 0;
    _maxWriteImageArgs = 0;
    
    _maxClockFrequency = 0;
    _maxComputeUnits = 0;
    
    _maxConstantArgs = 0;
    _maxConstantBufferSize = 0;
    
    _maxMemAllocSize = 0;
    _maxParameterSize = 0;
    
    _maxSamplers = 0;
    _maxWorkGroupSize = 0;
    
    _maxWorkItemDimensions = 0;
    _maxWorkItemSizes = 0;
    
    _memBaseAddrAlign = 0;
    
    _openCLCVersion = "";
    _parentDevice = 0;
    
    _platform = 0;
    _printfBufferSize = 0;
    
    _queueProperties = 0;
    
    _referenceCount = 0;
    
    _vendorId = 0;
    
    _version = "";
    
    _isInitialized = false;
}

//
// operators
//
Device& Device::operator=(const Device& rhs) {
    if (this != &rhs) // protect against invalid self-assignment
    {
        *_device = *rhs._device;
        
        // Ugly version that refetches all information
        clearInformation();
        if(rhs.isInitialized())
            fetchInformation();
    }
    return *this;
}
    
Device& Device::operator=(const cl::Device& rhs) {
    *_device = rhs;
    
    // Ugly version that refetches all information
    clearInformation();
    if(isInitialized())
        fetchInformation();
    
    return *this;
}
    
cl_device_id Device::operator()() const {
    return _device->operator()();
}

cl_device_id& Device::operator()() {
    return _device->operator()();
}

//
// GET
//
std::string Device::name() const {
    return _name;
}
std::string Device::vendor() const {
    return _vendor;
}
std::string Device::profile() const {
    return _profile;
}
std::string Device::type() const {
    return _type;
}

// native
cl_uint Device::nativeVectorWidthChar() const {
    return _nativeVectorWidthChar;
}
cl_uint Device::nativeVectorWidthInt() const {
    return _nativeVectorWidthInt;
}
cl_uint Device::nativeVectorWidthLong() const {
    return _nativeVectorWidthLong;
}
cl_uint Device::nativeVectorWidthShort() const {
    return _nativeVectorWidthShort;
}
cl_uint Device::nativeVectorWidthDouble() const {
    return _nativeVectorWidthDouble;
}
cl_uint Device::nativeVectorWidthHalf() const {
    return _nativeVectorWidthHalf;
}
cl_uint Device::nativeVectorWidthFloat() const {
    return _nativeVectorWidthFloat;
}

// preferred
cl_uint Device::preferredVectorWidthChar() const {
    return _preferredVectorWidthChar;
}
cl_uint Device::preferredVectorWidthInt() const {
    return _preferredVectorWidthInt;
}
cl_uint Device::preferredVectorWidthLong() const {
    return _preferredVectorWidthLong;
}
cl_uint Device::preferredVectorWidthShort() const {
    return _preferredVectorWidthShort;
}
cl_uint Device::preferredVectorWidthDouble() const {
    return _preferredVectorWidthDouble;
}
cl_uint Device::preferredVectorWidthHalf() const {
    return _preferredVectorWidthHalf;
}
cl_uint Device::preferredVectorWidthFloat() const {
    return _preferredVectorWidthFloat;
}

cl_uint Device::addressBits() const {
    return _addressBits;
}
std::string Device::builtInKernels() const {
    return _builtInKernels;
}
cl_bool Device::compilerAvailable() const {
    return _compilerAvailable;
}
cl_device_fp_config Device::doubleFpConfig() const {
    return _doubleFpConfig;
}
cl_device_fp_config Device::halfFpConfig() const {
    return _halfFpConfig;
}
cl_device_fp_config Device::singleFpConfig() const {
    return _singleFpConfig;
}

cl_bool Device::endianLittle() const {
    return _endianLittle;
}
std::string Device::extensions() const {
    return _extensions;
}
cl_bool Device::errorCorrectionSupport() const {
    return _errorCorrectionSupport;
}
cl_device_exec_capabilities Device::executionCapabilities() const {
    return _executionCapabilities;
}

cl_ulong Device::globalMemCacheSize() const {
    return _globalMemCacheSize;
}
cl_device_mem_cache_type Device::globalMemCacheType() const {
    return _globalMemCacheType;
}
cl_ulong Device::globalMemCacheCachelineSize() const {
    return _globalMemCacheCachelineSize;
}
cl_ulong Device::globalMemSize() const {
    return _globalMemSize;
}

cl_bool Device::hostUnifiedMemory() const {
    return _hostUnifiedMemory;
}

size_t Device::imageMaxArraySize() const {
    return _imageMaxArraySize;
}
size_t Device::imageMaxBufferSize() const {
    return _imageMaxBufferSize;
}

cl_bool Device::imageSupport() const {
    return _imageSupport;
}
size_t Device::image2dMaxWidth() const {
    return _image2dMaxWidth;
}
size_t Device::image2dMaxHeight() const {
    return _image2dMaxHeight;
}
size_t Device::image3dMaxWidth() const {
    return _image3dMaxWidth;
}
size_t Device::image3dMaxHeight() const {
    return _image3dMaxHeight;
}
size_t Device::image3dMaxDepth() const {
    return _image3dMaxDepth;
}

cl_device_local_mem_type Device::localMemType() const {
    return _localMemType;
}
cl_ulong Device::localMemSize() const {
    return _localMemSize;
}

cl_uint Device::maxReadImageArgs() const {
    return _maxReadImageArgs;
}
cl_uint Device::maxWriteImageArgs() const {
    return _maxWriteImageArgs;
}

cl_uint Device::maxClockFrequency() const {
    return _maxClockFrequency;
}
cl_uint Device::maxComputeUnits() const {
    return _maxComputeUnits;
}

cl_uint Device::maxConstantArgs() const {
    return _maxConstantArgs;
}
cl_ulong Device::maxConstantBufferSize() const {
    return _maxConstantBufferSize;
}

cl_ulong Device::maxMemAllocSize() const {
    return _maxMemAllocSize;
}
size_t Device::maxParameterSize() const {
    return _maxParameterSize;
}

cl_uint Device::maxSamplers() const {
    return _maxSamplers;
}
size_t Device::maxWorkGroupSize() const {
    return _maxWorkGroupSize;
}

cl_uint Device::maxWorkItemDimensions() const {
    return _maxWorkItemDimensions;
}
size_t Device::maxWorkItemSizes() const {
    return _maxWorkItemSizes;
}

cl_uint Device::memBaseAddrAlign() const {
    return _memBaseAddrAlign;
}

std::string Device::openCLCVersion() const {
    return _openCLCVersion;
}
cl_device_id Device::parentDevice() const {
    return _parentDevice;
}

cl_platform_id Device::platform() const {
    return _platform;
}
size_t Device::printfBufferSize() const {
    return _printfBufferSize;
}

cl_command_queue_properties Device::queueProperties() const {
    return _queueProperties;
}

cl_uint Device::referenceCount() const {
    return _referenceCount;
}

cl_uint Device::vendorId() const {
    return _vendorId;
}

std::string Device::version() const {
    return _version;
}

}
}
