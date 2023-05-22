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
find_package(glm CONFIG REQUIRED)
target_link_libraries(Ghoul PUBLIC glm::glm)
end_dependency("GLM")

if (GHOUL_MODULE_OPENGL)
  begin_module("OpenGL")

  # OpenGL
  begin_dependency("OpenGL")
  find_package(OpenGL REQUIRED)
  target_link_libraries(Ghoul PUBLIC OpenGL::GL)
  end_dependency("OpenGL")

  # glbindings
  if (NOT TARGET glbinding)
    begin_dependency("glbinding")
    find_package(glbinding CONFIG REQUIRED)
    target_link_libraries(Ghoul PUBLIC glbinding::glbinding glbinding::glbinding-aux)
    end_dependency("glbinding")
  endif ()

  end_module("OpenGL")
endif ()

# LZ4
begin_dependency("LZ4")
find_package(lz4 CONFIG REQUIRED)
target_link_libraries(Ghoul PRIVATE lz4::lz4)
end_dependency("LZ4")

# Lua
if (GHOUL_MODULE_LUA)
  begin_module("Lua")
  find_package(Lua REQUIRED)
  target_include_directories(Ghoul PUBLIC ${LUA_INCLUDE_DIR})
  target_link_libraries(Ghoul PUBLIC ${LUA_LIBRARIES})
  end_module("Lua")
endif ()


# Assimp
if (GHOUL_MODULE_ASSIMP)
  begin_module("AssImp")
  find_package(assimp CONFIG REQUIRED)
  target_link_libraries(Ghoul PRIVATE assimp::assimp)
  end_module("AssImp")
endif ()

# fmt
begin_dependency("fmt")
find_package(fmt CONFIG REQUIRED)
target_link_libraries(Ghoul PUBLIC fmt::fmt)
end_dependency("fmt")

# scnlib
begin_dependency("scnlib")
find_package(scn CONFIG REQUIRED)
target_link_libraries(Ghoul PUBLIC scn::scn)
end_dependency("scnlib")

# stb_image
begin_dependency("stb_image")
add_subdirectory(stb_image)
target_link_libraries(Ghoul PUBLIC stb_image)
set_target_properties(stb_image PROPERTIES FOLDER "External")
end_dependency("stb_image")

# Stackwalker
if (MSVC)
  begin_dependency("Stackwalker")
  set(StackWalker_DISABLE_TESTS ON CACHE BOOL "" FORCE)
  add_subdirectory(stackwalker)
  target_link_libraries(Ghoul PRIVATE StackWalker)
  set_target_properties(StackWalker PROPERTIES FOLDER "External")
  # The StackWalker CMakeLists file is doing weird things, so we need to patch it here
  target_include_directories(StackWalker PUBLIC stackwalker/Main/StackWalker)
  end_dependency("Stackwalker")
endif ()

# WebSocket++ / WebSocketPP
begin_dependency("WebSocketPP")
find_package(websocketpp CONFIG REQUIRED)
target_link_libraries(Ghoul PUBLIC websocketpp::websocketpp)
end_dependency("WebSocketPP")

# Freetype
if (GHOUL_MODULE_FONTRENDERING)
  begin_module("FontRendering")
  find_package(Freetype REQUIRED)
  target_link_libraries(Ghoul PUBLIC Freetype::Freetype)
  end_module("FontRendering")
endif ()

# Tiny process library
begin_dependency("tiny-process-library")
find_package(tiny-process-library CONFIG REQUIRED)
target_link_libraries(Ghoul PRIVATE tiny-process-library::tiny-process-library)
end_dependency("tiny-process-library")


# Tracy
begin_dependency("Tracy")
if (NOT TARGET TracyClient)
  option(TRACY_ENABLE "Enable profiling" OFF)
  option(TRACY_NO_FRAME_IMAGE "Disable the frame image support and its thread" ON)
  add_subdirectory(tracy)
  set_target_properties(TracyClient PROPERTIES FOLDER "External")
endif ()
target_link_libraries(Ghoul PUBLIC TracyClient)
end_dependency("Tracy")

# RenderDoc
begin_dependency("RenderDoc")
add_subdirectory(renderdoc)
target_link_libraries(Ghoul PUBLIC renderdoc)
end_dependency("RenderDoc")
