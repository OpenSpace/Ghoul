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

set(GHOUL_SOURCE 
    ${PROJECT_SOURCE_DIR}/src/cmdparser/commandlinecommand.cpp
    ${PROJECT_SOURCE_DIR}/src/cmdparser/commandlineparser.cpp
    ${PROJECT_SOURCE_DIR}/src/cmdparser/multiplecommand.cpp
    ${PROJECT_SOURCE_DIR}/src/cmdparser/singlecommand.cpp
    ${PROJECT_SOURCE_DIR}/src/exception/exception.cpp
    ${PROJECT_SOURCE_DIR}/src/filesystem/cachemanager.cpp
    ${PROJECT_SOURCE_DIR}/src/filesystem/directory.cpp
    ${PROJECT_SOURCE_DIR}/src/filesystem/file.cpp
    ${PROJECT_SOURCE_DIR}/src/filesystem/filesystem.cpp
    ${PROJECT_SOURCE_DIR}/src/filesystem/filesystem.linux.cpp
    ${PROJECT_SOURCE_DIR}/src/filesystem/filesystem.osx.cpp
    ${PROJECT_SOURCE_DIR}/src/filesystem/filesystem.windows.cpp
    ${PROJECT_SOURCE_DIR}/src/io/rawvolumereader.cpp
    ${PROJECT_SOURCE_DIR}/src/io/volumereader.cpp
    ${PROJECT_SOURCE_DIR}/src/io/model/modelreaderlua.cpp
    ${PROJECT_SOURCE_DIR}/src/io/model/modelreaderwavefront.cpp
    ${PROJECT_SOURCE_DIR}/src/io/texture/texturereader.cpp
    ${PROJECT_SOURCE_DIR}/src/io/texture/texturereadercmap.cpp
    ${PROJECT_SOURCE_DIR}/src/io/texture/texturereaderdevil.cpp
    ${PROJECT_SOURCE_DIR}/src/io/texture/texturereaderfreeimage.cpp
    ${PROJECT_SOURCE_DIR}/src/io/texture/texturereadersoil.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/bufferlog.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/callbacklog.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/consolelog.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/htmllog.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/log.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/logmanager.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/streamlog.cpp
    ${PROJECT_SOURCE_DIR}/src/logging/textlog.cpp
    ${PROJECT_SOURCE_DIR}/src/lua/lua_helper.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/assert.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/buffer.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/clipboard.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/crc32.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/dictionary.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/highresclock.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/misc.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/sharedmemory.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/templatefactory.cpp
    ${PROJECT_SOURCE_DIR}/src/misc/typeinfo.cpp
    ${PROJECT_SOURCE_DIR}/src/opengl/framebufferobject.cpp
    ${PROJECT_SOURCE_DIR}/src/opengl/programobject.cpp
    ${PROJECT_SOURCE_DIR}/src/opengl/shadermanager.cpp
    ${PROJECT_SOURCE_DIR}/src/opengl/shaderobject.cpp
    ${PROJECT_SOURCE_DIR}/src/opengl/texture.cpp
    ${PROJECT_SOURCE_DIR}/src/opengl/texturemanager.cpp
    ${PROJECT_SOURCE_DIR}/src/opengl/textureunit.cpp
    ${PROJECT_SOURCE_DIR}/src/opengl/vertexbufferobject.cpp
    ${PROJECT_SOURCE_DIR}/src/systemcapabilities/generalcapabilitiescomponent.cpp
    ${PROJECT_SOURCE_DIR}/src/systemcapabilities/openglcapabilitiescomponent.cpp
    ${PROJECT_SOURCE_DIR}/src/systemcapabilities/systemcapabilities.cpp
    ${PROJECT_SOURCE_DIR}/src/systemcapabilities/systemcapabilitiescomponent.cpp
)

set(GHOUL_HEADER 
    ${PROJECT_SOURCE_DIR}/include/ghoul/cmdparser/cmdparser
    ${PROJECT_SOURCE_DIR}/include/ghoul/cmdparser/commandlinecommand.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/cmdparser/commandlineparser.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/cmdparser/multiplecommand.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/cmdparser/multiplecommand.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/cmdparser/singlecommand.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/cmdparser/singlecommand.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/designpattern/observer.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/designpattern/observer.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/designpattern/singleton.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/exception/exception.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/filesystem/cachemanager.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/filesystem/directory.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/filesystem/file.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/filesystem/filesystem
    ${PROJECT_SOURCE_DIR}/include/ghoul/filesystem/filesystem.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/rawvolumereader.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/volumereader.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/model/modelreaderbase.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/model/modelreaderlua.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/model/modelreaderwavefront.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/texture/texturereader.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/texture/texturereaderbase.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/texture/texturereadercmap.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/texture/texturereaderdevil.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/texture/texturereaderfreeimage.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/io/texture/texturereadersoil.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/bufferlog.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/callbacklog.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/consolelog.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/htmllog.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/log.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/logging
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/logmanager.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/logmanager.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/streamlog.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/logging/textlog.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/lua/ghoul_lua.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/lua/lua_helper.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/assert.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/buffer.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/buffer.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/clipboard.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/crc32.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/dictionary.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/dictionary.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/highresclock.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/misc.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/sharedmemory.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/templatefactory.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/templatefactory.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/misc/typeinfo.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/framebufferobject.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/ghoul_gl.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/opengl
    ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/programobject.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/shadermanager.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/shaderobject.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/texture.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/texture.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/texturemanager.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/textureunit.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/vertexbufferobject.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/opengl/vertexbufferobject.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/systemcapabilities/generalcapabilitiescomponent.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/systemcapabilities/openglcapabilitiescomponent.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/systemcapabilities/systemcapabilities
    ${PROJECT_SOURCE_DIR}/include/ghoul/systemcapabilities/systemcapabilities.h
    ${PROJECT_SOURCE_DIR}/include/ghoul/systemcapabilities/systemcapabilities.inl
    ${PROJECT_SOURCE_DIR}/include/ghoul/systemcapabilities/systemcapabilitiescomponent.h
)

# Place files into a source_group
foreach(file ${GHOUL_SOURCE} ${GHOUL_HEADER})
    # Remove the prefixes from the files 
    set(original_file ${file})
    string(REPLACE "${PROJECT_SOURCE_DIR}/src/" "" file ${file})
    string(REPLACE "${PROJECT_SOURCE_DIR}/include/ghoul/" "" file ${file})
    get_filename_component(directory ${file} DIRECTORY)
    string(REPLACE "/" "\\" directory ${directory})
    string(SUBSTRING ${directory} 0 1 FIRST_LETTER)
    string(TOUPPER ${FIRST_LETTER} FIRST_LETTER)
    string(REGEX REPLACE "^.(.*)" "${FIRST_LETTER}\\1" directory "${directory}")
    source_group("${directory}" FILES ${original_file})
endforeach()

# Hide the variables from the GUI
mark_as_advanced(GHOUL_SOURCE GHOUL_HEADER)
