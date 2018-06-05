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

set(GHOUL_SOURCE 
    ${PROJECT_SOURCE_DIR}/src/filesystem/cachemanager.cpp
    ${PROJECT_SOURCE_DIR}/src/filesystem/directory.cpp
    ${PROJECT_SOURCE_DIR}/src/filesystem/file.cpp
    ${PROJECT_SOURCE_DIR}/src/filesystem/filesystem.cpp
    ${PROJECT_SOURCE_DIR}/src/filesystem/filesystem.linux.cpp
    ${PROJECT_SOURCE_DIR}/src/filesystem/filesystem.osx.cpp
    ${PROJECT_SOURCE_DIR}/src/filesystem/filesystem.windows.cpp
    ${PROJECT_SOURCE_DIR}/src/io/model/modelreaderbase.cpp
    ${PROJECT_SOURCE_DIR}/src/io/model/modelreadermultiformat.cpp
    ${PROJECT_SOURCE_DIR}/src/io/socket/socket.cpp
    ${PROJECT_SOURCE_DIR}/src/io/socket/tcpsocket.cpp
    ${PROJECT_SOURCE_DIR}/src/io/socket/tcpsocketserver.cpp
    ${PROJECT_SOURCE_DIR}/src/io/socket/websocket.cpp
    ${PROJECT_SOURCE_DIR}/src/io/socket/websocketserver.cpp
    ${PROJECT_SOURCE_DIR}/src/io/volume/rawvolumereader.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/bufferlog.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/callbacklog.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/consolelog.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/htmllog.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/log.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/loglevel.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/logmanager.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/streamlog.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/textlog.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/visualstudiooutputlog.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/any.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/assert.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/buffer.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/clipboard.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/crc32.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/csvreader.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/dictionary.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/dictionaryjsonformatter.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/dictionaryluaformatter.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/easing.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/exception.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/misc.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/sharedmemory.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/stacktrace.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/templatefactory.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/thread.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/threadpool.cpp
    ${PROJECT_SOURCE_DIR}/src/ghoul.cpp
)

if (GHOUL_MODULE_COMMANDLINEPARSER)
    set(GHOUL_SOURCE
        ${GHOUL_SOURCE}
        ${PROJECT_SOURCE_DIR}/src/cmdparser/commandlinecommand.cpp
        ${PROJECT_SOURCE_DIR}/src/cmdparser/commandlineparser.cpp
        ${PROJECT_SOURCE_DIR}/src/cmdparser/multiplecommand.cpp
        ${PROJECT_SOURCE_DIR}/src/cmdparser/singlecommand.cpp
    )
endif ()

if (GHOUL_MODULE_FONTRENDERING)
    set(GHOUL_SOURCE
        ${GHOUL_SOURCE}
        ${PROJECT_SOURCE_DIR}/src/font/font.cpp
        ${PROJECT_SOURCE_DIR}/src/font/fontmanager.cpp
        ${PROJECT_SOURCE_DIR}/src/font/fontrenderer.cpp
    )
endif ()

if (GHOUL_MODULE_LUA)
    set(GHOUL_SOURCE
        ${GHOUL_SOURCE}
        ${PROJECT_SOURCE_DIR}/src/io/model/modelreaderlua.cpp
        ${PROJECT_SOURCE_DIR}/src/lua/lua_helper.cpp
        ${PROJECT_SOURCE_DIR}/src/lua/luastate.cpp
    )
endif ()

if (GHOUL_MODULE_OPENGL)
    set(GHOUL_SOURCE
        ${GHOUL_SOURCE}
        ${PROJECT_SOURCE_DIR}/src/opengl/bufferbinding.cpp
        ${PROJECT_SOURCE_DIR}/src/opengl/debugcontext.cpp
        ${PROJECT_SOURCE_DIR}/src/opengl/framebufferobject.cpp
        ${PROJECT_SOURCE_DIR}/src/opengl/programobject.cpp
        ${PROJECT_SOURCE_DIR}/src/opengl/programobjectmanager.cpp
        ${PROJECT_SOURCE_DIR}/src/opengl/shadermanager.cpp
        ${PROJECT_SOURCE_DIR}/src/opengl/shaderobject.cpp
        ${PROJECT_SOURCE_DIR}/src/opengl/shaderpreprocessor.cpp
        ${PROJECT_SOURCE_DIR}/src/opengl/texture.cpp
        ${PROJECT_SOURCE_DIR}/src/opengl/textureatlas.cpp
        ${PROJECT_SOURCE_DIR}/src/opengl/textureconversion.cpp
        ${PROJECT_SOURCE_DIR}/src/opengl/texturemanager.cpp
        ${PROJECT_SOURCE_DIR}/src/opengl/textureunit.cpp
        ${PROJECT_SOURCE_DIR}/src/opengl/vertexbufferobject.cpp
        ${PROJECT_SOURCE_DIR}/src/io/texture/texturereader.cpp
        ${PROJECT_SOURCE_DIR}/src/io/texture/texturereaderbase.cpp
        ${PROJECT_SOURCE_DIR}/src/io/texture/texturereadercmap.cpp
        ${PROJECT_SOURCE_DIR}/src/io/texture/texturereaderdevil.cpp
        ${PROJECT_SOURCE_DIR}/src/io/texture/texturereaderfreeimage.cpp
        ${PROJECT_SOURCE_DIR}/src/io/texture/texturereadersoil.cpp
        ${PROJECT_SOURCE_DIR}/src/io/texture/texturereaderstb.cpp
        ${PROJECT_SOURCE_DIR}/src/io/texture/texturewriter.cpp
        ${PROJECT_SOURCE_DIR}/src/io/texture/texturewriterbase.cpp
        ${PROJECT_SOURCE_DIR}/src/io/texture/texturewritersoil.cpp
    )
endif ()

if (GHOUL_MODULE_SYSTEMCAPABILITIES)
    set(GHOUL_SOURCE
        ${GHOUL_SOURCE}
        ${PROJECT_SOURCE_DIR}/src/systemcapabilities/generalcapabilitiescomponent.cpp
        ${PROJECT_SOURCE_DIR}/src/systemcapabilities/openglcapabilitiescomponent.cpp
        ${PROJECT_SOURCE_DIR}/src/systemcapabilities/systemcapabilities.cpp
        ${PROJECT_SOURCE_DIR}/src/systemcapabilities/systemcapabilitiescomponent.cpp
        ${PROJECT_SOURCE_DIR}/src/systemcapabilities/version.cpp
    )
endif ()

set(GHOUL_HEADER 
    ${PROJECT_SOURCE_DIR}/include/ghoul/designpattern/singleton.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/designpattern/event.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/designpattern/event.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/filesystem/cachemanager.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/filesystem/directory.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/filesystem/file.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/filesystem/filesystem.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/model/modelreaderbase.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/model/modelreadermultiformat.h
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
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/streamlog.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/textlog.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/visualstudiooutputlog.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/any.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/any.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/assert.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/boolean.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/buffer.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/buffer.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/clipboard.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/constexpr.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/crc32.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/crc32.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/csvreader.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/defer.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/dictionary.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/dictionary.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/dictionaryformatter.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/dictionaryjsonformatter.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/dictionaryluaformatter.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/easing.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/easing.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/exception.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/fromstring.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/interpolator.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/interpolator.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/invariants.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/misc.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/sharedmemory.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/stacktrace.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/supportmacros.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/templatefactory.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/templatefactory.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/thread.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/threadpool.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/threadpool.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/ghoul.h
)

if (GHOUL_MODULE_COMMANDLINEPARSER)
    set(GHOUL_HEADER
        ${GHOUL_HEADER}
        ${PROJECT_SOURCE_DIR}/include/ghoul/cmdparser/commandlinecommand.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/cmdparser/commandlinecommand.inl
        ${PROJECT_SOURCE_DIR}/include/ghoul/cmdparser/commandlineparser.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/cmdparser/multiplecommand.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/cmdparser/multiplecommand.inl
        ${PROJECT_SOURCE_DIR}/include/ghoul/cmdparser/singlecommand.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/cmdparser/singlecommand.inl
    )
endif ()

if (GHOUL_MODULE_FONTRENDERING)
    set(GHOUL_HEADER
        ${GHOUL_HEADER}
        ${PROJECT_SOURCE_DIR}/include/ghoul/font/font.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/font/fontmanager.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/font/fontrenderer.h
    )
endif ()

if (GHOUL_MODULE_LUA)
    set(GHOUL_HEADER
        ${GHOUL_HEADER}
        ${PROJECT_SOURCE_DIR}/include/ghoul/io/model/modelreaderlua.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/lua/ghoul_lua.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/lua/lua_helper.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/lua/lua_helper.inl
        ${PROJECT_SOURCE_DIR}/include/ghoul/lua/luastate.h
    )
endif ()

if (GHOUL_MODULE_OPENGL)
    set(GHOUL_HEADER
        ${GHOUL_HEADER}
        ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/bufferbinding.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/bufferbinding.inl
        ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/debugcontext.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/framebufferobject.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/ghoul_gl.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/programobject.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/programobjectmanager.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/shadermanager.h
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
        ${PROJECT_SOURCE_DIR}/include/ghoul/io/texture/texturereaderdevil.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/io/texture/texturereaderfreeimage.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/io/texture/texturereadersoil.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/io/texture/texturereaderstb.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/io/texture/texturewriter.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/io/texture/texturewriterbase.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/io/texture/texturewritersoil.h
    )
endif ()


if (GHOUL_MODULE_SYSTEMCAPABILITIES)
    set(GHOUL_HEADER
        ${GHOUL_HEADER}
        ${PROJECT_SOURCE_DIR}/include/ghoul/systemcapabilities/generalcapabilitiescomponent.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/systemcapabilities/openglcapabilitiescomponent.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/systemcapabilities/systemcapabilities.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/systemcapabilities/systemcapabilities.inl
        ${PROJECT_SOURCE_DIR}/include/ghoul/systemcapabilities/systemcapabilitiescomponent.h
        ${PROJECT_SOURCE_DIR}/include/ghoul/systemcapabilities/version.h
    )
endif ()


include (${GHOUL_CMAKE_DIR}/GroupFilesIntoFolders.cmake)
group_files_into_folders(${GHOUL_SOURCE} ${GHOUL_HEADER})


# if (MSVC)
#     set(GHOUL_SOURCE
#         ${GHOUL_SOURCE}
#         ${PROJECT_SOURCE_DIR}/ext/stackwalker/StackWalker.cpp
#     )

#     set(GHOUL_HEADER
#         ${GHOUL_HEADER}
#         ${PROJECT_SOURCE_DIR}/ext/stackwalker/StackWalker.h
#     )

#     source_group(
#         "External"
#         FILES
#         ${PROJECT_SOURCE_DIR}/ext/stackwalker/StackWalker.cpp
#         ${PROJECT_SOURCE_DIR}/ext/stackwalker/StackWalker.h
#     )
# endif ()
