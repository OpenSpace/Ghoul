##########################################################################################
#                                                                                        #
# GHOUL                                                                                  #
#                                                                                        #
# Copyright (c) 2012-2020                                                                #
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

function(disable_external_warnings library_name)
  if (MSVC)
    target_compile_options(${library_name} PRIVATE "/W0")
    target_compile_definitions(${library_name} PRIVATE "_SCL_SECURE_NO_WARNINGS")
  else ()
    target_compile_options(${library_name} PRIVATE "-w")
  endif ()
endfunction ()


function(disable_external_warnings_for_file file_name)
  if (MSVC)
    set_source_files_properties(${file_name} PROPERTIES COMPILE_FLAGS "/W0")

    set_source_files_properties(
      ${file_name}
      PROPERTIES COMPILE_DEFINITIONS "_SCL_SECURE_NO_WARNINGS"
    )
  else ()
    set_source_files_properties(${file_name} PROPERTIES COMPILE_FLAGS "-w")
  endif ()
endfunction ()

function (set_folder_location target folder)
  if (TARGET ${target})
    set_property(TARGET ${target} PROPERTY FOLDER ${folder})
  endif ()
endfunction ()

# Includes an external library by adding its subdirectory using 'add_subdirectory'
# target_name: Target to which the library is added
# library_name: The library that is added by including 'path'
# path: The path that will be included
function (include_external_library target_name visibility library_name path)
  set (extra_macro_args ${ARGN})
  if (NOT TARGET ${library_name})
    add_subdirectory(${path})
    get_property(INCLUDE_DIR TARGET ${target_name} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
    target_link_libraries(${target_name} ${visibility} ${library_name})
    set_folder_location(${library_name} "External")
    if (MSVC)
      target_compile_options(${library_name} PUBLIC "/MP" "/bigobj")
    endif ()
    if (GHOUL_DISABLE_EXTERNAL_WARNINGS)
      disable_external_warnings(${library_name})
    endif ()
  endif ()
endfunction ()
