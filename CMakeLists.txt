#########################################################################################
#                                                                                       #
# GHOUL                                                                                 #
#                                                                                       #
# Copyright (c) 2012-2015                                                               #
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

cmake_minimum_required(VERSION 3.0)

#set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo")
set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo CACHE TYPE INTERNAL FORCE)
mark_as_advanced(CMAKE_CONFIGURATION_TYPES)
mark_as_advanced(CMAKE_INSTALL_PREFIX)

project(Ghoul)
message(STATUS "Generating Ghoul project")

set(GHOUL_ROOT_DIR ${PROJECT_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${GHOUL_ROOT_DIR}/ext)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${GHOUL_ROOT_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${GHOUL_ROOT_DIR}/bin")

#############################
# Including the source files (with source groups already defined)
#############################
include(src/CMakeLists.txt)

#############################
# Declare the library
#############################
add_library(Ghoul STATIC ${GHOUL_SOURCE} ${GHOUL_HEADER})
target_include_directories(Ghoul PUBLIC ${PROJECT_SOURCE_DIR}/include)

#############################
# Options
#############################
if (WIN32)
    option(GHOUL_USE_WMI "Use Windows Management Instrumentation" ON)
    if (GHOUL_USE_WMI)
        target_compile_definitions(Ghoul PUBLIC "GHOUL_USE_WMI" "_WIN32_DCOM") # Is _WIN32_DCOM necessary? ---abock
        target_link_libraries(Ghoul "WbemUuid.lib" "shlwapi")
#         set(GHOUL_DEPENDENCIES ${GHOUL_DEPENDENCIES} "WbemUuid.lib")
    endif ()
endif ()

#############################
# Compile settings
#############################
if (MSVC)
    target_compile_options(Ghoul PRIVATE "/MP" "/W4" "/wd4201" "/wd4127")
elseif (APPLE)
    target_compile_options(Ghoul PUBLIC "-std=c++11" "-stdlib=libc++" "-Wno-write-strings" "-Wall")
    target_compile_definitions(Ghoul PUBLIC "__APPLE__" "__gl_h_") # Is it set automatically? --- abock

    target_include_directories(Ghoul PUBLIC "/Developer/Headers/FlatCarbon")
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    find_library(CARBON_LIBRARY Carbon)
    find_library(COCOA_LIBRARY Carbon)
    find_library(APP_SERVICES_LIBRARY ApplicationServices)
    mark_as_advanced(CARBON_LIBRARY COCOA_LIBRARY APP_SERVICES_LIBRARY)
    target_link_libraries(Ghoul
        ${CARBON_LIBRARY}
        ${COREFOUNDATION_LIBRARY}
        ${COCOA_LIBRARY}
        ${APP_SERVICES_LIBRARY}
    )
elseif (UNIX) # The order is important as UNIX is also true on MacOS
    target_compile_options(Ghoul PUBLIC "-std=c++0x" "-ggdb" "-Wall" "-Wno-long-long" "-pedantic" "-Wextra") # Do we want to make GDB dependent on debug build? ---abock
endif ()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    if (MSVC OR XCODE)
        target_compile_definitions(Ghoul PRIVATE $<$<CONFIG:Debug>:GHL_DEBUG>)
    else () 
        target_compile_definitions(Ghoul PRIVATE "GHL_DEBUG" "_DEBUG")
    endif (MSVC OR XCODE)
endif()

#############################
# Dependencies
#############################
# GLM
set(GLM_ROOT_DIR "${GHOUL_ROOT_DIR}/ext/glm")
find_package(GLM REQUIRED)
target_include_directories(Ghoul SYSTEM PUBLIC ${GLM_INCLUDE_DIRS})
target_compile_definitions(Ghoul PUBLIC "GLM_SWIZZLE")

# OpenGL
find_package(OpenGL REQUIRED)
target_include_directories(Ghoul SYSTEM PUBLIC ${OPENGL_INCLUDE_DIR})
target_link_libraries(Ghoul ${OPENGL_LIBRARIES})

# GLEW
find_package(GLEW REQUIRED)
target_include_directories(Ghoul SYSTEM PUBLIC ${GLEW_INCLUDE_DIRS})
target_compile_definitions(Ghoul PUBLIC "GLEW_STATIC")
target_link_libraries(Ghoul ${GLEW_LIBRARIES})

# LZ4
find_package(LZ4 REQUIRED)
target_include_directories(Ghoul SYSTEM PUBLIC ${LZ4_INCLUDE_DIR})
target_link_libraries(Ghoul ${LZ4_LIBRARIES})

# Boost
set(BOOST_ROOT "${GHOUL_ROOT_DIR}/ext/boost")
find_package(Boost REQUIRED)
target_include_directories(Ghoul SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
# set(GHOUL_DEPENDENCIES ${GHOUL_DEPENDENCIES} ${Boost_LIBRARIES})

# Lua
add_subdirectory(ext/lua)
target_include_directories(Ghoul PUBLIC "ext/lua/src")
target_link_libraries(Ghoul Lua)
if (MSVC)
    target_compile_options(Lua PUBLIC "/W0" "/MP")
else ()
    target_compile_options(Lua PUBLIC "-w" "-Wno-deprecated-declarations")
endif ()


# tinyobjloader
add_subdirectory(ext/tinyobjloader)
target_link_libraries(Ghoul tinyobjloader)
target_include_directories(Ghoul SYSTEM PUBLIC "${GHOUL_ROOT_DIR}/ext/tinyobjloader")

# DevIL
find_package(DevIL)
if (IL_FOUND)
    option(GHOUL_USE_DEVIL "Use the DevIL library" ON)
    if (GHOUL_USE_DEVIL)
        target_compile_definitions(Ghoul PUBLIC "GHOUL_USE_DEVIL")
        # set(DevIL_USE_STATIC_LIBS OFF)
        target_include_directories(Ghoul PUBLIC ${IL_INCLUDE_DIR})
        target_link_libraries(Ghoul ${IL_LIBRARIES} ${ILU_LIBRARIES} ${ILUT_LIBRARIES})
    endif ()
endif ()

# FreeImage
find_package(FreeImage)
if (FREEIMAGE_FOUND)
    option(GHOUL_USE_FREEIMAGE "Use the FreeImage library" ON)
    if (GHOUL_USE_FREEIMAGE)
        target_compile_definitions(Ghoul PUBLIC "GHOUL_USE_FREEIMAGE")
        target_include_directories(Ghoul PUBLIC ${FREEIMAGE_INCLUDE_PATH})
        target_link_libraries(Ghoul ${FREEIMAGE_LIBRARIES})
    endif ()
endif ()

# SOIL
find_package(SOIL)
if (SOIL_FOUND)
    option(GHOUL_USE_SOIL "Use the SOIL library" ON)
    if (GHOUL_USE_SOIL)
        target_compile_definitions(Ghoul PUBLIC "GHOUL_USE_SOIL")
        target_include_directories(Ghoul PUBLIC ${SOIL_INCLUDE_DIRS})
        target_link_libraries(Ghoul ${SOIL_LIBRARIES})
    endif ()
endif ()

if (NOT GHOUL_USE_DEVIL AND NOT GHOUL_USE_FREEIMAGE AND NOT GHOUL_USE_SOIL)
    message(FATAL_ERROR "Ghoul needs to be built with DevIL, FreeImage, or SoIL")
endif ()


#############################
# Other applications
#############################
option(GHOUL_HAVE_TESTS "Activate the unit tests" OFF)
if (GHOUL_HAVE_TESTS)
    file(GLOB_RECURSE GHOUL_TEST_FILES ${GHOUL_ROOT_DIR}/tests/*.inl)
    add_executable(GhoulTest ${GHOUL_ROOT_DIR}/tests/main.cpp ${GHOUL_TEST_FILES})
    target_compile_definitions(GhoulTest PUBLIC
        "DGHOUL_HAVE_TESTS"
        "DGTEST_HAS_TR1_TUPLE=0"
        "DGTEST_HAS_GLOBAL_STRING=0"
    )
    target_include_directories(GhoulTest PUBLIC "${GHOUL_ROOT_DIR}/ext/gtest/include" "${GHOUL_ROOT_DIR}")
    add_subdirectory(${GHOUL_ROOT_DIR}/ext/gtest)
    target_link_libraries(GhoulTest gtest Ghoul Lua)

    if (APPLE)
        target_link_libraries(GhoulTest
            ${CARBON_LIBRARY}
            ${COREFOUNDATION_LIBRARY}
            ${COCOA_LIBRARY}
            ${APP_SERVICES_LIBRARY}
        )
    endif ()
endif ()
