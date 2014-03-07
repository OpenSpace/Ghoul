# FindopenCL - attempts to locate the OpenCL library.
#
# This module defines the following variables (on success):
# OPENCL_INCLUDE_DIRS - where to find OpenCL
# OPENCL_LIBRARIES - the names of the libraries needed by OpenCL
# OPENCL_FOUND - if the library was successfully located

if(WIN32)
	#find_path(OPENCL_INCLUDES CL/cl.h)
    
    # The AMD SDK currently installs both x86 and x86_64 libraries
    # This is only a hack to find out architecture
    if( ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "AMD64" )
    	set(OPENCL_LIB_DIR "$ENV{ATISTREAMSDKROOT}/lib/x86_64")
		set(OPENCL_LIB_DIR "$ENV{ATIINTERNALSTREAMSDKROOT}/lib/x86_64")
    else (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "AMD64")
    	set(OPENCL_LIB_DIR "$ENV{ATISTREAMSDKROOT}/lib/x86")
   		set(OPENCL_LIB_DIR "$ENV{ATIINTERNALSTREAMSDKROOT}/lib/x86")
    endif( ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "AMD64" )

    # find out if the user asked for a 64-bit build, and use the corresponding 
    # 64 or 32 bit NVIDIA library paths to the search:
    STRING(REGEX MATCH "Win64" ISWIN64 ${CMAKE_GENERATOR})
    if("${ISWIN64}" STREQUAL "Win64") 
    	find_library(OPENCL_LIBRARIES OpenCL.lib ${OPENCL_LIB_DIR} $ENV{CUDA_LIB_PATH} $ENV{CUDA_PATH}/lib/x64)
    else("${ISWIN64}" STREQUAL "Win64") 
    	find_library(OPENCL_LIBRARIES OpenCL.lib ${OPENCL_LIB_DIR} $ENV{CUDA_LIB_PATH} $ENV{CUDA_PATH}/lib/Win32)
    endif("${ISWIN64}" STREQUAL "Win64") 

    GET_FILENAME_COMPONENT(_OPENCL_INC_CAND ${OPENCL_LIB_DIR}/../../include ABSOLUTE)
    
    # On Win32 search relative to the library
    find_path(OPENCL_INCLUDE_DIRS CL/cl.h PATHS "${_OPENCL_INC_CAND}" $ENV{CUDA_INC_PATH} $ENV{CUDA_PATH}/include)
    find_path(_OPENCL_CPP_INCLUDE_DIRS CL/cl.hpp PATHS "${_OPENCL_INC_CAND}" $ENV{CUDA_INC_PATH} $ENV{CUDA_PATH}/include)
endif(WIN32)
if (APPLE)
	find_path(OPENCL_INCLUDE_DIRS OpenCL/cl.h)
    find_library(OPENCL_LIBRARIES
        NAMES OpenCL
        PATHS ${CMAKE_OSX_SYSROOT}/System/Library
        PATH_SUFFIXES Frameworks
        NO_DEFAULT_PATH
    )
#    set(OPENCL_LIBRARY ${FRAMEWORK_OpenCL})
endif(APPLE)
if(UNIX AND NOT APPLE)
	find_path( OPENCL_INCLUDE_DIRS CL/cl.h
		HINTS 	"/usr/include"
			"/usr/local/include")
	find_library(	OPENCL_LIBRARIES
		NAMES ${CMAKE_DYNAMIC_LIBRARY_PREFIX}OpenCL${CMAKE_DYNAMIC_LIBRARY_SUFFIX}
					HINTS 	"/lib"
							"/usr/lib"
							"/usr/local/lib"
	)
endif(UNIX AND NOT APPLE)

MARK_AS_ADVANCED(OPENCL_INCLUDE_DIRS OPENCL_LIBRARIES)


# handle the QUIETLY and REQUIRED arguments and set OPENCL_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(OPENCL  DEFAULT_MSG
                                  OPENCL_LIBRARIES OPENCL_INCLUDE_DIRS)
if(NOT OPENCL_FOUND) 
    MESSAGE(FATAL_ERROR "OpenCL not found!")
endif(NOT OPENCL_FOUND)
