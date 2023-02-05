##########################################################################################
#                                                                                        #
# GHOUL                                                                                  #
# General Helpful Open Utility Library                                                   #
#                                                                                        #
# Copyright (c) 2012-2023                                                                #
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

add_subdirectory(catch2)
# Catch2 by default compiles with C++14, which leads to some linker errors:
# https://github.com/catchorg/Catch2/issues/2046
target_compile_features(Catch2 PUBLIC cxx_std_20)
target_compile_features(Catch2WithMain PUBLIC cxx_std_20)
set_target_properties(Catch2 PROPERTIES FOLDER External)
set_target_properties(Catch2WithMain PROPERTIES FOLDER External)

if (GHOUL_MODULE_OPENGL)
  begin_module("OpenGL")

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
    target_link_libraries(Ghoul PUBLIC glbinding::glbinding glbinding::glbinding-aux)

    set_target_properties(glbinding PROPERTIES FOLDER "External")
    set_target_properties(glbinding-aux PROPERTIES FOLDER "External/glbinding-ext")
    set_target_properties(pack PROPERTIES FOLDER "External/glbinding-ext")
    set_target_properties(pack-glbinding PROPERTIES FOLDER "External/glbinding-ext")
    set_target_properties(KHRplatform-sources PROPERTIES FOLDER "External/glbinding-ext")

    end_dependency("glbinding")
  endif ()
  end_module("OpenGL")
endif ()

# LZ4
begin_dependency("LZ4")
find_package(LZ4 REQUIRED)
target_include_directories(Ghoul PRIVATE ${LZ4_INCLUDE_DIR})
target_link_libraries(Ghoul PRIVATE ${LZ4_LIBRARIES})
set_target_properties(lz4 PROPERTIES FOLDER "External")
end_dependency("LZ4")

# Lua
if (GHOUL_MODULE_LUA)
  begin_module("Lua")
  add_subdirectory(lua SYSTEM)
  target_link_libraries(Ghoul PUBLIC Lua)
  set_target_properties(Lua PROPERTIES FOLDER "External")
  end_module("Lua")
endif ()

# Assimp
if (GHOUL_MODULE_ASSIMP)
  begin_module("AssImp")
  set(ASSIMP_BUILD_SHARED_LIBS OFF)
  set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
  set(ASSIMP_NO_EXPORTER ON CACHE BOOL "")
  set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")

  if (UNIX)
    set(ASSIMP_BUILD_MINIZIP ON CACHE BOOL "Set to have assimp build minizip" FORCE)
  endif ()

  add_subdirectory(assimp SYSTEM)
  target_link_libraries(Ghoul PRIVATE assimp)
  set_target_properties(assimp PROPERTIES FOLDER "External")

  # We need to include this to be able to find the assimp/config.h file
  target_include_directories(assimp PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/ext/assimp/include")
  if (NOT APPLE)
    target_precompile_headers(assimp PRIVATE
      "$<$<COMPILE_LANGUAGE:CXX>:functional>"
      "$<$<COMPILE_LANGUAGE:CXX>:istream>"
      "$<$<COMPILE_LANGUAGE:CXX>:list>"
      "$<$<COMPILE_LANGUAGE:CXX>:locale>"
      "$<$<COMPILE_LANGUAGE:CXX>:map>"
      "$<$<COMPILE_LANGUAGE:CXX>:ostream>"
      "$<$<COMPILE_LANGUAGE:CXX>:set>"
      "$<$<COMPILE_LANGUAGE:CXX>:sstream>"
      "$<$<COMPILE_LANGUAGE:CXX>:vector>"
      "$<$<COMPILE_LANGUAGE:CXX>:tuple>"
      "$<$<COMPILE_LANGUAGE:CXX>:unordered_map>"
      "$<$<COMPILE_LANGUAGE:CXX>:assimp/DefaultLogger.hpp>"
      "$<$<COMPILE_LANGUAGE:CXX>:assimp/Exceptional.h>"
      "$<$<COMPILE_LANGUAGE:CXX>:assimp/mesh.h>"
      "$<$<COMPILE_LANGUAGE:CXX>:assimp/scene.h>"
      "$<$<COMPILE_LANGUAGE:CXX>:assimp/StringUtils.h>"
      "$<$<COMPILE_LANGUAGE:CXX>:assimp/types.h>"
      "$<$<COMPILE_LANGUAGE:CXX>:assimp/XmlParser.h>"
      <string.h>
    )
  endif ()
  set_target_properties(uninstall PROPERTIES FOLDER "External")
  if (TARGET UpdateAssimpLibsDebugSymbolsAndDLLs)
    set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER "External")
  endif ()
  end_module("AssImp")
endif ()

# fmt
begin_dependency("fmt")
set(FMT_DOC OFF CACHE BOOL "Generate the doc target.")
set(FMT_INSTALL OFF CACHE BOOL "Generate the install target.")
set(FMT_TEST OFF CACHE BOOL "Generate the test target.")
add_subdirectory(cppformat SYSTEM)
target_link_libraries(Ghoul PUBLIC fmt)
set_target_properties(fmt PROPERTIES FOLDER "External")
end_dependency("fmt")

# scnlib
begin_dependency("scnlib")
add_subdirectory(scnlib SYSTEM)
target_link_libraries(Ghoul PUBLIC scn)
set_target_properties(scn PROPERTIES FOLDER "External")
end_dependency("scnlib")

# stb_image
begin_dependency("stb_image")
add_subdirectory(stb_image SYSTEM)
target_link_libraries(Ghoul PUBLIC stb_image)
set_target_properties(stb_image PROPERTIES FOLDER "External")
end_dependency("stb_image")

# Stackwalker
if (MSVC)
  begin_dependency("Stackwalker")
  set(StackWalker_DISABLE_TESTS ON CACHE BOOL "" FORCE)
  add_subdirectory(stackwalker SYSTEM)
  target_link_libraries(Ghoul PRIVATE StackWalker)
  set_target_properties(StackWalker PROPERTIES FOLDER "External")
  # The StackWalker CMakeLists file is doing weird things, so we need to patch it here
  target_include_directories(StackWalker PUBLIC stackwalker/Main/StackWalker)
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
set_target_properties(websocketpp PROPERTIES FOLDER "External")
set_target_properties(websocketpp-build PROPERTIES FOLDER "External")
set_target_properties(websocketpp-configure PROPERTIES FOLDER "External")
end_dependency("WebSocketPP")

# Freetype
if (GHOUL_MODULE_FONTRENDERING)
  begin_module("FontRendering")
  set(FREETYPE_DIST OFF CACHE BOOL "")
  add_subdirectory(freetype2 SYSTEM)
  target_link_libraries(Ghoul PUBLIC freetype)
  set_target_properties(freetype PROPERTIES FOLDER "External")

  if (TARGET zlib)
    set_target_properties(zlib PROPERTIES FOLDER "External")
  endif ()
  end_module("FontRendering")
endif ()

# Tiny process library
begin_dependency("tiny-process-library")
add_subdirectory(tiny-process-library SYSTEM)
target_link_libraries(Ghoul PRIVATE tiny-process-library)
set_target_properties(tiny-process-library PROPERTIES FOLDER "External")
end_dependency("tiny-process-library")


# Tracy
begin_dependency("Profiling")
if (NOT TARGET tracy)
  add_library(tracy "${PROJECT_SOURCE_DIR}/ext/tracy/TracyClient.cpp")
  target_include_directories(tracy SYSTEM PUBLIC "${PROJECT_SOURCE_DIR}/ext/tracy")
  set_target_properties(tracy PROPERTIES FOLDER "External")
endif ()
if (GHOUL_PROFILING_ENABLE_TRACY)
  target_compile_definitions(tracy PUBLIC "TRACY_ENABLE")
  target_link_libraries(Ghoul PUBLIC tracy)
  if (UNIX)
    target_link_libraries(Ghoul PUBLIC dl)
  endif ()
endif ()
end_dependency("Profiling")

if (WIN32)
  option(GHOUL_ENABLE_EDIT_CONTINUE "Enable Edit&Continue" ON)

  if (GHOUL_PROFILING_ENABLE_TRACY)
    message(WARNING "Tracy does not support compiling with Edit&Continue, so we have to disable that feature")
    set(GHOUL_ENABLE_EDIT_CONTINUE OFF CACHE BOOL "" FORCE)
  endif ()
endif ()

