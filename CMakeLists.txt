##########################################################################################
#                                                                                        #
# GHOUL                                                                                  #
#                                                                                        #
# Copyright (c) 2012-2020                                                                #
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

cmake_minimum_required(VERSION 3.17 FATAL_ERROR)
project(Ghoul)

# Setting default paths
set(GHOUL_ROOT_DIR ${PROJECT_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${GHOUL_ROOT_DIR}/support/cmake)

include(${GHOUL_ROOT_DIR}/support/cmake/handle_external_library.cmake)
include(${GHOUL_ROOT_DIR}/support/cmake/set_ghoul_compile_settings.cmake)
include(${GHOUL_ROOT_DIR}/support/cmake/copy_shared_libraries.cmake)
include(${GHOUL_ROOT_DIR}/support/cmake/message_macros.cmake)

begin_header("Configuring Ghoul Project")
list(APPEND CMAKE_MESSAGE_CONTEXT "ghoul")

#############################
# Options
#############################
option(GHOUL_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(GHOUL_HIGH_DEBUG_MODE "Add additional debugging code" ON)
option(GHOUL_LOGGING_ENABLE_TRACE "Enables the LTRACE macro" ON)
option(GHOUL_THROW_ON_ASSERT "Disables the feedback on asserts; for use in unit tests" OFF)
option(GHOUL_PROFILING_ENABLE_TRACY "Enable profiling with Tracy" OFF)

option(GHOUL_MODULE_ASSIMP "Enable AssImp Model loader" ON)
option(GHOUL_MODULE_COMMANDLINEPARSER "Enable Commandlineparser" ON)
option(GHOUL_MODULE_FONTRENDERING "Enable Fontrendering" ON)
option(GHOUL_MODULE_LUA "Enable Lua" ON)
option(GHOUL_MODULE_OPENGL "Enable OpenGL" ON)
option(GHOUL_MODULE_SYSTEMCAPABILITIES "Enable System Capabilities" ON)

option(BUILD_SHARED_LIBS "Build package with shared libraries" OFF)

if (MSVC)
  option(GHOUL_OPTIMIZATION_ENABLE_AVX "Enable AVX instruction set for compilation" OFF)
  option(GHOUL_OPTIMIZATION_ENABLE_AVX2 "Enable AVX2 instruction set for compilation" OFF)
  option(GHOUL_OPTIMIZATION_ENABLE_AVX512 "Enable AVX2 instruction set for compilation" OFF)
  option(GHOUL_OPTIMIZATION_ENABLE_OTHER_OPTIMIZATIONS "Enable other optimizations, like LTCG, intrinsics, etc")

  if (GHOUL_OPTIMIZATION_ENABLE_AVX AND GHOUL_OPTIMIZATION_ENABLE_AVX2)
    message(FATAL_ERROR "Cannot enable AVX and AVX2 instructions simultaneously")
  endif ()

  if (GHOUL_OPTIMIZATION_ENABLE_AVX AND GHOUL_OPTIMIZATION_ENABLE_AVX512)
    message(FATAL_ERROR "Cannot enable AVX and AVX512 instructions simultaneously")
  endif ()

  if (GHOUL_OPTIMIZATION_ENABLE_AVX2 AND GHOUL_OPTIMIZATION_ENABLE_AVX512)
    message(FATAL_ERROR "Cannot enable AVX2 and AVX512 instructions simultaneously")
  endif ()
endif ()

list(APPEND CMAKE_MESSAGE_CONTEXT "src")
include(src/CMakeLists.txt)
list(POP_BACK CMAKE_MESSAGE_CONTEXT)

# Compile settings
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

# Dependencies

# System Libraries
if (WIN32)
  target_link_libraries(Ghoul PRIVATE "shlwapi" "WbemUuid.lib" "Ws2_32.lib")
endif ()

if (APPLE)
  begin_dependency("Core Libraries")
  find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
  find_library(CARBON_LIBRARY Carbon REQUIRED)
  find_library(COCOA_LIBRARY Carbon REQUIRED)
  find_library(APP_SERVICES_LIBRARY ApplicationServices REQUIRED)
  target_include_directories(Ghoul SYSTEM PRIVATE "/Developer/Headers/FlatCarbon")
  target_link_libraries(Ghoul PRIVATE ${CARBON_LIBRARY} ${COREFOUNDATION_LIBRARY} ${COCOA_LIBRARY} ${APP_SERVICES_LIBRARY})
  end_dependency("Core Libraries")
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
  begin_dependency("iNotify")
  find_library(INOTIFY_LIBRARIES inotify PATHS "/usr/local/lib")
  target_include_directories(Ghoul SYSTEM PRIVATE "/usr/local/include")
  target_link_libraries(Ghoul PRIVATE ${INOTIFY_LIBRARIES})
  end_dependency("iNotify")
endif ()

# GLM
begin_dependency("GLM")
set(GLM_ROOT_DIR "${GHOUL_ROOT_DIR}/ext/glm")
find_package(GLM REQUIRED)
message(STATUS "Including GLM found at ${GLM_INCLUDE_DIRS}")
target_include_directories(Ghoul SYSTEM PUBLIC ${GLM_INCLUDE_DIRS})
end_dependency("GLM")

if (GHOUL_MODULE_OPENGL)
  begin_module("OpenGL")
  cmake_policy(SET CMP0072 NEW) # FindOpenGL prefers GLVND by default when available.

  # OpenGL
  begin_dependency("OpenGL")
  find_package(OpenGL REQUIRED)
  target_link_libraries(Ghoul PUBLIC ${OPENGL_LIBRARIES})
  message(STATUS "Linking OpenGL found at ${OPENGL_LIBRARIES}")
  end_dependency("OpenGL")

  # glbindings
  if (NOT TARGET glbinding)
    begin_dependency("glbinding")
  
    set(OPTION_BUILD_GPU_TESTS OFF CACHE BOOL "")
    set(OPTION_BUILD_TESTS OFF CACHE BOOL "")
    set(OPTION_BUILD_TOOLS OFF CACHE BOOL "")
    set(OPTION_BUILD_EXAMPLES OFF CACHE BOOL "")
    add_subdirectory(${GHOUL_ROOT_DIR}/ext/glbinding)
    disable_external_warnings(glbinding)
    disable_external_warnings(glbinding-aux)
    # target_include_directories(Ghoul SYSTEM PUBLIC
    #   ${GHOUL_ROOT_DIR}/ext/glbinding/source/glbinding/include
    #   ${GHOUL_ROOT_DIR}/ext/glbinding/source/glbinding-aux/include
    # )
    target_link_libraries(Ghoul PUBLIC glbinding::glbinding glbinding::glbinding-aux)
  
    set_folder_location(glbinding "External")
    set_folder_location(glbinding-aux "External/glbinding-ext")
    set_folder_location(pack "External/glbinding-ext")
    set_folder_location(pack-glbinding "External/glbinding-ext")
    set_folder_location(KHRplatform-sources "External/glbinding-ext")
  
    end_dependency("glbinding")
  endif ()
  end_module("OpenGL")
endif ()

# LZ4
begin_dependency("LZ4")
find_package(LZ4 REQUIRED)
target_include_directories(Ghoul PRIVATE ${LZ4_INCLUDE_DIR})
target_link_libraries(Ghoul PRIVATE ${LZ4_LIBRARIES})
set_folder_location(lz4 "External")
end_dependency("LZ4")

# Lua
if (GHOUL_MODULE_LUA)
  begin_module("Lua")
  include_external_library(Ghoul PUBLIC Lua ${GHOUL_ROOT_DIR}/ext/lua ${GHOUL_ROOT_DIR}/ext/lua/src)
  end_module("Lua")
endif ()

# Assimp
if (GHOUL_MODULE_ASSIMP)
  begin_module("AssImp")
  find_package(Assimp QUIET)
  if (Assimp_FOUND)
    message(STATUS "Using system assimp")
    # target_include_directories(Ghoul SYSTEM PRIVATE ${ASSIMP_INCLUDE_DIR})
    target_link_libraries(Ghoul PRIVATE ${ASSIMP_LIBRARIES})
    set(GHOUL_LIBRARY_DIRS "${ASSIMP_LIBRARY_DIRS}" PARENT_SCOPE)
  else ()
    message(STATUS "Building assimp from source")
    set(ASSIMP_BUILD_SHARED_LIBS OFF)
    set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)

    # These importers produce errors due to missing entries in the config.h right now
    # When they compile, there is no reason to keep them disabled
    set(ASSIMP_BUILD_FBX_IMPORTER OFF CACHE BOOL "")
    set(ASSIMP_BUILD_IFC_IMPORTER OFF CACHE BOOL "")
    set(ASSIMP_BUILD_X3D_IMPORTER OFF CACHE BOOL "")
    set(ASSIMP_BUILD_X_IMPORTER OFF CACHE BOOL "")
    set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "")
    set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "")
    set(ASSIMP_NO_EXPORT ON CACHE BOOL "")
    set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")

    include_external_library(Ghoul PRIVATE assimp ${GHOUL_ROOT_DIR}/ext/assimp ${GHOUL_ROOT_DIR}/ext/assimp/include)

    # We need to include this to be able to find the assimp/config.h file
    target_include_directories(assimp PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/ext/assimp/include")

    set_folder_location(IrrXML "External")
    set_folder_location(uninstall "External")
    set_folder_location(UpdateAssimpLibsDebugSymbolsAndDLLs "External")
  endif ()
  end_module("AssImp")
endif ()

# fmt
begin_dependency("fmt")
set(FMT_DOC OFF CACHE BOOL "Generate the doc target.")
set(FMT_INSTALL OFF CACHE BOOL "Generate the install target.")
set(FMT_TEST OFF CACHE BOOL "Generate the test target.")
include_external_library(Ghoul PUBLIC fmt ${GHOUL_ROOT_DIR}/ext/cppformat ${GHOUL_ROOT_DIR}/ext/cppformat/include)
end_dependency("fmt")

message(STATUS "Loading image libraries")

# DevIL
find_package(DevIL QUIET)
if (IL_FOUND)
  begin_dependency("DevIL")
  option(GHOUL_USE_DEVIL "Use the DevIL library" ON)
  if (GHOUL_USE_DEVIL)
    target_compile_definitions(Ghoul PRIVATE "GHOUL_USE_DEVIL")
    # target_include_directories(Ghoul SYSTEM PRIVATE ${IL_INCLUDE_DIR})
    target_link_libraries(Ghoul PRIVATE ${IL_LIBRARIES} ${ILU_LIBRARIES} ${ILUT_LIBRARIES})
  endif ()
  end_dependency("DevIL")
endif ()

# FreeImage
find_package(FreeImage QUIET)
if (FREEIMAGE_FOUND)
  begin_dependency("FreeImage")
  option(GHOUL_USE_FREEIMAGE "Use the FreeImage library" ON)
  if (GHOUL_USE_FREEIMAGE)
    target_compile_definitions(Ghoul PUBLIC "GHOUL_USE_FREEIMAGE")
    # target_include_directories(Ghoul SYSTEM PRIVATE ${FREEIMAGE_INCLUDE_PATH})
    target_link_libraries(Ghoul PRIVATE ${FREEIMAGE_LIBRARIES})
  endif ()
  end_dependency("FreeImage")
endif ()

# SOIL
find_package(SOIL QUIET)
if (SOIL_FOUND)
  begin_dependency("SOIL")
  option(GHOUL_USE_SOIL "Use the SOIL library" ON)
  if (GHOUL_USE_SOIL)
    target_compile_definitions(Ghoul PUBLIC "GHOUL_USE_SOIL")
    # target_include_directories(Ghoul PRIVATE ${SOIL_INCLUDE_DIRS} ${OPENGL_LIBRARIES})
    target_link_libraries(Ghoul PRIVATE ${SOIL_LIBRARIES})
  endif ()
  end_dependency("SOIL")
endif ()

# stb_image
begin_dependency("stb_image")
target_compile_definitions(Ghoul PUBLIC "GHOUL_USE_STB_IMAGE")
include_external_library(Ghoul PUBLIC stb_image ${GHOUL_ROOT_DIR}/ext/stb_image ${GHOUL_ROOT_DIR}/ext/stb_image)
end_dependency("stb_image")

# Stackwalker
if (MSVC)
  begin_dependency("Stackwalker")
  include_external_library(Ghoul PRIVATE StackWalker ${GHOUL_ROOT_DIR}/ext/stackwalker ${GHOUL_ROOT_DIR}/ext/stackwalker)
  end_dependency("Stackwalker")
endif ()

# WebSocket++ / WebSocketPP
begin_dependency("WebSocketPP")
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
set_folder_location(websocketpp "External")
set_folder_location(websocketpp-build "External")
set_folder_location(websocketpp-configure "External")
end_dependency("WebSocketPP")

# Freetype
if (GHOUL_MODULE_FONTRENDERING)
  begin_module("FontRendering")
  set(FREETYPE_DIST OFF CACHE BOOL "")
  include_external_library(Ghoul PUBLIC freetype ${GHOUL_ROOT_DIR}/ext/freetype2 ${GHOUL_ROOT_DIR}/ext/freetype2/include)
  set_folder_location(dist "External")

  if (TARGET zlib)
    set_folder_location(zlib "External")
  endif ()
  end_module("FontRendering")
endif ()

# Tiny process library
begin_dependency("tiny-process-library")
include_external_library(Ghoul PRIVATE tiny-process-library ${GHOUL_ROOT_DIR}/ext/tiny-process-library ${GHOUL_ROOT_DIR}/ext/tiny-process-library)
end_dependency("tiny-process-library")


# Tracy
begin_dependency("Profiling")
if (NOT TARGET tracy)
  add_library(tracy "${PROJECT_SOURCE_DIR}/ext/tracy/TracyClient.cpp")
  disable_external_warnings(tracy)
  target_include_directories(tracy PUBLIC "${PROJECT_SOURCE_DIR}/ext/tracy")
  set_folder_location(tracy "External")
endif ()
if (GHOUL_PROFILING_ENABLE_TRACY)
  target_compile_definitions(tracy PUBLIC "TRACY_ENABLE")
  target_link_libraries(Ghoul PUBLIC tracy)
  if (UNIX)
    target_link_libraries(Ghoul PUBLIC dl)
  endif ()
endif ()
end_dependency("Profiling")

# Other applications
option(GHOUL_HAVE_TESTS "Activate the unit tests" ON)
if (GHOUL_HAVE_TESTS)
  begin_header("Generating unit test")
  add_subdirectory(tests)
  end_header()
endif ()

end_header("End: Configuring Ghoul Project")
