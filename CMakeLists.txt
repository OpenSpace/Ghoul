##########################################################################################
#                                                                                        #
# GHOUL                                                                                  #
#                                                                                        #
# Copyright (c) 2012-2018                                                                #
#                                                                                        #
# Permission is hereby granted, free of charge, to any person obtaining a copy of this   #
# software and associated documentation files (the "Software"), to deal in the Software  #
# without restriction, including without limitation the rights to use, copy, modify,     #
# merge, publish, distribute, sublicense, and/or sell copies of the Software, and to     #
# permit persons to whom the Software is furnished to do so, subject to the following    #
# conditions:                                                                            #
#                                                                                        #
# The above copyright notice and this permission notice shall be included in all copies  #
# or substantial portions of the Software.                                               #
#                                                                                        #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,    #
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A          #
# PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT     #
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF   #
# CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE   #
# OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                          #
##########################################################################################

cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project(Ghoul)
message(STATUS "Generating Ghoul project")

# Setting default paths
set(GHOUL_ROOT_DIR ${PROJECT_SOURCE_DIR})
set(GHOUL_CMAKE_DIR ${GHOUL_ROOT_DIR}/support/cmake)
set(GHOUL_CMAKE_DIR ${GHOUL_ROOT_DIR}/support/cmake PARENT_SCOPE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${GHOUL_CMAKE_DIR} ${GHOUL_ROOT_DIR}/ext)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${GHOUL_ROOT_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${GHOUL_ROOT_DIR}/bin")

include(${GHOUL_CMAKE_DIR}/handle_external_library.cmake)
include(${GHOUL_CMAKE_DIR}/set_ghoul_compile_settings.cmake)


#############################
# Options
#############################
if (WIN32)
    option(GHOUL_USE_WMI "Use Windows Management Instrumentation" ON)
endif ()
option(GHOUL_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(GHOUL_HIGH_DEBUG_MODE "Add additional debugging code" ON)
option(GHOUL_LOGGING_ENABLE_TRACE "Enables the LTRACE macro" ON)
option(GHOUL_DISABLE_EXTERNAL_WARNINGS "Disable warnings in external libraries" ON)
option(GHOUL_THROW_ON_ASSERT "Disables the feedback on asserts; for use in unit tests" OFF)

option(GHOUL_MODULE_ASSIMP "Enable AssImp Model loader" ON)
option(GHOUL_MODULE_COMMANDLINEPARSER "Enable Commandlineparser" ON)
option(GHOUL_MODULE_FONTRENDERING "Enable Fontrendering" ON)
option(GHOUL_MODULE_LUA "Enable Lua" ON)
option(GHOUL_MODULE_OPENGL "Enable OpenGL" ON)
option(GHOUL_MODULE_SYSTEMCAPABILITIES "Enable System Capabilities" ON)

option(BUILD_SHARED_LIBS "Build package with shared libraries." OFF)

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
# Compile settings
#############################
set_property(TARGET Ghoul PROPERTY CXX_STANDARD 17)
set_property(TARGET Ghoul PROPERTY CXX_STANDARD_REQUIRED On)

set_ghoul_compile_settings(Ghoul)

if (GHOUL_HIGH_DEBUG_MODE)
    target_compile_definitions(Ghoul PUBLIC "GHL_DEBUG")
endif ()

if (GHOUL_LOGGING_ENABLE_TRACE)
    target_compile_definitions(Ghoul PUBLIC "GHOUL_LOGGING_ENABLE_TRACE")
endif ()

if (GHOUL_THROW_ON_ASSERT)
    target_compile_definitions(Ghoul PUBLIC "GHL_THROW_ON_ASSERT")
endif ()

#############################
# Dependencies
#############################

# System Libraries
if (WIN32)
    if (GHOUL_USE_WMI)
        target_compile_definitions(Ghoul PUBLIC "GHOUL_USE_WMI")
        target_link_libraries(Ghoul "WbemUuid.lib")
    endif ()
    target_link_libraries(Ghoul "shlwapi")
endif ()

if (APPLE)
    target_include_directories(Ghoul SYSTEM PUBLIC "/Developer/Headers/FlatCarbon")
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
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    target_include_directories(Ghoul SYSTEM PUBLIC "/usr/local/include")
    find_library(INOTIFY_LIBRARIES inotify PATHS "/usr/local/lib")
    target_link_libraries(Ghoul ${INOTIFY_LIBRARIES})
endif ()

# GLM
set(GLM_ROOT_DIR "${GHOUL_ROOT_DIR}/ext/glm")
find_package(GLM REQUIRED)
target_include_directories(Ghoul SYSTEM PUBLIC ${GLM_INCLUDE_DIRS})

if (GHOUL_MODULE_OPENGL)
    # OpenGL
    find_package(OpenGL REQUIRED)
    target_include_directories(Ghoul SYSTEM PUBLIC ${OPENGL_INCLUDE_DIR})
    target_link_libraries(Ghoul ${OPENGL_LIBRARIES})

    # glbindings
    set(OPTION_BUILD_GPU_TESTS OFF CACHE BOOL "")
    set(OPTION_BUILD_TESTS OFF CACHE BOOL "")
    set(OPTION_BUILD_TOOLS OFF CACHE BOOL "")
    add_subdirectory(${GHOUL_ROOT_DIR}/ext/glbinding)
    target_include_directories(Ghoul SYSTEM PUBLIC ${GHOUL_ROOT_DIR}/ext/glbindings/source/glbindings/include)
    target_link_libraries(Ghoul glbinding::glbinding)
    if (TARGET glbinding)
        set_property(TARGET glbinding PROPERTY FOLDER "External")
    endif ()
    if (TARGET check-glbinding)
        set_property(TARGET check-glbinding PROPERTY FOLDER "External/glbinding-ext")
    endif ()
    if (TARGET component_install)
        set_property(TARGET component_install PROPERTY FOLDER "External/glbinding-ext")
    endif ()
    if (TARGET generate)
        set_property(TARGET generate PROPERTY FOLDER "External/glbinding-ext")
    endif ()
    if (TARGET pack)
        set_property(TARGET pack PROPERTY FOLDER "External/glbinding-ext")
    endif ()
    if (TARGET pack-glbinding)
        set_property(TARGET pack-glbinding PROPERTY FOLDER "External/glbinding-ext")
    endif ()
    if (TARGET update)
        set_property(TARGET update PROPERTY FOLDER "External/glbinding-ext")
    endif ()
    if (TARGET check-all)
        set_property(TARGET check-all PROPERTY FOLDER "External/glbinding-ext")
    endif ()
    if (TARGET check-template)
        set_property(TARGET check-template PROPERTY FOLDER "External/glbinding-ext")
    endif ()
    if (TARGET cppcheck-glbinding)
        set_property(TARGET cppcheck-glbinding PROPERTY FOLDER "External/glbinding-ext")
    endif ()

    target_compile_definitions(Ghoul PUBLIC "GHOUL_MODULE_OPENGL")
endif ()

# LZ4
find_package(LZ4 REQUIRED)
target_include_directories(Ghoul SYSTEM PUBLIC ${LZ4_INCLUDE_DIR})
target_link_libraries(Ghoul ${LZ4_LIBRARIES})
set_property(TARGET lz4 PROPERTY FOLDER "External")

# Lua
if (GHOUL_MODULE_LUA)
    include_external_library(Ghoul Lua ${GHOUL_ROOT_DIR}/ext/lua)
endif ()

# Assimp
if (GHOUL_MODULE_ASSIMP)
    find_package(Assimp QUIET)
    if (Assimp_FOUND)
        message(STATUS "Using system assimp")
        target_include_directories(Ghoul SYSTEM PUBLIC ${ASSIMP_INCLUDE_DIRS})
        target_link_libraries(Ghoul ${ASSIMP_LIBRARIES})
        link_directories("${ASSIMP_LIBRARY_DIRS}")
        set(GHOUL_LIBRARY_DIRS "${ASSIMP_LIBRARY_DIRS}" PARENT_SCOPE)
    else ()
        message(STATUS "Building assimp")
        set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "We don't build assimp tools because they need DirectX.")

        # These two importers produce errors due to missing entries in the config.h right now
        # When they compile, there is no reason to keep them disabled
        set(ASSIMP_BUILD_FBX_IMPORTER OFF CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_IFC_IMPORTER OFF CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_X3D_IMPORTER OFF CACHE BOOL "" FORCE)

        set(ASSIMP_BUILD_X_IMPORTER OFF CACHE BOOL "Disable DirectX file format.")
        set(ASSIMP_BUILD_X_IMPORTER OFF CACHE BOOL "Disable DirectX file format.")
        set(ASSIMP_NO_EXPORTER ON CACHE BOOL "Decrease compiling time.")
        set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Disable GTest.")
        include_external_library(Ghoul assimp ${GHOUL_ROOT_DIR}/ext/assimp ${GHOUL_ROOT_DIR}/ext/assimp/include)
        set_property(TARGET uninstall PROPERTY FOLDER "External")

        # We need to include this to be able to find the assimp/config.h file
        target_include_directories(Ghoul PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/ext/assimp/include")

        if (WIN32)
            if (TARGET IrrXML)
                set_property(TARGET IrrXML PROPERTY FOLDER "External")
            endif ()
            if (TARGET UpdateAssimpLibsDebugSymbolsAndDLLs)
                set_property(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTY FOLDER "External")
            endif ()
        endif ()
    endif ()
endif ()

# Cppformat
set(FMT_DOC OFF CACHE BOOL "Generate the doc target.")
set(FMT_INSTALL OFF CACHE BOOL "Generate the install target.")
set(FMT_TEST OFF CACHE BOOL "Generate the test target.")
include_external_library(Ghoul fmt ${GHOUL_ROOT_DIR}/ext/cppformat ${GHOUL_ROOT_DIR}/ext/cppformat)

# DevIL
find_package(DevIL)
if (IL_FOUND)
    option(GHOUL_USE_DEVIL "Use the DevIL library" ON)
    if (GHOUL_USE_DEVIL)
        target_compile_definitions(Ghoul PUBLIC "GHOUL_USE_DEVIL")
        # set(DevIL_USE_STATIC_LIBS OFF)
        target_include_directories(Ghoul SYSTEM PUBLIC ${IL_INCLUDE_DIR})
        target_link_libraries(Ghoul ${IL_LIBRARIES} ${ILU_LIBRARIES} ${ILUT_LIBRARIES})
    endif ()
endif ()

# FreeImage
find_package(FreeImage)
if (FREEIMAGE_FOUND)
    option(GHOUL_USE_FREEIMAGE "Use the FreeImage library" ON)
    if (GHOUL_USE_FREEIMAGE)
        target_compile_definitions(Ghoul PUBLIC "GHOUL_USE_FREEIMAGE")
        target_include_directories(Ghoul SYSTEM PUBLIC ${FREEIMAGE_INCLUDE_PATH})
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

# stb_image
option(GHOUL_USE_STB_IMAGE "Use the stb_image library" ON)
if (GHOUL_USE_STB_IMAGE)
    target_compile_definitions(Ghoul PUBLIC "GHOUL_USE_STB_IMAGE")
    include_external_library(Ghoul stb_image ${GHOUL_ROOT_DIR}/ext/stb_image)
endif ()

# Stackwalker
if (MSVC)
    target_include_directories(Ghoul PUBLIC ${GHOUL_ROOT_DIR}/ext/stackwalker)
    include_external_library(Ghoul StackWalker ${GHOUL_ROOT_DIR}/ext/stackwalker ${GHOUL_ROOT_DIR}/ext/stackwalker)
endif ()

# WebSocket++ / WebSocketPP
include(ExternalProject)
set(WEBSOCKETPP_ROOT_DIR ${GHOUL_ROOT_DIR}/ext/websocketpp)
ExternalProject_Add(websocketpp
    SOURCE_DIR ${WEBSOCKETPP_ROOT_DIR}
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS ${EXTERNAL_TOOLCHAIN} ${EXTRA_EXTERNAL_ARGS} -DBUILD_TESTS=OFF -DBUILD_EXAMPLES=OFF
    INSTALL_COMMAND ""
    LOG_CONFIGURE 1
    LOG_BUILD 1
    STEP_TARGETS configure build
)

add_dependencies(Ghoul websocketpp)
target_include_directories(Ghoul SYSTEM PUBLIC ${WEBSOCKETPP_ROOT_DIR})

# Image library assertion
if (NOT GHOUL_USE_DEVIL AND NOT GHOUL_USE_FREEIMAGE AND NOT GHOUL_USE_SOIL AND NOT GHOUL_USE_STB_IMAGE)
    message(FATAL_ERROR "Ghoul needs to be built with DevIL, FreeImage, SoIL or stb_image")
endif ()

# Freetype
if (GHOUL_MODULE_FONTRENDERING)
    set(FREETYPE_DIST OFF CACHE BOOL "Generate the dist target.")
    include_external_library(Ghoul freetype ${GHOUL_ROOT_DIR}/ext/freetype2 ${GHOUL_ROOT_DIR}/ext/freetype2/include)
    set_property(TARGET dist PROPERTY FOLDER "External")
endif ()

#############################
# Other applications
#############################
option(GHOUL_HAVE_TESTS "Activate the unit tests" ON)
if (GHOUL_HAVE_TESTS)
    file(GLOB_RECURSE GHOUL_TEST_FILES ${GHOUL_ROOT_DIR}/tests/*.inl)
    add_executable(GhoulTest ${GHOUL_ROOT_DIR}/tests/main.cpp ${GHOUL_TEST_FILES})

    set_property(TARGET GhoulTest PROPERTY CXX_STANDARD 17)
    set_property(TARGET GhoulTest PROPERTY CXX_STANDARD_REQUIRED On)

    target_compile_definitions(GhoulTest PRIVATE
        "GHL_THROW_ON_ASSERT" # Jenkins shouldn't ask for asserts when they happen, but just throw
        "GHOUL_HAVE_TESTS"
        "GTEST_HAS_TR1_TUPLE=0"
        "GTEST_HAS_GLOBAL_STRING=0"
        "GHOUL_ROOT_DIR=\"${GHOUL_ROOT_DIR}\""
    )

    set_ghoul_compile_settings(GhoulTest)

    if (MSVC)
        target_compile_options(
            GhoulTest
            PUBLIC
            "-Od"
            "/bigobj"
        )
    else ()
        target_compile_options(
            GhoulTest
            PUBLIC
            "-O0"
        )
    endif ()

    target_include_directories(GhoulTest PUBLIC "${GHOUL_ROOT_DIR}/ext/googletest/googletest/include" "${GHOUL_ROOT_DIR}")

    set(BUILD_GTEST ON CACHE BOOL "")
    set(BUILD_GMOCK OFF CACHE BOOL "")
    set(gtest_force_shared_crt ON CACHE BOOL "")
    # set(BUILD_SHARED_LIBS ON CACHE BOOL "")

    # option(BUILD_GTEST "Builds the googletest subproject" CACHE ON)
    # option(BUILD_GMOCK "Builds the googlemock subproject" CACHE OFF)
    # option(BUILD_SHARED_LIBS "Build shared libraries (DLLs)." CACHE ON)
    add_subdirectory(${GHOUL_ROOT_DIR}/ext/googletest)
    target_link_libraries(GhoulTest gtest Ghoul Lua)

    target_compile_definitions(gtest PUBLIC
        "_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING"
    )

    set_property(TARGET gtest_main PROPERTY FOLDER "External")
    set_property(TARGET gtest PROPERTY FOLDER "External")

    if (APPLE)
        target_link_libraries(GhoulTest
            ${CARBON_LIBRARY}
            ${COREFOUNDATION_LIBRARY}
            ${COCOA_LIBRARY}
            ${APP_SERVICES_LIBRARY}
        )
    endif ()
endif ()
