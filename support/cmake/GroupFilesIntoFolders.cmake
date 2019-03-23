##########################################################################################
#                                                                                        #
# GHOUL                                                                                  #
#                                                                                        #
# Copyright (c) 2012-2019                                                                #
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

function (group_files_into_folders)
  # Place files into source_group
  foreach (file ${ARGN})
    # Remove prefixes from the files 
    set(original_file ${file})
    string(REPLACE "${PROJECT_SOURCE_DIR}/src/" "" file ${file})
    string(REPLACE "${PROJECT_SOURCE_DIR}/include/ghoul/" "" file ${file})

    # Extract the directory part of the rest
    get_filename_component(directory ${file} DIRECTORY)
    if (NOT ${directory} STREQUAL "")
      if (MSVC)
        string(REPLACE "/" "\\" directory ${directory})
      endif ()

      # Make the directory's first letter upper case
      string(SUBSTRING ${directory} 0 1 FIRST_LETTER)
      string(TOUPPER ${FIRST_LETTER} FIRST_LETTER)
      string(REGEX REPLACE "^.(.*)" "${FIRST_LETTER}\\1" directory "${directory}")
      source_group("${directory}" FILES ${original_file})
    else ()
      source_group("" FILES ${original_file})
    endif ()
  endforeach ()
endfunction ()
