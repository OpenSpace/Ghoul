#
# Try to find the FreeImage library and include path.
# Once done this will define
#
# FREEIMAGE_FOUND
# FREEIMAGE_INCLUDE_PATH
# FREEIMAGE_LIBRARY
# 
INCLUDE(FindPackageHandleStandardArgs)

IF (WIN32)
    if (CMAKE_CL_64)
		FIND_PATH( FREEIMAGE_INCLUDE_PATH FreeImage.h
			HINTS ${CMAKE_MODULE_PATH}/freeimage/include
			DOC "The directory where FreeImage.h resides")
		FIND_LIBRARY( FREEIMAGE_LIBRARY
			NAMES FreeImage freeimage
			PATHS
			HINTS ${CMAKE_MODULE_PATH}/freeimage/lib/win64
			PATH_SUFFIXES lib 
			DOC "The FreeImage library")
    else ()
		FIND_PATH( FREEIMAGE_INCLUDE_PATH FreeImage.h
			HINTS ${CMAKE_MODULE_PATH}/freeimage/include
			DOC "The directory where FreeImage.h resides")
		FIND_LIBRARY( FREEIMAGE_LIBRARY
			NAMES FreeImage freeimage
			PATHS
			HINTS ${CMAKE_MODULE_PATH}/freeimage/lib/win32
			PATH_SUFFIXES lib 
			DOC "The FreeImage library")
    endif ()
ELSE (WIN32)
	FIND_PATH( FREEIMAGE_INCLUDE_PATH FreeImage.h
		/usr/include
		/usr/local/include
		/sw/include
		/opt/local/include
		DOC "The directory where FreeImage.h resides")
	FIND_LIBRARY( FREEIMAGE_LIBRARY
		NAMES FreeImage freeimage
		PATHS
		/usr/lib64
		/usr/lib
		/usr/local/lib64
		/usr/local/lib
		/sw/lib
		/opt/local/lib
		DOC "The FreeImage library")
ENDIF (WIN32)

SET(FREEIMAGE_LIBRARIES ${FREEIMAGE_LIBRARY})
SET(FREEIMAGE_INCLUDE_DIR ${FREEIMAGE_INCLUDE_PATH})

find_package_handle_standard_args(FREEIMAGE
                                  REQUIRED_VARS FREEIMAGE_INCLUDE_DIR FREEIMAGE_LIBRARIES)

MARK_AS_ADVANCED(
	FREEIMAGE_FOUND 
	FREEIMAGE_LIBRARY
	FREEIMAGE_LIBRARIES
	FREEIMAGE_INCLUDE_PATH)

