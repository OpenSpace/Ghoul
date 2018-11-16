##########################################################################################
#                                                                                        #
# GHOUL                                                                                  #
#                                                                                        #
# Copyright (c) 2012-2018                                                                #
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

cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project(Ghoul)

# Setting default paths
set(GHOUL_ROOT_DIR ${PROJECT_SOURCE_DIR})
set(GHOUL_CMAKE_DIR ${GHOUL_ROOT_DIR}/support/cmake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${GHOUL_CMAKE_DIR})

include(${GHOUL_CMAKE_DIR}/handle_external_library.cmake)
include(${GHOUL_CMAKE_DIR}/set_ghoul_compile_settings.cmake)
include(${GHOUL_CMAKE_DIR}/message_macros.cmake)
include(${GHOUL_CMAKE_DIR}/include_gtest.cmake)

begin_header("Configuring Ghoul Project")

#############################
# Options
#############################
if (WIN32)
    option(GHOUL_USE_WMI "Use Windows Management Instrumentation" ON)
endif ()
option(GHOUL_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(GHOUL_HIGH_DEBUG_MODE "Add additional debugging code" ON)
option(GHOUL_LOGGING_ENABLE_TRACE "Enables the LTRACE macro" ON)
option(GHOUL_DISABLE_EXTERNAL_WARNINGS "Disable warnings in external libraries" ON)
option(GHOUL_THROW_ON_ASSERT "Disables the feedback on asserts; for use in unit tests" OFF)

option(GHOUL_MODULE_ASSIMP "Enable AssImp Model loader" ON)
option(GHOUL_MODULE_COMMANDLINEPARSER "Enable Commandlineparser" ON)
option(GHOUL_MODULE_FONTRENDERING "Enable Fontrendering" ON)
option(GHOUL_MODULE_LUA "Enable Lua" ON)
option(GHOUL_MODULE_OPENGL "Enable OpenGL" ON)
option(GHOUL_MODULE_SYSTEMCAPABILITIES "Enable System Capabilities" ON)

option(BUILD_SHARED_LIBS "Build package with shared libraries." OFF)

#############################
# Including the source files (with source groups already defined)
#############################
include(src/CMakeLists.txt)


#############################
# Declare the library
#############################
add_library(Ghoul STATIC ${GHOUL_SOURCE} ${GHOUL_HEADER})
target_include_directories(Ghoul PUBLIC ${PROJECT_SOURCE_DIR}/include)


#############################
# Compile settings
#############################
set_property(TARGET Ghoul PROPERTY CXX_STANDARD 17)
set_property(TARGET Ghoul PROPERTY CXX_STANDARD_REQUIRED On)

set_ghoul_compile_settings(Ghoul)

if (GHOUL_HIGH_DEBUG_MODE)
    target_compile_definitions(Ghoul PUBLIC "GHL_DEBUG")
endif ()

if (GHOUL_LOGGING_ENABLE_TRACE)
    target_compile_definitions(Ghoul PUBLIC "GHOUL_LOGGING_ENABLE_TRACE")
endif ()

if (GHOUL_THROW_ON_ASSERT)
    target_compile_definitions(Ghoul PUBLIC "GHL_THROW_ON_ASSERT")
endif ()

#############################
# Dependencies
#############################

# System Libraries
if (WIN32)
    begin_dependency("WMI")
    if (GHOUL_USE_WMI)
        target_compile_definitions(Ghoul PUBLIC "GHOUL_USE_WMI")
        target_link_libraries(Ghoul "WbemUuid.lib" "Ws2_32.lib")
    endif ()
    target_link_libraries(Ghoul "shlwapi")
    end_dependency()
endif ()

if (APPLE)
    begin_dependency("Core Libraries")
    find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
    find_library(CARBON_LIBRARY Carbon REQUIRED)
    find_library(COCOA_LIBRARY Carbon REQUIRED)
    find_library(APP_SERVICES_LIBRARY ApplicationServices REQUIRED)
    mark_as_advanced(COREFOUNDATION_LIBRARY COREVIDEO_LIBRARY CARBON_LIBRARY
        COCOA_LIBRARY APP_SERVICES_LIBRARY)
    target_include_directories(Ghoul SYSTEM PUBLIC "/Developer/Headers/FlatCarbon")
    target_link_libraries(Ghoul ${CARBON_LIBRARY} ${COREFOUNDATION_LIBRARY}
        ${COCOA_LIBRARY} ${APP_SERVICES_LIBRARY})
    end_dependency()
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    begin_dependency("iNotify")
    find_library(INOTIFY_LIBRARIES inotify PATHS "/usr/local/lib")
    mark_as_advanced(INOTIFY_LIBRARIES)
    target_include_directories(Ghoul SYSTEM PUBLIC "/usr/local/include")
    target_link_libraries(Ghoul ${INOTIFY_LIBRARIES})
    end_dependency("iNotify")
endif ()

# GLM
begin_dependency("GLM")
set(GLM_ROOT_DIR "${GHOUL_ROOT_DIR}/ext/glm")
find_package(GLM REQUIRED)
message(STATUS "Including GLM found at ${GLM_INCLUDE_DIRS}")
mark_as_advanced(GLM_INCLUDE_DIR GLM_INCLUDE_DIRS)
target_include_directories(Ghoul SYSTEM PUBLIC ${GLM_INCLUDE_DIRS})
end_dependency("GLM")

if (GHOUL_MODULE_OPENGL)
    begin_module("OpenGL")

    # OpenGL
    begin_dependency("OpenGL")
    find_package(OpenGL REQUIRED)
    target_include_directories(Ghoul SYSTEM PUBLIC ${OPENGL_INCLUDE_DIR})
    message(STATUS "Including OpenGL found at ${OPENGL_INCLUDE_DIR}")
    target_link_libraries(Ghoul ${OPENGL_LIBRARIES})
    message(STATUS "Linking OpenGL found at ${OPENGL_LIBRARIES}")

    mark_as_advanced(OPENGL_INCLUDE_DIR OPENGL_LIBRARIES)
    end_dependency("OpenGL")

    # glbindings
    begin_dependency("glbindings")
    target_compile_definitions(Ghoul PUBLIC "GHOUL_MODULE_OPENGL")

    set(OPTION_BUILD_GPU_TESTS OFF CACHE BOOL "")
    set(OPTION_BUILD_TESTS OFF CACHE BOOL "")
    set(OPTION_BUILD_TOOLS OFF CACHE BOOL "")
    set(OPTION_BUILD_EXAMPLES OFF CACHE BOOL "")
    mark_as_advanced(OPTION_BUILD_GPU_TESTS OPTION_BUILD_TESTS OPTION_BUILD_TOOLS
        OPTION_BUILD_EXAMPLES OPTION_BUILD_CHECK OPTION_BUILD_DOCS
        OPTION_BUILD_WITH_BOOST_THREAD INSTALL_MSVC_REDIST_FILEPATH)
    add_subdirectory(${GHOUL_ROOT_DIR}/ext/glbinding)
    target_include_directories(Ghoul SYSTEM PUBLIC
        ${GHOUL_ROOT_DIR}/ext/glbinding/source/glbinding/include)
    target_include_directories(Ghoul SYSTEM PUBLIC
        ${GHOUL_ROOT_DIR}/ext/glbinding/source/glbinding-aux/include)
    target_include_directories(Ghoul SYSTEM PUBLIC
        ${CMAKE_BINARY_DIR}/ext/ghoul/ext/glbinding/source/glbinding-aux/include)
    target_link_libraries(Ghoul glbinding::glbinding glbinding::glbinding-aux)

    set_folder_location(glbinding "External")
    set_folder_location(check-glbinding "External/glbinding-ext")
    set_folder_location(check-glbinding-aux "External/glbinding-ext")
    set_folder_location(glbinding-aux "External/glbinding-ext")
    set_folder_location(component_install "External/glbinding-ext")
    set_folder_location(generate "External/glbinding-ext")
    set_folder_location(pack "External/glbinding-ext")
    set_folder_location(pack-glbinding "External/glbinding-ext")
    set_folder_location(update "External/glbinding-ext")
    set_folder_location(check-all "External/glbinding-ext")
    set_folder_location(check-template "External/glbinding-ext")
    set_folder_location(cppcheck-glbinding "External/glbinding-ext")

    end_dependency("glbindings")
    end_module("OpenGL")
endif ()

# LZ4
begin_dependency("LZ4")
find_package(LZ4 REQUIRED)
target_include_directories(Ghoul SYSTEM PUBLIC ${LZ4_INCLUDE_DIR})
target_link_libraries(Ghoul ${LZ4_LIBRARIES})

mark_as_advanced(LZ4_INCLUDE_DIR)
set_folder_location(lz4 "External")
end_dependency("LZ4")

# Lua
if (GHOUL_MODULE_LUA)
    begin_module("Lua")
    include_external_library(Ghoul Lua ${GHOUL_ROOT_DIR}/ext/lua)
    end_module()
endif ()

# Assimp
if (GHOUL_MODULE_ASSIMP)
    begin_module("AssImp")
    find_package(Assimp QUIET)
    if (Assimp_FOUND)
        message(STATUS "Using system assimp")
        target_include_directories(Ghoul SYSTEM PUBLIC ${ASSIMP_INCLUDE_DIRS})
        target_link_libraries(Ghoul ${ASSIMP_LIBRARIES})
        link_directories("${ASSIMP_LIBRARY_DIRS}")
        set(GHOUL_LIBRARY_DIRS "${ASSIMP_LIBRARY_DIRS}" PARENT_SCOPE)
    else ()
        message(STATUS "Building assimp from source")
        set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)

        # These importers produce errors due to missing entries in the config.h right now
        # When they compile, there is no reason to keep them disabled
        set(ASSIMP_BUILD_FBX_IMPORTER OFF CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_IFC_IMPORTER OFF CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_X3D_IMPORTER OFF CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_X_IMPORTER OFF CACHE BOOL "")
        set(ASSIMP_NO_EXPORTER ON CACHE BOOL "")
        set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")

        mark_as_advanced(Assimp_DIR ASSIMP_ANDROID_JNIIOSYSTEM ASSIMP_ASAN
            ASSIMP_BIN_INSTALL_DIR ASSIMP_BUILD_3DS_IMPORTER ASSIMP_BUILD_3D_IMPORTER
            ASSIMP_BUILD_3MF_IMPORTER ASSIMP_BUILD_AC_IMPORTER
            ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT ASSIMP_BUILD_AMF_IMPORTER
            ASSIMP_BUILD_ASE_IMPORTER ASSIMP_BUILD_ASSBIN_IMPORTER
            ASSIMP_BUILD_ASSIMP_TOOLS ASSIMP_BUILD_ASSXML_IMPORTER
            ASSIMP_BUILD_B3D_IMPORTER ASSIMP_BUILD_BLEND_IMPORTER
            ASSIMP_BUILD_BVH_IMPORTER ASSIMP_BUILD_COB_IMPORTER
            ASSIMP_BUILD_COLLADA_IMPORTER ASSIMP_BUILD_CSM_IMPORTER
            ASSIMP_BUILD_DXF_IMPORTER ASSIMP_BUILD_FBX_IMPORTER ASSIMP_BUILD_GLTF_IMPORTER
            ASSIMP_BUILD_HMP_IMPORTER ASSIMP_BUILD_IFC_IMPORTER
            ASSIMP_BUILD_IRRMESH_IMPORTER ASSIMP_BUILD_IRR_IMPORTER
            ASSIMP_BUILD_LWO_IMPORTER ASSIMP_BUILD_LWS_IMPORTER ASSIMP_BUILD_MD2_IMPORTER
            ASSIMP_BUILD_MD3_IMPORTER ASSIMP_BUILD_MD5_IMPORTER ASSIMP_BUILD_MDC_IMPORTER
            ASSIMP_BUILD_MDL_IMPORTER ASSIMP_BUILD_MMD_IMPORTER ASSIMP_BUILD_MS3D_IMPORTER
            ASSIMP_BUILD_NDO_IMPORTER ASSIMP_BUILD_NFF_IMPORTER
            ASSIMP_BUILD_NONFREE_C4D_IMPORTER ASSIMP_BUILD_OBJ_IMPORTER
            ASSIMP_BUILD_OFF_IMPORTER ASSIMP_BUILD_OGRE_IMPORTER
            ASSIMP_BUILD_OPENGEX_IMPORTER ASSIMP_BUILD_PLY_IMPORTER
            ASSIMP_BUILD_Q3BSP_IMPORTER ASSIMP_BUILD_Q3D_IMPORTER
            ASSIMP_BUILD_RAW_IMPORTER ASSIMP_BUILD_SAMPLES ASSIMP_BUILD_SIB_IMPORTER
            ASSIMP_BUILD_SMD_IMPORTER ASSIMP_BUILD_STL_IMPORTER
            ASSIMP_BUILD_TERRAGEN_IMPORTER ASSIMP_BUILD_TESTS ASSIMP_BUILD_X_IMPORTER
            ASSIMP_BUILD_X3D_IMPORTER ASSIMP_BUILD_XGL_IMPORTER ASSIMP_BUILD_ZLIB
            ASSIMP_COVERALLS ASSIMP_DOUBLE_PRECISION ASSIMP_INCLUDE_INSTALL_DIR
            ASSIMP_INSTALL_PDB ASSIMP_LIBRARY_SUFFIX ASSIMP_LIB_INSTALL_DIR
            ASSIMP_NO_EXPORT ASSIMP_NO_EXPORTER ASSIMP_OPT_BUILD_PACKAGES
            ASSIMP_PACKAGE_VERSION ASSIMP_UBSAN ASSIMP_WERROR BUILD_DOCS BUILD_FRAMEWORK
            SYSTEM_IRRXML LIBRARY_SUFFIX)

        include_external_library(Ghoul
            assimp
            ${GHOUL_ROOT_DIR}/ext/assimp ${GHOUL_ROOT_DIR}/ext/assimp/include)

        # We need to include this to be able to find the assimp/config.h file
        target_include_directories(Ghoul PUBLIC
            "${CMAKE_CURRENT_BINARY_DIR}/ext/assimp/include")

        set_folder_location(IrrXML "External")
        set_folder_location(uninstall "External")
        set_folder_location(UpdateAssimpLibsDebugSymbolsAndDLLs "External")
    endif ()
    end_module("AssImp")
endif ()

# fmt
begin_dependency("fmt")
set(FMT_DOC OFF CACHE BOOL "Generate the doc target.")
set(FMT_INSTALL OFF CACHE BOOL "Generate the install target.")
set(FMT_TEST OFF CACHE BOOL "Generate the test target.")
include_external_library(Ghoul fmt
    ${GHOUL_ROOT_DIR}/ext/cppformat ${GHOUL_ROOT_DIR}/ext/cppformat)
mark_as_advanced(FMT_DOC FMT_INSTALL FMT_PEDANTIC FMT_TEST FMT_WERROR)
end_dependency("fmt")

message(STATUS "Loading image libraries")

# DevIL
find_package(DevIL QUIET)
if (IL_FOUND)
    begin_dependency("DevIL")
    option(GHOUL_USE_DEVIL "Use the DevIL library" ON)
    if (GHOUL_USE_DEVIL)
        target_compile_definitions(Ghoul PUBLIC "GHOUL_USE_DEVIL")

        target_include_directories(Ghoul SYSTEM PUBLIC ${IL_INCLUDE_DIR})
        target_link_libraries(Ghoul ${IL_LIBRARIES} ${ILU_LIBRARIES} ${ILUT_LIBRARIES})
    endif ()
    end_dependency()
endif ()
mark_as_advanced(IL_INCLUDE_DIR IL_LIBRARIES ILU_LIBRARIES ILUT_LIBRARIES)

# FreeImage
find_package(FreeImage QUIET)
if (FREEIMAGE_FOUND)
    begin_dependency("FreeImage")
    option(GHOUL_USE_FREEIMAGE "Use the FreeImage library" ON)
    if (GHOUL_USE_FREEIMAGE)
        target_compile_definitions(Ghoul PUBLIC "GHOUL_USE_FREEIMAGE")
        target_include_directories(Ghoul SYSTEM PUBLIC ${FREEIMAGE_INCLUDE_PATH})
        target_link_libraries(Ghoul ${FREEIMAGE_LIBRARIES})
    endif ()
    end_dependency()
endif ()

# SOIL
find_package(SOIL QUIET)
if (SOIL_FOUND)
    begin_dependency("SOIL")
    option(GHOUL_USE_SOIL "Use the SOIL library" ON)
    if (GHOUL_USE_SOIL)
        target_compile_definitions(Ghoul PUBLIC "GHOUL_USE_SOIL")
        target_include_directories(Ghoul PUBLIC ${SOIL_INCLUDE_DIRS} ${OPENGL_LIBRARIES})
        target_link_libraries(Ghoul ${SOIL_LIBRARIES})
    endif ()
    end_dependency()
endif ()

# stb_image
option(GHOUL_USE_STB_IMAGE "Use the stb_image library" ON)
if (GHOUL_USE_STB_IMAGE)
    begin_dependency("stb_image")
    target_compile_definitions(Ghoul PUBLIC "GHOUL_USE_STB_IMAGE")
    include_external_library(Ghoul stb_image ${GHOUL_ROOT_DIR}/ext/stb_image)
    end_dependency()
endif ()

# Stackwalker
if (MSVC)
    begin_dependency("Stackwalker")
    target_include_directories(Ghoul PUBLIC ${GHOUL_ROOT_DIR}/ext/stackwalker)
    include_external_library(Ghoul StackWalker
        ${GHOUL_ROOT_DIR}/ext/stackwalker ${GHOUL_ROOT_DIR}/ext/stackwalker)
    end_dependency()
endif ()

begin_dependency("WebSocketPP")
# WebSocket++ / WebSocketPP
include(ExternalProject)
set(WEBSOCKETPP_ROOT_DIR ${GHOUL_ROOT_DIR}/ext/websocketpp)
ExternalProject_Add(websocketpp
    SOURCE_DIR ${WEBSOCKETPP_ROOT_DIR}
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_ARGS ${EXTERNAL_TOOLCHAIN} ${EXTRA_EXTERNAL_ARGS} -DBUILD_TESTS=OFF -DBUILD_EXAMPLES=OFF
    INSTALL_COMMAND ""
    LOG_CONFIGURE 1
    LOG_BUILD 1
    STEP_TARGETS configure build
)
add_dependencies(Ghoul websocketpp)
target_include_directories(Ghoul SYSTEM PUBLIC ${WEBSOCKETPP_ROOT_DIR})
set_folder_location(websocketpp "External")
set_folder_location(websocketpp-build "External")
set_folder_location(websocketpp-configure "External")
end_dependency()

# Freetype
if (GHOUL_MODULE_FONTRENDERING)
    begin_module("FontRendering")
    set(FREETYPE_DIST OFF CACHE BOOL "")
    include_external_library(Ghoul freetype
        ${GHOUL_ROOT_DIR}/ext/freetype2 ${GHOUL_ROOT_DIR}/ext/freetype2/include)

    mark_as_advanced(FT_WITH_BZIP2 FT_WITH_HARFBUZZ FT_WITH_PNG FT_WITH_ZLIB
        FREETYPE_DIST pkgcfg_lib_PC_HARFBUZZ_harfbuzz)
    set_folder_location(dist "External")
    end_module("FontRendering")
endif ()

begin_dependency("tiny-process-library")
include_external_library(Ghoul tiny-process-library
    ${GHOUL_ROOT_DIR}/ext/tiny-process-library)
mark_as_advanced(BUILD_TESTING)
end_dependency("tiny-process-library")



#############################
# Other applications
#############################
option(GHOUL_HAVE_TESTS "Activate the unit tests" ON)
if (GHOUL_HAVE_TESTS)
    begin_header("Generating tests")
    file(GLOB_RECURSE GHOUL_TEST_FILES ${GHOUL_ROOT_DIR}/tests/*.inl)
    add_executable(GhoulTest ${GHOUL_ROOT_DIR}/tests/main.cpp ${GHOUL_TEST_FILES})

    target_compile_definitions(GhoulTest PRIVATE
        # Jenkins shouldn't ask for asserts when they happen, but just throw
        "GHL_THROW_ON_ASSERT"
        "GHOUL_HAVE_TESTS"
        "GTEST_HAS_TR1_TUPLE=0"
        "GTEST_HAS_GLOBAL_STRING=0"
        "GHOUL_ROOT_DIR=\"${GHOUL_ROOT_DIR}\""
    )

    set_ghoul_compile_settings(GhoulTest)

    if (MSVC)
        target_compile_options(GhoulTest PUBLIC "-Od" "/bigobj")
    else ()
        target_compile_options(GhoulTest PUBLIC "-O0" )
    endif ()

    target_include_directories(GhoulTest PUBLIC
        "${GHOUL_ROOT_DIR}/ext/googletest/googletest/include" "${GHOUL_ROOT_DIR}")

    include_gtest("${GHOUL_ROOT_DIR}/ext/googletest")

    target_link_libraries(GhoulTest gtest Ghoul Lua)

    if (APPLE)
        target_link_libraries(GhoulTest
            ${CARBON_LIBRARY}
            ${COREFOUNDATION_LIBRARY}
            ${COCOA_LIBRARY}
            ${APP_SERVICES_LIBRARY}
        )
    endif ()
    end_header()
endif ()

end_header("End: Configuring Ghoul Project")
message(STATUS "\n\n")
