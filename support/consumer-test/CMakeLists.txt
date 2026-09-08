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

# Minimal stand-in for a third-party project. It consumes Ghoul the same way an external
# developer would - through the `ghoul` overlay port and `find_package` - and therefore
# catches breakage in the port, the install rules, and the exported target that building
# Ghoul in-tree cannot catch.
#
# The manifest here depends on the `ghoul` port, which builds and installs Ghoul into
# vcpkg_installed; configure this project with the vcpkg toolchain and it resolves:
#   cmake -S support/consumer-test -B build/consumer-test
#     --toolchain $env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
#     -DVCPKG_TARGET_TRIPLET=x64-windows-static-md
#   cmake --build build/consumer-test
#   ctest --test-dir build/consumer-test

cmake_minimum_required(VERSION 3.25)
project(GhoulConsumerTest LANGUAGES CXX)

find_package(ghoul CONFIG REQUIRED)

add_executable(GhoulConsumerTest main.cpp)
target_compile_features(GhoulConsumerTest PRIVATE cxx_std_20)
target_link_libraries(GhoulConsumerTest PRIVATE Ghoul::Ghoul)

enable_testing()
add_test(NAME GhoulConsumerTest COMMAND GhoulConsumerTest)
