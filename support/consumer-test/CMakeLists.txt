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

# Minimal stand-in for a third-party project. It consumes an *installed* Ghoul the same way
# an external developer would and therefore catches breakage in the install and export
# rules that building Ghoul in-tree cannot catch.
#
# Install Ghoul in every configuration you want to consume, then configure this project
# against that prefix:
#   cmake --build build/windows --config Debug
#   cmake --install build/windows --config Debug
#   cmake -S support/consumer-test -B build/consumer-test
#     --toolchain $env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
#     -DVCPKG_TARGET_TRIPLET=x64-windows-static-md
#     -DCMAKE_CONFIGURATION_TYPES="Debug;Release"
#     -DCMAKE_PREFIX_PATH=<ghoul-install-prefix>
#
# CMAKE_CONFIGURATION_TYPES has to be restricted to the configurations that were actually
# installed, because a multi-config generator otherwise asks the imported target for
# MinSizeRel and RelWithDebInfo as well.

cmake_minimum_required(VERSION 3.25)
project(GhoulConsumerTest LANGUAGES CXX)

find_package(Ghoul CONFIG REQUIRED)

add_executable(GhoulConsumerTest main.cpp)
target_compile_features(GhoulConsumerTest PRIVATE cxx_std_20)
target_link_libraries(GhoulConsumerTest PRIVATE Ghoul::Ghoul)

enable_testing()
add_test(NAME GhoulConsumerTest COMMAND GhoulConsumerTest)
