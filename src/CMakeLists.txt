##########################################################################################
#                                                                                        #
# GHOUL                                                                                  #
# General Helpful Open Utility Library                                                   #
#                                                                                        #
# Copyright (c) 2012-2014                                                                #
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


######
# Macros
######

# source group macro
macro(add_source_group GROUP_NAME GROUP_DIRECTORY)
    file(GLOB SOURCE    ${SOURCE_ROOT_DIR}/${GROUP_DIRECTORY}/*.cpp 
                        ${SOURCE_ROOT_DIR}/${GROUP_DIRECTORY}/*.c)
    file(GLOB HEADER    ${HEADER_ROOT_DIR}/${GROUP_DIRECTORY}/*.h
                        ${HEADER_ROOT_DIR}/${GROUP_DIRECTORY}/*.hpp
                        ${HEADER_ROOT_DIR}/${GROUP_DIRECTORY}/*.inl
                        ${HEADER_ROOT_DIR}/${GROUP_DIRECTORY}/${GROUP_DIRECTORY})

    set(GHOUL_SOURCE ${GHOUL_SOURCE} ${SOURCE})
    set(GHOUL_HEADER ${GHOUL_HEADER} ${HEADER} ${GLOB_HEADER})
    source_group(${GROUP_NAME} FILES
                    ${SOURCE}
                    ${HEADER}
                    ${GLOB_HEADER})
endmacro(add_source_group)

######
# Source
######

set(SOURCE_ROOT_DIR ${GHOUL_ROOT_DIR}/src)
set(HEADER_ROOT_DIR ${GHOUL_ROOT_DIR}/include/ghoul)

add_source_group("CmdParser" "cmdparser")
add_source_group("FileSystem" "filesystem")
add_source_group("IO" "io")

file(GLOB IO_TEXTURE_SOURCE ${SOURCE_ROOT_DIR}/io/texture/*.cpp)
set(GHOUL_SOURCE ${GHOUL_SOURCE} ${IO_TEXTURE_SOURCE})
file(GLOB IO_TEXTURE_HEADER ${HEADER_ROOT_DIR}/io/texture/*.h)
set(GHOUL_HEADER ${GHOUL_HEADER} ${IO_TEXTURE_HEADER})
source_group(IO\\Texture FILES ${IO_TEXTURE_SOURCE} ${IO_TEXTURE_HEADER})
             
add_source_group("LogManager" "logging") 
add_source_group("Lua" "lua") 
add_source_group("Misc" "misc") 
add_source_group("OpenGL" "opengl") 
add_source_group("SystemCapabilities" "systemcapabilities") 

set(GHOUL_HEADER ${GHOUL_HEADER} ${HEADER_ROOT_DIR}/glm.h)

######
# Dependencies
######
set(GHOUL_DEPENDENCIES ${GHOUL_DEPENDENCIES} ${DEVIL_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${MAC_LIBS})

include_directories(
    ${GHOUL_ROOT_DIR}/include
    ${GHOUL_ROOT_DIR}/include/ghoul)
    
if (CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake|MSBuild)")
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

# Create the Ghoul library
add_library(Ghoul STATIC ${GHOUL_SOURCE} ${GHOUL_HEADER})

# Adding debug flags for the debug build
if(CMAKE_BUILD_TYPE MATCHES Debug)
    if(MSVC OR XCODE)
        if(${CMAKE_VERSION} VERSION_GREATER 3.0 OR ${CMAKE_VERSION} VERSION_EQUAL 3.0)
            target_compile_definitions(Ghoul PRIVATE $<$<CONFIG:Debug>:GHL_DEBUG>)
        else()
            set_target_properties(Ghoul PROPERTIES COMPILE_DEFINITIONS_DEBUG "GHL_DEBUG")
        endif()
    else() 
        add_definitions(-DGHL_DEBUG)
        add_definitions(-D_DEBUG)
    endif(MSVC OR XCODE)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

target_link_libraries(Ghoul Lua ${GHOUL_DEPENDENCIES})
