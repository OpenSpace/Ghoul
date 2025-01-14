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

cmake_minimum_required(VERSION 3.0)
project(Lua)

set(LUA_ROOT_DIR ${PROJECT_SOURCE_DIR})

# LUA_USE_APICHECK  in full debug modes

file(GLOB LUA_SOURCE ${LUA_ROOT_DIR}/src/*.c)
file(GLOB LUA_HEADER ${LUA_ROOT_DIR}/src/*.h)

list(
  REMOVE_ITEM LUA_SOURCE
  "${LUA_ROOT_DIR}/src/lua.c"
  "${LUA_ROOT_DIR}/src/luac.c"
  "${LUA_ROOT_DIR}/src/onelua.c"
)

add_library(Lua STATIC ${LUA_SOURCE})
target_compile_features(Lua PUBLIC cxx_std_20)

if (WIN32)
  target_compile_definitions(Lua PRIVATE "_CRT_SECURE_NO_WARNINGS")
  set_target_properties(Lua PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
elseif (UNIX)
  target_compile_definitions(Lua PRIVATE "LUA_USE_POSIX")
endif()

target_include_directories(Lua PUBLIC "${LUA_ROOT_DIR}/src")
