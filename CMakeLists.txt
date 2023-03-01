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

cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
cmake_policy(VERSION 3.25)
project(Ghoul)

# Setting default paths
set(GHOUL_ROOT_DIR ${PROJECT_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${GHOUL_ROOT_DIR}/support/cmake)

include(${GHOUL_ROOT_DIR}/support/cmake/message_macros.cmake)

begin_header("Configuring Ghoul Project")

#############################
# Options
#############################
option(GHOUL_HIGH_DEBUG_MODE "Add additional debugging code" ON)
option(GHOUL_LOGGING_ENABLE_TRACE "Enables the LTRACE macro" ON)
option(GHOUL_THROW_ON_ASSERT "Disables the feedback on asserts; for use in unit tests" OFF)

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


if (WIN32)
  option(GHOUL_ENABLE_EDIT_CONTINUE "Enable Edit&Continue" ON)

  if (TRACY_ENABLE)
    message(WARNING "Tracy does not support compiling with Edit&Continue, so we have to disable that feature")
    set(GHOUL_ENABLE_EDIT_CONTINUE OFF CACHE BOOL "" FORCE)
  endif ()
endif ()


add_subdirectory(src)
add_subdirectory(ext SYSTEM)

# Other applications
option(GHOUL_HAVE_TESTS "Activate the unit tests" ON)
if (GHOUL_HAVE_TESTS)
  begin_header("Generating unit test")
  add_subdirectory(tests)
  end_header()
endif ()

end_header("End: Configuring Ghoul Project")
