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

# @TODO Enable whole program optimization ---abock
# @TODO Enable inline function expansion ---abock
# @TODO Check/enable function level linking ---abock
# @TODO Enable precompiled header ---abock

cmake_minimum_required(VERSION 3.0)

project(Ghoul)
message(STATUS "Generating Ghoul project")

set(GHOUL_ROOT_DIR ${PROJECT_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${GHOUL_ROOT_DIR}/ext)

# Including the source files (with source groups already defined)
include(src/CMakeLists.txt)

# Options
if (WIN32)
    option(GHOUL_USE_WMI "Use Windows Management Instrumentation" ON)
endif()

add_library(Ghoul STATIC ${GHOUL_SOURCE} ${GHOUL_HEADER})
target_include_directories(Ghoul PUBLIC ${PROJECT_SOURCE_DIR}/include)

if (MSVC)
    target_compile_options(Ghoul PRIVATE "/MP")

    if (GHOUL_USE_WMI)
        target_compile_definitions(Ghoul PUBLIC "GHOUL_USE_WMI" "_WIN32_DCOM") # Is _WIN32_DCOM necessary? ---abock
        target_link_libraries(Ghoul "WbemUuid.lib" "shlwapi")
#         set(GHOUL_DEPENDENCIES ${GHOUL_DEPENDENCIES} "WbemUuid.lib")
    endif()
elseif (APPLE)
    target_compile_options(Ghoul PUBLIC "-std=c++11 -stdlib=libc++")
    target_compile_definitions(Ghoul PUBLIC "__APPLE__") # Is it set automatically? --- abock

    include_directories(/Developer/Headers/FlatCarbon)
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
    target_compile_options(Ghoul PUBLIC "-std=c++0x -ggdb") # Do we want to make GDB dependent on debug build? ---abock
endif()


# GLM
set(GLM_ROOT_DIR "${GHOUL_ROOT_DIR}/ext/glm")
find_package(GLM REQUIRED)
target_include_directories(Ghoul PUBLIC ${GLM_INCLUDE_DIRS})
target_compile_definitions(Ghoul PUBLIC "GLM_SWIZZLE")

# OpenGL
find_package(OpenGL REQUIRED)
target_include_directories(Ghoul PUBLIC ${OPENGL_INCLUDE_DIR})

# GLEW
find_package(GLEW REQUIRED)
target_include_directories(Ghoul PUBLIC ${GLEW_INCLUDE_DIRS})
target_link_libraries(Ghoul ${GLEW_LIBRARIES})

# LZ4
find_package(LZ4 REQUIRED)
target_include_directories(Ghoul PUBLIC ${LZ4_INCLUDE_DIR})
target_link_libraries(Ghoul ${LZ4_LIBRARIES})

# Boost
set(BOOST_ROOT "${GHOUL_ROOT_DIR}/ext/boost")
find_package(Boost REQUIRED)
target_include_directories(Ghoul PUBLIC ${Boost_INCLUDE_DIRS})
# set(GHOUL_DEPENDENCIES ${GHOUL_DEPENDENCIES} ${Boost_LIBRARIES})

# Lua
add_subdirectory(ext/lua)

# tinyobjloader
add_subdirectory(ext/tinyobjloader)
# # tinyobjloader
# set(GHOUL_EXT_TINYOBJ_SOURCE
#     ${GHOUL_ROOT_DIR}/ext/tinyobjloader/tiny_obj_loader.h
#     ${GHOUL_ROOT_DIR}/ext/tinyobjloader/tiny_obj_loader.cc
#     )
# add_library(tinyobjloader ${GHOUL_EXT_TINYOBJ_SOURCE})
# set(GHOUL_DEPENDENCIES ${GHOUL_DEPENDENCIES} tinyobjloader)


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
    endif()
endif ()

# SOIL
find_package(SOIL)
if (SOIL_FOUND)
    option(GHOUL_USE_SOIL "Use the SOIL library" ON)
    if (GHOUL_USE_SOIL)
        target_compile_definitions(Ghoul PUBLIC "GHOUL_USE_SOIL")
        target_include_directories(Ghoul PUBLIC ${SOIL_INCLUDE_DIRS})
        target_link_libraries(Ghoul ${SOIL_LIBRARIES})
    endif()
endif()

if (NOT GHOUL_USE_DEVIL AND NOT GHOUL_USE_FREEIMAGE AND NOT GHOUL_USE_SOIL)
    message(FATAL_ERROR "Ghoul needs to be built with DevIL, FreeImage, or SoIL")
endif()




# option(GHOUL_HAVE_TESTS "Activate the unit tests" ON)
# if (GHOUL_HAVE_TESTS)
#     add_definitions(-DGHOUL_HAVE_TESTS)
#     add_definitions(-DGTEST_HAS_TR1_TUPLE=0)
#     add_definitions(-DGTEST_HAS_GLOBAL_STRING=0)
#     add_subdirectory(${GHOUL_ROOT_DIR}/ext/gtest)
    
#     include_directories("${GHOUL_ROOT_DIR}/ext/gtest/include")
#     include_directories("${GHOUL_ROOT_DIR}")
#     include_directories("${GHOUL_ROOT_DIR}/include")
#     file(GLOB_RECURSE GHOUL_TEST_FILES ${GHOUL_ROOT_DIR}/tests/*.inl)
    
#     add_executable(GhoulTest ${GHOUL_ROOT_DIR}/tests/main.cpp ${GHOUL_TEST_FILES})
#     target_link_libraries(GhoulTest gtest Ghoul Lua ${MAC_LIBS})
# endif ()

# #####
# # Platform-dependent Settings
# #####
# if (APPLE)
#     add_definitions(-D__gl_h_)
#     add_definitions(-Wno-write-strings)
# endif ()

# add_subdirectory(src)
# add_subdirectory(${GHOUL_ROOT_DIR}/ext/lua)

# #####
# # User macros
# #####
# macro(GhoulCopySharedLibraries TARGET_EXECUTABLE)

#     # Windows DLL
#     if (WIN32)

#         # DevIL
#         if(GHOUL_USE_DEVIL)
#             if (CMAKE_CL_64)
#                 set(DEVIL_DLL_DIR ${GHOUL_ROOT_DIR}/ext/il/lib/win64)
#             else (CMAKE_CL_64)
#                 set(DEVIL_DLL_DIR ${GHOUL_ROOT_DIR}/ext/il/lib/win32)
#             endif (CMAKE_CL_64)
#             set(SHARED_LIBS ${SHARED_LIBS} ${DEVIL_DLL_DIR}/DevIL.dll)
#             set(SHARED_LIBS ${SHARED_LIBS} ${DEVIL_DLL_DIR}/ILU.dll)
#             set(SHARED_LIBS ${SHARED_LIBS} ${DEVIL_DLL_DIR}/ILUT.dll)
#         endif(GHOUL_USE_DEVIL)

#         # FreeImage
#         if(GHOUL_USE_FREEIMAGE)
#             if (CMAKE_CL_64)
#                 set(FREEIMAGE_DLL_DIR ${GHOUL_ROOT_DIR}/ext/freeimage/lib/win64)
#             else (CMAKE_CL_64)
#                 set(FREEIMAGE_DLL_DIR ${GHOUL_ROOT_DIR}/ext/freeimage/lib/win32)
#             endif (CMAKE_CL_64)
#             set(SHARED_LIBS ${SHARED_LIBS} ${FREEIMAGE_DLL_DIR}/FreeImage.dll)
#         endif(GHOUL_USE_FREEIMAGE)

#         # Add the copy command
#         foreach( file_i ${SHARED_LIBS})
#             add_custom_command(TARGET ${TARGET_EXECUTABLE} POST_BUILD
#             COMMAND ${CMAKE_COMMAND} -E copy_if_different
#             "${file_i}"
#             $<TARGET_FILE_DIR:${TARGET_EXECUTABLE}>)
#         endforeach( file_i )
#     endif(WIN32)

# endmacro(GhoulCopySharedLibraries)

# ######
# # Source
# ######

# set(SOURCE_ROOT_DIR ${GHOUL_ROOT_DIR}/src)
# set(HEADER_ROOT_DIR ${GHOUL_ROOT_DIR}/include/ghoul)

# add_source_group("CmdParser" "cmdparser")
# add_source_group("DesignPattern" "designpattern")
# add_source_group("Exception" "exception")
# add_source_group("FileSystem" "filesystem")
# add_source_group("IO" "io")

# file(GLOB IO_TEXTURE_SOURCE ${SOURCE_ROOT_DIR}/io/texture/*.cpp)
# set(GHOUL_SOURCE ${GHOUL_SOURCE} ${IO_TEXTURE_SOURCE})
# file(GLOB IO_TEXTURE_HEADER ${HEADER_ROOT_DIR}/io/texture/*.h)
# set(GHOUL_HEADER ${GHOUL_HEADER} ${IO_TEXTURE_HEADER})
# source_group(IO\\Texture FILES ${IO_TEXTURE_SOURCE} ${IO_TEXTURE_HEADER})

# file(GLOB IO_MODEL_SOURCE ${SOURCE_ROOT_DIR}/io/model/*.cpp)
# set(GHOUL_SOURCE ${GHOUL_SOURCE} ${IO_MODEL_SOURCE})
# file(GLOB IO_MODEL_HEADER ${HEADER_ROOT_DIR}/io/model/*.h)
# set(GHOUL_HEADER ${GHOUL_HEADER} ${IO_MODEL_HEADER})
# source_group(IO\\Model FILES ${IO_MODEL_SOURCE} ${IO_MODEL_HEADER})
             
# add_source_group("LogManager" "logging") 
# add_source_group("Lua" "lua") 
# add_source_group("Misc" "misc") 
# add_source_group("OpenGL" "opengl") 
# add_source_group("SystemCapabilities" "systemcapabilities") 

# set(GHOUL_HEADER ${GHOUL_HEADER} ${HEADER_ROOT_DIR}/glm.h)

# ######
# # Dependencies
# ######
# set(GHOUL_DEPENDENCIES ${GHOUL_DEPENDENCIES} ${DEVIL_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${MAC_LIBS})

# include_directories(
#     ${GHOUL_ROOT_DIR}/include
#     ${GHOUL_ROOT_DIR}/include/ghoul)
    
# if (CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake|MSBuild)")
#     if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
#         string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
#     else ()
#         set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
#     endif ()
#     # C4201: nonstandard extension used : nameless struct/union    # Extension is ubiquitous
#     # C4127: 'if' or 'while' loop evaluates to a constant          # Used for 'swallowing the semicolon" idiom
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4201 /wd4127")
# else()
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
#     if(UNIX)
#         set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-long-long -pedantic -Wextra")
#     endif(UNIX)
# endif ()

# # Create the Ghoul library
# add_library(Ghoul STATIC ${GHOUL_SOURCE} ${GHOUL_HEADER})

# # Adding debug flags for the debug build
# if(CMAKE_BUILD_TYPE MATCHES Debug)
#     if(MSVC OR XCODE)
#         if(${CMAKE_VERSION} VERSION_GREATER 3.0 OR ${CMAKE_VERSION} VERSION_EQUAL 3.0)
#             target_compile_definitions(Ghoul PRIVATE $<$<CONFIG:Debug>:GHL_DEBUG>)
#         else()
#             set_target_properties(Ghoul PROPERTIES COMPILE_DEFINITIONS_DEBUG "GHL_DEBUG")
#         endif()
#     else() 
#         add_definitions(-DGHL_DEBUG)
#         add_definitions(-D_DEBUG)
#     endif(MSVC OR XCODE)
# endif(CMAKE_BUILD_TYPE MATCHES Debug)

# target_link_libraries(Ghoul Lua ${GHOUL_DEPENDENCIES})
