##########################################################################################
#                                                                                        #
# GHOUL                                                                                  #
# General Helpful Open Utility Library                                                   #
#                                                                                        #
# Copyright (c) 2012-2026                                                                #
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

# All third-party dependencies are provided by vcpkg, see the root vcpkg.json. The only
# exceptions are stb and RenderDoc, which stay vendored because they consist of a single
# translation unit that has to be compiled with Ghoul-specific settings.

# System Libraries
if (WIN32)
  target_link_libraries(Ghoul PRIVATE "shlwapi" "WbemUuid.lib" "Ws2_32.lib")
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
find_package(glm CONFIG REQUIRED)
target_link_libraries(Ghoul PUBLIC glm::glm)
end_dependency("GLM")

# LZ4
begin_dependency("LZ4")
find_package(lz4 CONFIG REQUIRED)
target_link_libraries(Ghoul PRIVATE lz4::lz4)
end_dependency("LZ4")

# scnlib
begin_dependency("scnlib")
find_package(scn CONFIG REQUIRED)
target_link_libraries(Ghoul PUBLIC scn::scn)
end_dependency("scnlib")

# stb_image
# The headers come from vcpkg, but the translation unit instantiating them is ours as it
# selects the stb feature macros
begin_dependency("stb_image")
find_path(STB_INCLUDE_DIR "stb_image.h" REQUIRED)
add_subdirectory(stb_image)
target_sources(Ghoul PRIVATE $<TARGET_OBJECTS:stb_image>)
target_include_directories(Ghoul SYSTEM PRIVATE ${STB_INCLUDE_DIR})
set_target_properties(stb_image PROPERTIES FOLDER "External")
end_dependency("stb_image")

# WebSocket++ / WebSocketPP
# Header only and without a CMake package, so it is located through its include directory
begin_dependency("WebSocketPP")
find_path(WEBSOCKETPP_INCLUDE_DIR "websocketpp/version.hpp" REQUIRED)
target_include_directories(Ghoul SYSTEM PUBLIC $<BUILD_INTERFACE:${WEBSOCKETPP_INCLUDE_DIR}>)
end_dependency("WebSocketPP")

# Tiny process library
begin_dependency("tiny-process-library")
find_package(tiny-process-library CONFIG REQUIRED)
target_link_libraries(Ghoul PRIVATE tiny-process-library::tiny-process-library)
end_dependency("tiny-process-library")

# RenderDoc
# A single vendored header that is included by the public ghoul/opengl/renderdoc.h
begin_dependency("RenderDoc")
target_include_directories(
  Ghoul SYSTEM PUBLIC $<BUILD_INTERFACE:${GHOUL_ROOT_DIR}/ext/renderdoc>
)
end_dependency("RenderDoc")

# Tracy
if (TRACY_ENABLE)
  begin_dependency("Tracy")
  find_package(Tracy CONFIG REQUIRED)
  target_link_libraries(Ghoul PUBLIC Tracy::TracyClient)
  target_compile_definitions(Ghoul PUBLIC "TRACY_ENABLE")
  end_dependency("Tracy")
endif ()

# OpenGL
if (GHOUL_MODULE_OPENGL)
  begin_module("OpenGL")

  begin_dependency("OpenGL")
  find_package(OpenGL REQUIRED)
  target_link_libraries(Ghoul PUBLIC OpenGL::GL)
  end_dependency("OpenGL")

  begin_dependency("glbinding")
  find_package(glbinding CONFIG REQUIRED)
  target_link_libraries(Ghoul PUBLIC glbinding::glbinding glbinding::glbinding-aux)
  end_dependency("glbinding")

  end_module("OpenGL")
endif ()

# Lua
if (GHOUL_MODULE_LUA)
  begin_module("Lua")
  find_package(unofficial-lua CONFIG REQUIRED)
  if (TARGET unofficial::lua::lua)
    target_link_libraries(Ghoul PUBLIC unofficial::lua::lua)
  else ()
    # Lua 5.4.7 exports an unnamespaced target, later versions do not
    target_link_libraries(Ghoul PUBLIC lua)
  endif ()
  end_module("Lua")
endif ()

# Assimp
if (GHOUL_MODULE_ASSIMP)
  begin_module("AssImp")
  find_package(assimp CONFIG REQUIRED)
  target_link_libraries(Ghoul PRIVATE assimp::assimp)
  end_module("AssImp")
endif ()

# Freetype
if (GHOUL_MODULE_FONTRENDERING)
  begin_module("FontRendering")
  find_package(Freetype REQUIRED)
  target_link_libraries(Ghoul PUBLIC Freetype::Freetype)
  end_module("FontRendering")
endif ()
