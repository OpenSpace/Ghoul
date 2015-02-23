#########################################################################################
#                                                                                       #
# GHOUL                                                                                 #
#                                                                                       #
# Copyright (c) 2012 Alexander Bock                                                     #
#                                                                                       #
# Permission is hereby granted, free of charge, to any person obtaining a copy of this  #
# software and associated documentation files (the "Software"), to deal in the Software #
# without restriction, including without limitation the rights to use, copy, modify,    #
# merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    #
# permit persons to whom the Software is furnished to do so, subject to the following   #
# conditions:                                                                           #
#                                                                                       #
# The above copyright notice and this permission notice shall be included in all copies #
# or substantial portions of the Software.                                              #
#                                                                                       #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   #
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         #
# PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    #
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  #
# CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  #
# OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         #
#########################################################################################

# @TODO Enable whole program optimization ---abock
# @TODO Enable inline function expansion ---abock
# @TODO Check/enable function level linking ---abock
# @TODO Enable precompiled header ---abock

cmake_minimum_required(VERSION 2.8.0)
project(Ghoul)

mark_as_advanced(CMAKE_BACKWARDS_COMPATIBILITY)

if (POLICY CMP0011)
   cmake_policy(SET CMP0011 OLD) # or even better, NEW
endif ()

if (NOT GHOUL_ROOT_DIR)
    set(GHOUL_ROOT_DIR ${PROJECT_SOURCE_DIR})
endif ()

add_definitions(-DGHOUL_ROOT_DIR="${GHOUL_ROOT_DIR}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/ext)

if (APPLE)
	add_definitions(-D__APPLE__)
	set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ ${CMAKE_CXX_FLAGS}")
else ()
    if (UNIX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -ggdb")
    endif ()
endif ()

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif ()

######
# Options
######

if (WIN32)
    option(GHOUL_USE_WMI "Use Windows Management Instrumentation" ON)
    if (GHOUL_USE_WMI)
        add_definitions(-DGHOUL_USE_WMI -D_WIN32_DCOM)
        set(GHOUL_DEPENDENCIES ${GHOUL_DEPENDENCIES} "WbemUuid.lib")
    endif ()
endif ()

######
# General dependencies
######
set(GLM_ROOT_DIR "${GHOUL_ROOT_DIR}/ext/glm")
find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})
add_definitions(-DGLM_SWIZZLE)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
if (NOT GHOUL_USE_DYNAMIC_GLEW)
    message("Using GLEW as a static library")
    add_definitions(-DGLEW_STATIC)
endif ()

find_package(LZ4 REQUIRED)
include_directories(${LZ4_INCLUDE_DIR})
set(GHOUL_DEPENDENCIES ${GHOUL_DEPENDENCIES} ${LZ4_LIBRARIES})

find_package(DevIL)
if (IL_FOUND)
    option(GHOUL_USE_DEVIL "Use the DevIL library" ON)
    if (GHOUL_USE_DEVIL)
        add_definitions(-DGHOUL_USE_DEVIL)
        set(DevIL_USE_STATIC_LIBS OFF)
        include_directories(${IL_INCLUDE_DIR})
        set(DEVIL_LIBRARIES ${IL_LIBRARIES} ${ILU_LIBRARIES} ${ILUT_LIBRARIES})
    endif ()
endif ()

find_package(FreeImage)
if (FREEIMAGE_FOUND)
    option(GHOUL_USE_FREEIMAGE "Use the FreeImage library" ON)
    if (GHOUL_USE_FREEIMAGE)
        add_definitions(-DGHOUL_USE_FREEIMAGE)
        include_directories(${FREEIMAGE_INCLUDE_PATH})
        set(GHOUL_DEPENDENCIES ${GHOUL_DEPENDENCIES} ${FREEIMAGE_LIBRARIES})
    endif()
endif ()

if (NOT GHOUL_USE_DEVIL AND NOT GHOUL_USE_FREEIMAGE) 
    message(FATAL_ERROR "Need to build with DevIL or FreeImage")
endif()

include_directories(${GHOUL_ROOT_DIR}/ext/lua/include)
set(BOOST_ROOT "${GHOUL_ROOT_DIR}/ext/boost")
set(Boost_USE_STATIC_LIBS ON)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
set(GHOUL_DEPENDENCIES ${GHOUL_DEPENDENCIES} ${Boost_LIBRARIES})

if (APPLE)
    include_directories(/Developer/Headers/FlatCarbon)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    find_library(CARBON_LIBRARY Carbon)
    find_library(COCOA_LIBRARY Carbon)
    find_library(APP_SERVICES_LIBRARY ApplicationServices)
    mark_as_advanced(CARBON_LIBRARY COCOA_LIBRARY APP_SERVICES_LIBRARY)
    set(MAC_LIBS
        ${CARBON_LIBRARY}
        ${COREFOUNDATION_LIBRARY}
        ${COCOA_LIBRARY}
        ${APP_SERVICES_LIBRARY})
endif()

if (WIN32)
    set(GHOUL_DEPENDENCIES ${GHOUL_DEPENDENCIES} shlwapi)
endif()

# tinyobjloader
set(GHOUL_EXT_TINYOBJ_SOURCE
    ${GHOUL_ROOT_DIR}/ext/tinyobjloader/tiny_obj_loader.h
    ${GHOUL_ROOT_DIR}/ext/tinyobjloader/tiny_obj_loader.cc
    )
add_library(tinyobjloader ${GHOUL_EXT_TINYOBJ_SOURCE})
set(GHOUL_DEPENDENCIES ${GHOUL_DEPENDENCIES} tinyobjloader)

option(GHOUL_HAVE_TESTS "Activate the unit tests" ON)
if (GHOUL_HAVE_TESTS)
    add_definitions(-DGHOUL_HAVE_TESTS)
    add_definitions(-DGTEST_HAS_TR1_TUPLE=0)
    add_definitions(-DGTEST_HAS_GLOBAL_STRING=0)
    add_subdirectory(${GHOUL_ROOT_DIR}/ext/gtest)
    
    include_directories("${GHOUL_ROOT_DIR}/ext/gtest/include")
    include_directories("${GHOUL_ROOT_DIR}")
    include_directories("${GHOUL_ROOT_DIR}/include")
    file(GLOB_RECURSE GHOUL_TEST_FILES ${GHOUL_ROOT_DIR}/tests/*.inl)
    
    add_executable(GhoulTest ${GHOUL_ROOT_DIR}/tests/main.cpp ${GHOUL_TEST_FILES})
    target_link_libraries(GhoulTest gtest Ghoul Lua ${MAC_LIBS})
    # set_target_properties(
endif ()

#####
# Platform-dependent Settings
#####
if (APPLE)
    add_definitions(-D__gl_h_)
    add_definitions(-Wno-write-strings)
endif ()

add_subdirectory(src)
add_subdirectory(${GHOUL_ROOT_DIR}/ext/lua)

#####
# User macros
#####
macro(GhoulCopySharedLibraries TARGET_EXECUTABLE)

    # Windows DLL
    if (WIN32)

        # DevIL
        if(GHOUL_USE_DEVIL)
            if (CMAKE_CL_64)
                set(DEVIL_DLL_DIR ${GHOUL_ROOT_DIR}/ext/il/lib/win64)
            else (CMAKE_CL_64)
                set(DEVIL_DLL_DIR ${GHOUL_ROOT_DIR}/ext/il/lib/win32)
            endif (CMAKE_CL_64)
            set(SHARED_LIBS ${SHARED_LIBS} ${DEVIL_DLL_DIR}/DevIL.dll)
            set(SHARED_LIBS ${SHARED_LIBS} ${DEVIL_DLL_DIR}/ILU.dll)
            set(SHARED_LIBS ${SHARED_LIBS} ${DEVIL_DLL_DIR}/ILUT.dll)
        endif(GHOUL_USE_DEVIL)

        # FreeImage
        if(GHOUL_USE_FREEIMAGE)
            if (CMAKE_CL_64)
                set(FREEIMAGE_DLL_DIR ${GHOUL_ROOT_DIR}/ext/freeimage/lib/win64)
            else (CMAKE_CL_64)
                set(FREEIMAGE_DLL_DIR ${GHOUL_ROOT_DIR}/ext/freeimage/lib/win32)
            endif (CMAKE_CL_64)
            set(SHARED_LIBS ${SHARED_LIBS} ${FREEIMAGE_DLL_DIR}/FreeImage.dll)
        endif(GHOUL_USE_FREEIMAGE)

        # Add the copy command
        foreach( file_i ${SHARED_LIBS})
            add_custom_command(TARGET ${TARGET_EXECUTABLE} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${file_i}"
            $<TARGET_FILE_DIR:${TARGET_EXECUTABLE}>)
        endforeach( file_i )
    endif(WIN32)

endmacro(GhoulCopySharedLibraries)