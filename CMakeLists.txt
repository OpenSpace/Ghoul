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

# The options gating optional dependencies have to be declared before the first project()
# call: that is when vcpkg's toolchain file runs and reads VCPKG_MANIFEST_FEATURES.
#
# vcpkg only reads VCPKG_MANIFEST_FEATURES on the configure that first creates the build
# directory. Changing any of the options below afterwards will not add or remove the
# corresponding dependency: delete build/<preset>/vcpkg_installed (or the whole build
# directory) and configure again.

#############################
# Options
#############################
option(GHOUL_HIGH_DEBUG_MODE "Add additional debugging code" ON)
option(GHOUL_LOGGING_ENABLE_TRACE "Enables the LTRACE macro" ON)
option(GHOUL_THROW_ON_ASSERT "Disables the feedback on asserts; for use in unit tests" OFF)

option(TRACY_ENABLE "Enable profiling with Tracy" OFF)

# The tests pull in Catch2 and the install rules are of no use to a project that consumes
# Ghoul, so both default to on only when Ghoul is the top-level project. project() has not
# run yet, so PROJECT_IS_TOP_LEVEL is not available and the directory comparison stands in.
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  option(GHOUL_HAVE_TESTS "Activate the unit tests" ON)
  option(GHOUL_ENABLE_INSTALL "Generate the install and export rules for Ghoul" ON)
else ()
  option(GHOUL_HAVE_TESTS "Activate the unit tests" OFF)
  option(GHOUL_ENABLE_INSTALL "Generate the install and export rules for Ghoul" OFF)
endif ()

option(BUILD_SHARED_LIBS "Build package with shared libraries" OFF)

# The optional switches above are translated into vcpkg manifest features
if (TRACY_ENABLE)
  list(APPEND VCPKG_MANIFEST_FEATURES "profiling")
endif ()
if (GHOUL_HAVE_TESTS)
  list(APPEND VCPKG_MANIFEST_FEATURES "tests")
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
