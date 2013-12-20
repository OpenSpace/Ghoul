cmake_minimum_required(VERSION 2.8.0)
project(Ghoul)

#SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows")
#SET(CMAKE_RELEASE_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows")
#SET(CMAKE_RELWITHDEBINFO_POSTFIX "rd" CACHE STRING "add a postfix, usually empty on windows")
#SET(CMAKE_MINSIZEREL_POSTFIX "s" CACHE STRING "add a postfix, usually empty on windows")
#${CMAKE_CONFIGURATION_TYPES}
#        SET(OSG_AGGRESSIVE_WARNING_FLAGS /W4 /wd4706 /wd4127 /wd4100)
#    IF(OSG_USE_AGGRESSIVE_WARNINGS)
#        # Add flags defined by OSG_AGGRESSIVE_WARNING_FLAGS if they aren't already there
#        FOREACH(flag ${OSG_AGGRESSIVE_WARNING_FLAGS})
#            IF(NOT CMAKE_CXX_FLAGS MATCHES "${flag}")
#                SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
#            ENDIF()
#        ENDFOREACH()
#    ELSE()
#        # Remove all flags considered aggresive
#        FOREACH(flag ${OSG_AGGRESSIVE_WARNING_FLAGS})
#            STRING(REGEX REPLACE "${flag}" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
#        ENDFOREACH()
#    ENDIF()

mark_as_advanced(CMAKE_BACKWARDS_COMPATIBILITY)

if (POLICY CMP0011)
   cmake_policy(SET CMP0011 OLD) # or even better, NEW
endif (POLICY CMP0011)

if (NOT GHOUL_ROOT_DIR)
    set(GHOUL_ROOT_DIR ${PROJECT_SOURCE_DIR})
endif ()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/ext)

if (APPLE)
	add_definitions(-D__APPLE__)
	set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ ${CMAKE_CXX_FLAGS}")
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -ggdb")
endif ()

######
# Options
######

if (WIN32)
    option(GHOUL_USE_WMI "Use Windows Management Instrumentation" ON)
    if (GHOUL_USE_WMI)
        add_definitions(-DGHOUL_USE_WMI -D_WIN32_DCOM)
        set(GHOUL_DEPENDENCIES ${GHOUL_DEPENDENCIES} "WbemUuid.lib")
    endif ()
endif ()

######
# General dependencies
######
set(GLM_ROOT_DIR "${GHOUL_ROOT_DIR}/ext/glm")
find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})
add_definitions(-DGLM_SWIZZLE)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

option(GHOUL_USE_DEVIL "Use the DevIL library" ON)
if (GHOUL_USE_DEVIL)
    add_definitions(-DGHOUL_USE_DEVIL)
    find_package(DevIL)
    include_directories(${IL_INCLUDE_DIR})
    set(DEVIL_LIBRARIES ${IL_LIBRARIES} ${ILU_LIBRARIES} ${ILUT_LIBRARIES})
endif ()

#####
# Platform-dependent Settings
#####
if (APPLE)
	add_definitions(-D__gl_h_)
#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-gnu")
endif ()

add_subdirectory(src)
