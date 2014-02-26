# FindopenCL - attempts to locate the OpenCL library.
#
# This module defines the following variables (on success):
# OPENCL_INCLUDE_DIRS - where to find OpenCL
# OPENCL_LIBRARIES - the names of the libraries needed by OpenCL
# OPENCL_FOUND - if the library was successfully located

if(WIN32)

endif(WIN32)

if (APPLE)
    find_library(FRAMEWORK_OpenCL
        NAMES OpenCL
        PATHS ${CMAKE_OSX_SYSROOT}/System/Library
        PATH_SUFFIXES Frameworks
        NO_DEFAULT_PATH
    )
    set(OPENCL_LIBRARY ${FRAMEWORK_OpenCL})
endif(APPLE)


set(OPENCL_INCLUDE_DIRECTORIES ${OPENCL_INCLUDES})
set(OPENCL_LIBRARIES ${OPENCL_LIBRARY} ${OPENCL_DEPENDENCIES})


# handle the QUIETLY and REQUIRED arguments and set OPENCL_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(OpenCL  DEFAULT_MSG
                                  OPENCL_LIBRARY)
