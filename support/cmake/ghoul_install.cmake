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

# Install and export rules that make Ghoul consumable through
# `find_package(ghoul CONFIG REQUIRED)` and `target_link_libraries(... Ghoul::Ghoul)`

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(GHOUL_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/ghoul")

install(
  TARGETS Ghoul
  EXPORT GhoulTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Ghoul links compile_settings privately, but a static library keeps its private
# dependencies in the link interface, so the target has to be part of the export set
if (TARGET compile_settings)
  install(TARGETS compile_settings EXPORT GhoulTargets)
endif ()

install(
  DIRECTORY ${GHOUL_ROOT_DIR}/include/ghoul
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# ghoul/opengl/renderdoc.h includes this vendored header
install(
  FILES ${GHOUL_ROOT_DIR}/ext/renderdoc/renderdoc_app.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  EXPORT GhoulTargets
  FILE GhoulTargets.cmake
  NAMESPACE Ghoul::
  DESTINATION ${GHOUL_INSTALL_CMAKEDIR}
)

configure_package_config_file(
  ${GHOUL_ROOT_DIR}/support/cmake/ghoulConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/ghoulConfig.cmake
  INSTALL_DESTINATION ${GHOUL_INSTALL_CMAKEDIR}
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/ghoulConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMinorVersion
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ghoulConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ghoulConfigVersion.cmake
  DESTINATION ${GHOUL_INSTALL_CMAKEDIR}
)
