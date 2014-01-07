# This module locates the developer's image library.
# http://openil.sourceforge.net/
#
# This module sets:
#   IL_LIBRARIES -   the name of the IL library. These include the full path to
#                    the core DevIL library. This one has to be linked into the
#                    application.
#   ILU_LIBRARIES -  the name of the ILU library. Again, the full path. This
#                    library is for filters and effects, not actual loading. It
#                    doesn't have to be linked if the functionality it provides
#                    is not used.
#   ILUT_LIBRARIES - the name of the ILUT library. Full path. This part of the
#                    library interfaces with OpenGL. It is not strictly needed
#                    in applications.
#   IL_INCLUDE_DIR - where to find the il.h, ilu.h and ilut.h files.
#   IL_FOUND -       this is set to TRUE if all the above variables were set.
#                    This will be set to false if ILU or ILUT are not found,
#                    even if they are not needed. In most systems, if one
#                    library is found all the others are as well. That's the
#                    way the DevIL developers release it.

#=============================================================================
# Copyright 2008-2009 Kitware, Inc.
# Copyright 2008 Christopher Harvey
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# TODO: Add version support.
# Tested under Linux and Windows (MSVC)

find_path(IL_INCLUDE_DIR IL/il.h
  HINTS ${CMAKE_MODULE_PATH}/il/include
  DOC "The path the the directory that contains il.h"
)

if (WIN32)
    if (CMAKE_CL_64)
        find_library(IL_LIBRARIES
        NAMES IL DEVIL
        PATHS ${CMAKE_MODULE_PATH}/il
        PATH_SUFFIXES lib64 lib lib/win64
        DOC "The file that corresponds to the base il library."
        )
        
        find_library(ILU_LIBRARIES
        NAMES ILU
        PATHS ${CMAKE_MODULE_PATH}/il
        PATH_SUFFIXES lib64 lib lib/win64
        DOC "The file that corresponds to the il utility library."
        )

        find_library(ILUT_LIBRARIES
        NAMES ILUT
        PATHS ${CMAKE_MODULE_PATH}/il
        PATH_SUFFIXES lib64 lib lib/win64
        DOC "The file that corresponds to the il (system?) utility library."
        )
    else ()
        find_library(IL_LIBRARIES
        NAMES IL DEVIL
        PATHS ${CMAKE_MODULE_PATH}/il
        PATH_SUFFIXES lib lib/win32
        DOC "The file that corresponds to the base il library."
        )
        
        find_library(ILU_LIBRARIES
        NAMES ILU
        PATHS ${CMAKE_MODULE_PATH}/il
        PATH_SUFFIXES lib lib32 lib/win32
        DOC "The file that corresponds to the il utility library."
        )

        find_library(ILUT_LIBRARIES
        NAMES ILUT
        PATHS ${CMAKE_MODULE_PATH}/il
        PATH_SUFFIXES lib lib32 lib/win32
        DOC "The file that corresponds to the il (system?) utility library."
        )
    endif ()
else ()
    find_library(ILU_LIBRARIES
    NAMES ILU
    PATHS ${CMAKE_MODULE_PATH}/il
    PATH_SUFFIXES lib64 lib lib32 lib/win32 lib/win64
    DOC "The file that corresponds to the il utility library."
    )

    find_library(IL_LIBRARIES
    NAMES IL DEVIL
    PATHS ${CMAKE_MODULE_PATH}/il
    PATH_SUFFIXES lib64 lib lib32 lib/win32 lib/win64
    DOC "The file that corresponds to the base il library."
    )

    find_library(ILUT_LIBRARIES
    NAMES ILUT
    PATHS ${CMAKE_MODULE_PATH}/il
    PATH_SUFFIXES lib64 lib lib32 lib/win32 lib/win64
    DOC "The file that corresponds to the il (system?) utility library."
    )
endif ()

# message("IL_LIBRARIES is ${IL_LIBRARIES}")
# message("ILUT_LIBRARIES is ${ILUT_LIBRARIES}")
# message("ILU_LIBRARIES is ${ILU_LIBRARIES}")

FIND_PACKAGE_HANDLE_STANDARD_ARGS(IL DEFAULT_MSG
                                  IL_LIBRARIES ILU_LIBRARIES
                                  ILUT_LIBRARIES IL_INCLUDE_DIR)
