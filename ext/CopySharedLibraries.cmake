#########################################################################################
#                                                                                       #
# GHOUL                                                                                 #
#                                                                                       #
# Copyright (c) 2012-2015                                                               #
#                                                                                       #
# Permission is hereby granted, free of charge, to any person obtaining a copy of this  #
# software and associated documentation files (the "Software"), to deal in the Software #
# without restriction, including without limitation the rights to use, copy, modify,    #
# merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    #
# permit persons to whom the Software is furnished to do so, subject to the following   #
# conditions:                                                                           #
#                                                                                       #
# The above copyright notice and this permission notice shall be included in all copies #
# or substantial portions of the Software.                                              #
#                                                                                       #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   #
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         #
# PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    #
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  #
# CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  #
# OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         #
#########################################################################################

function (copy_files target)
    # Add the copy command
    foreach(file_i ${ARGN})
        add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${file_i}"
        $<TARGET_FILE_DIR:${target}>)
    endforeach ()
endfunction ()

macro (ghl_copy_shared_libraries target ghoul_dir)
    # Windows DLL
    if (WIN32)
        # DevIL
        if (GHOUL_USE_DEVIL)
            if (CMAKE_CL_64)
                set(DEVIL_DLL_DIR ${ghoul_dir}/ext/il/lib/win64)
            else (CMAKE_CL_64)
                set(DEVIL_DLL_DIR ${ghoul_dir}/ext/il/lib/win32)
            endif ()
            set(SHARED_LIBS ${SHARED_LIBS} ${DEVIL_DLL_DIR}/DevIL.dll)
            set(SHARED_LIBS ${SHARED_LIBS} ${DEVIL_DLL_DIR}/ILU.dll)
            set(SHARED_LIBS ${SHARED_LIBS} ${DEVIL_DLL_DIR}/ILUT.dll)
        endif ()

        # FreeImage
        if(GHOUL_USE_FREEIMAGE)
            if (CMAKE_CL_64)
                set(FREEIMAGE_DLL_DIR ${ghoul_dir}/ext/freeimage/lib/win64)
            else (CMAKE_CL_64)
                set(FREEIMAGE_DLL_DIR ${ghoul_dir}/ext/freeimage/lib/win32)
            endif ()
            set(SHARED_LIBS ${SHARED_LIBS} ${FREEIMAGE_DLL_DIR}/FreeImage.dll)
        endif ()

        copy_files(${target} ${SHARED_LIBS})
    endif (WIN32)
endmacro ()
