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

function (set_folder_location target folder)
  if (TARGET ${target})
    set_target_properties(${target} PROPERTIES FOLDER ${folder})
  endif ()
endfunction ()

# Includes an external library by adding its subdirectory using 'add_subdirectory'
# target_name: Target to which the library is added
# library_name: The library that is added by including 'path'
# path: The path that will be included
function (include_external_library target_name visibility library_name path)
  set (extra_macro_args ${ARGN})
  if (NOT TARGET ${library_name})
    add_subdirectory(${path} SYSTEM)
    target_link_libraries(${target_name} ${visibility} ${library_name})
    set_folder_location(${library_name} "External")
    if (MSVC)
      target_compile_options(${library_name} PRIVATE "/MP" "/bigobj")
    else ()
      target_compile_options(${library_name} PRIVATE "-w")
    endif ()
  endif ()
endfunction ()
