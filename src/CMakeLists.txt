##########################################################################################
#                                                                                        #
# GHOUL                                                                                  #
# General Helpful Open Utility Library                                                   #
#                                                                                        #
# Copyright (c) 2012-2025                                                                #
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

include(${GHOUL_ROOT_DIR}/support/cmake/set_ghoul_compile_settings.cmake)

add_library(Ghoul STATIC)
set_ghoul_compile_settings(Ghoul)
target_include_directories(Ghoul PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_sources(
  Ghoul
  PRIVATE
    ${PROJECT_SOURCE_DIR}/include/ghoul/format.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/freetype.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/ghoul.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/glm.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/designpattern/event.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/designpattern/event.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/filesystem/cachemanager.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/filesystem/file.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/filesystem/filesystem.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/model/modelanimation.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/model/modelgeometry.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/model/modelmesh.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/model/modelnode.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/model/modelreader.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/model/modelreaderbase.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/model/modelreaderbinary.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/socket/socket.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/socket/socketserver.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/socket/sockettype.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/socket/tcpsocket.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/socket/tcpsocket.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/socket/tcpsocketserver.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/socket/websocket.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/socket/websocketserver.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/volume/rawvolumereader.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/volume/volumereader.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/bufferlog.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/callbacklog.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/consolelog.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/htmllog.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/log.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/loglevel.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/logmanager.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/logmanager.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/textlog.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/visualstudiooutputlog.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/assert.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/boolean.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/clipboard.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/constexpr.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/constmap.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/crc32.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/crc32.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/csvreader.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/defer.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/dictionary.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/dictionaryjsonformatter.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/dictionaryluaformatter.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/easing.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/easing.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/exception.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/integration.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/integration.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/interpolator.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/interpolator.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/invariants.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/managedmemoryuniqueptr.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/memorypool.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/memorypool.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/objectmanager.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/objectmanager.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/profiling.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/sharedmemory.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/stacktrace.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/stringconversion.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/stringhelper.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/supportmacros.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/templatefactory.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/templatefactory.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/thread.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/threadpool.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/threadpool.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/process.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/ghoul.h

    filesystem/cachemanager.cpp
    filesystem/file.cpp
    filesystem/filesystem.cpp
    filesystem/filesystem.linux.cpp
    filesystem/filesystem.osx.cpp
    filesystem/filesystem.windows.cpp
    io/model/modelanimation.cpp
    io/model/modelgeometry.cpp
    io/model/modelmesh.cpp
    io/model/modelnode.cpp
    io/model/modelreader.cpp
    io/model/modelreaderbase.cpp
    io/model/modelreaderbinary.cpp
    io/socket/socket.cpp
    io/socket/tcpsocket.cpp
    io/socket/tcpsocketserver.cpp
    io/socket/websocket.cpp
    io/socket/websocketserver.cpp
    io/volume/rawvolumereader.cpp
    logging/bufferlog.cpp
    logging/callbacklog.cpp
    logging/consolelog.cpp
    logging/htmllog.cpp
    logging/log.cpp
    logging/logmanager.cpp
    logging/textlog.cpp
    logging/visualstudiooutputlog.cpp
    misc/assert.cpp
    misc/clipboard.cpp
    misc/crc32.cpp
    misc/csvreader.cpp
    misc/dictionary.cpp
    misc/dictionaryjsonformatter.cpp
    misc/dictionaryluaformatter.cpp
    misc/easing.cpp
    misc/exception.cpp
    misc/interpolator.cpp
    misc/sharedmemory.cpp
    misc/stacktrace.cpp
    misc/stringhelper.cpp
    misc/templatefactory.cpp
    misc/thread.cpp
    misc/threadpool.cpp
    misc/process.cpp
    ghoul.cpp
)

if (GHOUL_MODULE_ASSIMP)
  target_sources(
    Ghoul
    PRIVATE
      ${PROJECT_SOURCE_DIR}/include/ghoul/io/model/modelreaderassimp.h
      io/model/modelreaderassimp.cpp
  )
endif ()

if (GHOUL_MODULE_COMMANDLINEPARSER)
  target_sources(
    Ghoul
    PRIVATE
      ${PROJECT_SOURCE_DIR}/include/ghoul/cmdparser/commandlinecommand.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/cmdparser/commandlinecommand.inl
      ${PROJECT_SOURCE_DIR}/include/ghoul/cmdparser/commandlineparser.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/cmdparser/multiplecommand.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/cmdparser/multiplecommand.inl
      ${PROJECT_SOURCE_DIR}/include/ghoul/cmdparser/singlecommand.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/cmdparser/singlecommand.inl
      cmdparser/commandlinecommand.cpp
      cmdparser/commandlineparser.cpp
      cmdparser/multiplecommand.cpp
      cmdparser/singlecommand.cpp
  )
endif ()

if (GHOUL_MODULE_FONTRENDERING)
  target_sources(
    Ghoul
    PRIVATE
      ${PROJECT_SOURCE_DIR}/include/ghoul/font/font.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/font/fonterrors.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/font/fontmanager.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/font/fontrenderer.h
      font/font.cpp
      font/fonterrors.cpp
      font/fontmanager.cpp
      font/fontrenderer.cpp
  )
endif ()

if (GHOUL_MODULE_LUA)
  target_sources(
    Ghoul
    PRIVATE
      ${PROJECT_SOURCE_DIR}/include/ghoul/lua/ghoul_lua.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/lua/lua_helper.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/lua/lua_helper.inl
      ${PROJECT_SOURCE_DIR}/include/ghoul/lua/luastate.h
      lua/lua_helper.cpp
      lua/luastate.cpp
  )
endif ()

if (GHOUL_MODULE_OPENGL)
  target_sources(
    Ghoul
    PRIVATE
      ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/bufferbinding.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/bufferbinding.inl
      ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/debugcontext.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/framebufferobject.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/ghoul_gl.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/openglstatecache.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/programobject.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/programobjectmanager.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/renderdoc.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/shaderobject.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/shaderpreprocessor.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/texture.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/textureatlas.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/textureconversion.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/texture.inl
      ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/texturemanager.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/textureunit.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/uniformcache.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/vertexbufferobject.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/vertexbufferobject.inl
      ${PROJECT_SOURCE_DIR}/include/ghoul/io/texture/texturereader.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/io/texture/texturereaderbase.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/io/texture/texturereadercmap.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/io/texture/texturereaderstb.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/io/texture/texturewriter.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/io/texture/texturewriterbase.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/io/texture/texturewriterstb.h
      opengl/bufferbinding.cpp
      opengl/debugcontext.cpp
      opengl/framebufferobject.cpp
      opengl/openglstatecache.cpp
      opengl/programobject.cpp
      opengl/programobjectmanager.cpp
      opengl/renderdoc.cpp
      opengl/shaderobject.cpp
      opengl/shaderpreprocessor.cpp
      opengl/texture.cpp
      opengl/textureatlas.cpp
      opengl/textureconversion.cpp
      opengl/texturemanager.cpp
      opengl/textureunit.cpp
      opengl/vertexbufferobject.cpp
      io/texture/texturereader.cpp
      io/texture/texturereaderbase.cpp
      io/texture/texturereadercmap.cpp
      io/texture/texturereaderstb.cpp
      io/texture/texturewriter.cpp
      io/texture/texturewriterbase.cpp
      io/texture/texturewriterstb.cpp
  )
endif ()

if (GHOUL_MODULE_SYSTEMCAPABILITIES)
  target_sources(
    Ghoul
    PRIVATE
      ${PROJECT_SOURCE_DIR}/include/ghoul/systemcapabilities/generalcapabilitiescomponent.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/systemcapabilities/openglcapabilitiescomponent.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/systemcapabilities/systemcapabilities.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/systemcapabilities/systemcapabilities.inl
      ${PROJECT_SOURCE_DIR}/include/ghoul/systemcapabilities/systemcapabilitiescomponent.h
      ${PROJECT_SOURCE_DIR}/include/ghoul/systemcapabilities/version.h
      systemcapabilities/generalcapabilitiescomponent.cpp
      systemcapabilities/openglcapabilitiescomponent.cpp
      systemcapabilities/systemcapabilities.cpp
      systemcapabilities/systemcapabilitiescomponent.cpp
      systemcapabilities/version.cpp
  )
endif ()


target_precompile_headers(Ghoul PRIVATE
  [["ghoul/format.h"]]
  [["ghoul/glm.h"]]
  [["ghoul/misc/assert.h"]]
  [["ghoul/misc/boolean.h"]]
  [["ghoul/misc/exception.h"]]
  [["ghoul/opengl/ghoul_gl.h"]]
  <algorithm>
  <array>
  <chrono>
  <filesystem>
  <map>
  <memory>
  <string>
  <string_view>
  <vector>
)

if (GHOUL_HIGH_DEBUG_MODE)
  target_compile_definitions(Ghoul PUBLIC "GHL_DEBUG")
endif ()

if (GHOUL_LOGGING_ENABLE_TRACE)
  target_compile_definitions(Ghoul PUBLIC "GHOUL_LOGGING_ENABLE_TRACE")
endif ()

if (GHOUL_THROW_ON_ASSERT)
  target_compile_definitions(Ghoul PUBLIC "GHL_THROW_ON_ASSERT")
endif ()

# Place files into source_group
get_target_property(source_files Ghoul SOURCES)
foreach (file ${source_files})
  # Remove prefixes from the files
  set(original_file ${file})
  string(REPLACE "${PROJECT_SOURCE_DIR}/src/" "" file ${file})
  string(REPLACE "${PROJECT_SOURCE_DIR}/include/ghoul/" "" file ${file})

  # Extract the directory part of the rest
  get_filename_component(directory ${file} DIRECTORY)
  if (NOT ${directory} STREQUAL "")
    if (MSVC)
      string(REPLACE "/" "\\" directory ${directory})
    endif ()

    # Make the directory's first letter upper case
    string(SUBSTRING ${directory} 0 1 FIRST_LETTER)
    string(TOUPPER ${FIRST_LETTER} FIRST_LETTER)
    string(REGEX REPLACE "^.(.*)" "${FIRST_LETTER}\\1" directory "${directory}")
    source_group("${directory}" FILES ${original_file})
  else ()
    source_group("" FILES ${original_file})
  endif ()
endforeach ()
