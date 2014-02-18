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

cmake_minimum_required(VERSION 2.8.0)
project(Ghoul)

#SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows")
#SET(CMAKE_RELEASE_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows")
#SET(CMAKE_RELWITHDEBINFO_POSTFIX "rd" CACHE STRING "add a postfix, usually empty on windows")
#SET(CMAKE_MINSIZEREL_POSTFIX "s" CACHE STRING "add a postfix, usually empty on windows")
#${CMAKE_CONFIGURATION_TYPES}
#        SET(OSG_AGGRESSIVE_WARNING_FLAGS /W4 /wd4706 /wd4127 /wd4100)
#    IF(OSG_USE_AGGRESSIVE_WARNINGS)
#        # Add flags defined by OSG_AGGRESSIVE_WARNING_FLAGS if they aren't already there
#        FOREACH(flag ${OSG_AGGRESSIVE_WARNING_FLAGS})
#            IF(NOT CMAKE_CXX_FLAGS MATCHES "${flag}")
#                SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
#            ENDIF()
#        ENDFOREACH()
#    ELSE()
#        # Remove all flags considered aggresive
#        FOREACH(flag ${OSG_AGGRESSIVE_WARNING_FLAGS})
#            STRING(REGEX REPLACE "${flag}" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
#        ENDFOREACH()
#    ENDIF()

# Whole Program Optimization
# Inline function expansion
# omit frame pointer?
# function level linking
# enhanced instructed set
# hotpatchable image?
# precompiled header
# linker: show progress

mark_as_advanced(CMAKE_BACKWARDS_COMPATIBILITY)

if (POLICY CMP0011)
   cmake_policy(SET CMP0011 OLD) # or even better, NEW
endif (POLICY CMP0011)

if (NOT GHOUL_ROOT_DIR)
    set(GHOUL_ROOT_DIR ${PROJECT_SOURCE_DIR})
endif ()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/ext)

if (APPLE)
	add_definitions(-D__APPLE__)
	set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ ${CMAKE_CXX_FLAGS}")
else ()
    if (UNIX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -ggdb")
    endif ()
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

option(GHOUL_USE_DEVIL "Use the DevIL library" ON)
if (GHOUL_USE_DEVIL)
    add_definitions(-DGHOUL_USE_DEVIL)
    find_package(DevIL)
    include_directories(${IL_INCLUDE_DIR})
    set(DEVIL_LIBRARIES ${IL_LIBRARIES} ${ILU_LIBRARIES} ${ILUT_LIBRARIES})
endif ()

include_directories(${GHOUL_ROOT_DIR}/ext/lua/include)
include_directories(${GHOUL_ROOT_DIR}/ext/boost)

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
#   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-gnu")
endif ()

add_subdirectory(src)

add_subdirectory(${GHOUL_ROOT_DIR}/ext/lua)
