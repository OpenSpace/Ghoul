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

add_executable(GhoulTest)
set_ghoul_compile_settings(GhoulTest)
target_sources(
  GhoulTest
  PRIVATE
    ${GHOUL_ROOT_DIR}/tests/main.cpp
    ${GHOUL_ROOT_DIR}/tests/test_commandlineparser.cpp
    ${GHOUL_ROOT_DIR}/tests/test_crc32.cpp
    ${GHOUL_ROOT_DIR}/tests/test_csvreader.cpp
    ${GHOUL_ROOT_DIR}/tests/test_dictionary.cpp
    ${GHOUL_ROOT_DIR}/tests/test_dictionaryjsonformatter.cpp
    ${GHOUL_ROOT_DIR}/tests/test_dictionaryluaformatter.cpp
    ${GHOUL_ROOT_DIR}/tests/test_filesystem.cpp
    ${GHOUL_ROOT_DIR}/tests/test_luaconversions.cpp
    ${GHOUL_ROOT_DIR}/tests/test_luatodictionary.cpp
    ${GHOUL_ROOT_DIR}/tests/test_memorypool.cpp
    ${GHOUL_ROOT_DIR}/tests/test_templatefactory.cpp
)

target_compile_definitions(GhoulTest PRIVATE
  # Jenkins shouldn't ask for asserts when they happen, but just throw
  "GHL_THROW_ON_ASSERT"
  "GHOUL_HAVE_TESTS"
  "GHOUL_ROOT_DIR=\"${GHOUL_ROOT_DIR}\""
)

if (MSVC)
  target_compile_options(GhoulTest PRIVATE "-Od" "/bigobj")
else ()
  target_compile_options(GhoulTest PRIVATE "-O0" )
endif ()

target_link_libraries(GhoulTest PRIVATE Catch2 Ghoul)

target_precompile_headers(GhoulTest PRIVATE
  <catch2/catch_test_macros.hpp>
)

if (APPLE)
  target_link_libraries(GhoulTest PRIVATE ${CARBON_LIBRARY} ${COREFOUNDATION_LIBRARY} ${COCOA_LIBRARY} ${APP_SERVICES_LIBRARY})
endif ()
