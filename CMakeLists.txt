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

cmake_minimum_required(VERSION 4.0)

# Ghoul is either built standalone or added to a superproject through add_subdirectory. In
# the latter case vcpkg ignores this manifest and the superproject has to provide the
# dependencies itself, which it can do by depending on the `ghoul-deps` port
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(GHOUL_IS_TOP_LEVEL ON)
else ()
  set(GHOUL_IS_TOP_LEVEL OFF)
endif ()

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
option(GHOUL_HAVE_TESTS "Activate the unit tests" ${GHOUL_IS_TOP_LEVEL})
option(GHOUL_ENABLE_INSTALL "Generate the install and export rules for Ghoul" ${GHOUL_IS_TOP_LEVEL})
option(TRACY_ENABLE "Enable profiling with Tracy" OFF)

option(BUILD_SHARED_LIBS "Build package with shared libraries" OFF)

# The options above are translated into vcpkg manifest features, which has to happen before
# the first project() call so that vcpkg installs the correct set of dependencies
if (GHOUL_IS_TOP_LEVEL)
  if (GHOUL_MODULE_ASSIMP)
    list(APPEND VCPKG_MANIFEST_FEATURES "assimp")
  endif ()
  if (GHOUL_MODULE_FONTRENDERING)
    list(APPEND VCPKG_MANIFEST_FEATURES "fontrendering")
  endif ()
  if (GHOUL_MODULE_LUA)
    list(APPEND VCPKG_MANIFEST_FEATURES "lua")
  endif ()
  if (GHOUL_MODULE_OPENGL)
    list(APPEND VCPKG_MANIFEST_FEATURES "opengl")
  endif ()
  if (TRACY_ENABLE)
    list(APPEND VCPKG_MANIFEST_FEATURES "profiling")
  endif ()
  if (GHOUL_HAVE_TESTS)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
  endif ()
endif ()

project(Ghoul VERSION 0.1.0 LANGUAGES C CXX)

# Setting default paths
set(GHOUL_ROOT_DIR ${PROJECT_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${GHOUL_ROOT_DIR}/support/cmake)

include(${GHOUL_ROOT_DIR}/support/cmake/message_macros.cmake)

begin_header("Configuring Ghoul Project")

if (WIN32)
  option(GHOUL_ENABLE_EDIT_CONTINUE "Enable Edit&Continue" ON)

  if (TRACY_ENABLE)
    message(WARNING "Tracy does not support compiling with Edit&Continue, so we have to disable that feature")
    set(GHOUL_ENABLE_EDIT_CONTINUE OFF CACHE BOOL "" FORCE)
  endif ()
endif ()


add_subdirectory(src)
add_subdirectory(ext SYSTEM)

if (GHOUL_ENABLE_INSTALL)
  include(${GHOUL_ROOT_DIR}/support/cmake/ghoul_install.cmake)
endif ()

# Other applications
if (GHOUL_HAVE_TESTS)
  begin_header("Generating unit test")
  enable_testing()
  add_subdirectory(tests)
  end_header()
endif ()

end_header("End: Configuring Ghoul Project")
