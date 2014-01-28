#########################################################################################
#                                                                                       #
# GHOUL                                                                                 #
#                                                                                       #
# Copyright (c) 2012 Alexander Bock                                                     #
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

######
# Source
######

set(SOURCE_ROOT_DIR ${GHOUL_ROOT_DIR}/src)
set(HEADER_ROOT_DIR ${GHOUL_ROOT_DIR}/include/ghoul)

file(GLOB CMDPARSER_SOURCE ${SOURCE_ROOT_DIR}/cmdparser/*.cpp)
file(GLOB CMDPARSER_SOURCE_C ${SOURCE_ROOT_DIR}/cmdparser/*.c)
set(GHOUL_SOURCE ${GHOUL_SOURCE} ${CMDPARSER_SOURCE} ${CMDPARSER_SOURCE_C})
file(GLOB CMDPARSER_HEADER ${HEADER_ROOT_DIR}/cmdparser/*.h)
file(GLOB CMDPARSER_HEADER_INL ${HEADER_ROOT_DIR}/cmdparser.inl)
set(CMDPARSER_GLOB_HEADER ${HEADER_ROOT_DIR}/cmdparser/cmdparser)
set(GHOUL_HEADER ${GHOUL_HEADER} ${CMDPARSER_HEADER} ${CMDPARSER_HEADER_INL})
source_group(CmdParser FILES
				${CMDPARSER_SOURCE}
				${CMDPARSER_SOURCE_C}
				${CMDPARSER_GLOB_HEADER}
				${CMDPARSER_HEADER}
				${CMDPARSER_HEADER_INL})

file(GLOB FILESYSTEM_SOURCE ${SOURCE_ROOT_DIR}/filesystem/*.cpp)
file(GLOB FILESYSTEM_SOURCE_C ${SOURCE_ROOT_DIR}/filesystem/*.c)
set(GHOUL_SOURCE ${GHOUL_SOURCE} ${FILESYSTEM_SOURCE} ${FILESYSTEM_SOURCE_C})
file(GLOB FILESYSTEM_HEADER ${HEADER_ROOT_DIR}/filesystem/*.h)
file(GLOB FILESYSTEM_HEADER_INL ${HEADER_ROOT_DIR}/filesystem/*.inl)
set(FILESYSTEM_GLOB_HEADER ${HEADER_ROOT_DIR}/filesystem/filesystem)
set(GHOUL_HEADER ${GHOUL_HEADER} ${FILESYSTEM_HEADER} ${FILESYSTEM_HEADER_INL})
source_group(FileSystem FILES
				${FILESYSTEM_SOURCE}
				${FILESYSTEM_SOURCE_C}
				${FILESYSTEM_GLOB_HEADER}
				${FILESYSTEM_HEADER}
				${FILESYSTEM_HEADER_INL})

file(GLOB LOG_SOURCE ${SOURCE_ROOT_DIR}/logging/*.cpp)
file(GLOB LOG_SOURCE_C ${SOURCE_ROOT_DIR}/logging/*.c)
set(GHOUL_SOURCE ${GHOUL_SOURCE} ${LOG_SOURCE} ${LOG_SOURCE_C})
file(GLOB LOG_HEADER ${HEADER_ROOT_DIR}/logging/*.h)
file(GLOB LOG_HEADER_INL ${HEADER_ROOT_DIR}/logging/*.inl)
set(LOG_GLOB_HEADER ${HEADER_ROOT_DIR}/logging/logging)
set(GHOUL_HEADER ${GHOUL_HEADER} ${LOG_HEADER} ${LOG_HEADER_INL})
source_group(LogManager FILES
				${LOG_SOURCE}
				${LOG_SOURCE_C}
				${LOG_GLOB_HEADER}
				${LOG_HEADER}
				${LOG_HEADER_INL})

file(GLOB LUA_SOURCE ${SOURCE_ROOT_DIR}/lua/*.cpp)
file(GLOB LUA_SOURCE_C ${SOURCE_ROOT_DIR}/lua/*.c)
set(GHOUL_SOURCE ${GHOUL_SOURCE} ${LUA_SOURCE} ${LUA_SOURCE_C})
file(GLOB LUA_HEADER ${HEADER_ROOT_DIR}/lua/*.h)
file(GLOB LUA_HEADER_INL ${HEADER_ROOT_DIR}/lua/*.inl)
set(GHOUL_HEADER ${GHOUL_HEADER} ${LUA_HEADER} ${LUA_HEADER_INL})
source_group(Lua FILES ${LUA_SOURCE} ${LUA_SOURCE_C} ${LUA_HEADER} ${LUA_HEADER_INL})

file(GLOB MISC_SOURCE ${SOURCE_ROOT_DIR}/misc/*.cpp)
file(GLOB MISC_SOURCE_C ${SOURCE_ROOT_DIR}/misc/*.c)
set(GHOUL_SOURCE ${GHOUL_SOURCE} ${MISC_SOURCE} ${MISC_SOURCE_C})
file(GLOB MISC_HEADER ${HEADER_ROOT_DIR}/misc/*.h)
file(GLOB MISC_HEADER_INL ${HEADER_ROOT_DIR}/misc/*.inl)
set(MISC_HEADER ${MISC_HEADER}
    ${GHOUL_ROOT_DIR}/include/ghoul/glm.h)
set(GHOUL_HEADER ${GHOUL_HEADER} ${MISC_HEADER} ${MISC_HEADER_INL})
source_group(Misc FILES ${MISC_SOURCE} ${MISC_SOURCE_C} ${MISC_HEADER} ${MISC_HEADER_INL})

file(GLOB OPENGL_SOURCE ${SOURCE_ROOT_DIR}/opengl/*.cpp)
file(GLOB OPENGL_SOURCE_C ${SOURCE_ROOT_DIR}/opengl/*.c)
set(GHOUL_SOURCE ${GHOUL_SOURCE} ${OPENGL_SOURCE} ${OPENGL_SOURCE_C})
file(GLOB OPENGL_HEADER ${HEADER_ROOT_DIR}/opengl/*.h)
file(GLOB OPENGL_HEADER_INL ${HEADER_ROOT_DIR}/opengl/*.inl)
set(OPENGL_GLOB_HEADER ${HEADER_ROOT_DIR}/opengl/opengl)
set(GHOUL_HEADER ${GHOUL_HEADER} ${OPENGL_HEADER} ${OPENGL_HEADER_INL})
source_group(OpenGL FILES
				${OPENGL_SOURCE}
				${OPENGL_SOURCE_C}
				${OPENGL_GLOB_HEADER}
				${OPENGL_HEADER}
				${OPENGL_HEADER_INL})

file(GLOB SYSTEMCAPABILITIES_SOURCE ${SOURCE_ROOT_DIR}/systemcapabilities/*.cpp)
file(GLOB SYSTEMCAPABILITIES_SOURCE_C ${SOURCE_ROOT_DIR}/systemcapabilities/*.c)
set(GHOUL_SOURCE ${GHOUL_SOURCE} ${SYSTEMCAPABILITIES_SOURCE} ${SYSTEMCAPABILITIES_SOURCE_C})
file(GLOB SYSTEMCAPABILITIES_HEADER ${HEADER_ROOT_DIR}/systemcapabilities/*.h)
file(GLOB SYSTEMCAPABILITIES_HEADER_INL ${HEADER_ROOT_DIR}/systemcapabilities/*.inl)
set(SYSTEMCAPABILITIES_GLOB_HEADER ${HEADER_ROOT_DIR}/systemcapabilities/systemcapabilities)
set(GHOUL_HEADER ${GHOUL_HEADER} ${SYSTEMCAPABILITIES_HEADER} ${SYSTEMCAPABILITIES_HEADER_INL})
source_group(SystemCapabilities FILES
				${SYSTEMCAPABILITIES_SOURCE}
				${SYSTEMCAPABILITIES_SOURCE_C}
				${SYSTEMCAPABILITIES_GLOB_HEADER}
				${SYSTEMCAPABILITIES_HEADER}
				${SYSTEMCAPABILITIES_HEADER_INL})

set(GHOUL_HEADER ${GHOUL_HEADER} ${HEADER_ROOT_DIR}/glm.h)

######
# Dependencies
######
set(GHOUL_DEPENDENCIES ${GHOUL_DEPENDENCIES} ${DEVIL_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})

include_directories(
    ${GHOUL_ROOT_DIR}/include
    ${GHOUL_ROOT_DIR}/include/ghoul)
    
if (CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")
    if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    else ()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif ()
    # C4201: nonstandard extension used : nameless struct/union    # Extension is ubiquitous
    # C4127: 'if' or 'while' loop evaluates to a constant          # Used for 'swallowing the semicolon" idiom
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4201 /wd4127")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    if(UNIX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-long-long -pedantic -Wextra")
    endif(UNIX)
endif ()
    
add_library(Ghoul ${GHOUL_SOURCE} ${GHOUL_HEADER})
target_link_libraries(Ghoul Lua ${GHOUL_DEPENDENCIES})