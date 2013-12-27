######
# Source
######
set(GHOUL_SOURCE_LOG
    ${GHOUL_ROOT_DIR}/src/logging/callbacklog.cpp
    ${GHOUL_ROOT_DIR}/src/logging/consolelog.cpp
    ${GHOUL_ROOT_DIR}/src/logging/htmllog.cpp
    ${GHOUL_ROOT_DIR}/src/logging/log.cpp
    ${GHOUL_ROOT_DIR}/src/logging/logmanager.cpp
    ${GHOUL_ROOT_DIR}/src/logging/textlog.cpp
    ${GHOUL_ROOT_DIR}/src/logging/streamlog.cpp)
source_group(LogManager FILES ${GHOUL_SOURCE_LOG})    
    
set(GHOUL_SOURCE_OPENGL
    ${GHOUL_ROOT_DIR}/src/opengl/programobject.cpp
    ${GHOUL_ROOT_DIR}/src/opengl/shaderobject.cpp
    ${GHOUL_ROOT_DIR}/src/opengl/shadermanager.cpp
    ${GHOUL_ROOT_DIR}/src/opengl/texture.cpp
    ${GHOUL_ROOT_DIR}/src/opengl/texturemanager.cpp
    ${GHOUL_ROOT_DIR}/src/opengl/texturereader.cpp
    ${GHOUL_ROOT_DIR}/src/opengl/textureunit.cpp
    )
source_group(OpenGL FILES ${GHOUL_SOURCE_OPENGL})    

set(GHOUL_SOURCE_FILESYSTEM
    ${GHOUL_ROOT_DIR}/src/filesystem/directory.cpp
    ${GHOUL_ROOT_DIR}/src/filesystem/file.cpp
    ${GHOUL_ROOT_DIR}/src/filesystem/filesystem.cpp
    )
source_group(FileSystem FILES ${GHOUL_SOURCE_FILESYSTEM})
    
set(GHOUL_SOURCE_MISC
    ${GHOUL_ROOT_DIR}/src/misc/bufferlog.cpp
    ${GHOUL_ROOT_DIR}/src/misc/crc32.cpp
    ${GHOUL_ROOT_DIR}/src/misc/syscapabilities.cpp
	)
source_group(Misc FILES ${GHOUL_SOURCE_MISC})
    
set(GHOUL_SOURCE
    ${GHOUL_SOURCE_CRC32}
    ${GHOUL_SOURCE_FILESYSTEM}
    ${GHOUL_SOURCE_LOG}
    ${GHOUL_SOURCE_MISC}
    ${GHOUL_SOURCE_OPENGL}
    )

######
# Headers
######
set(GHOUL_HEADER_LOG
    ${GHOUL_ROOT_DIR}/include/ghoul/logging/callbacklog.h
    ${GHOUL_ROOT_DIR}/include/ghoul/logging/consolelog.h
    ${GHOUL_ROOT_DIR}/include/ghoul/logging/htmllog.h
    ${GHOUL_ROOT_DIR}/include/ghoul/logging/logmanager.h
    ${GHOUL_ROOT_DIR}/include/ghoul/logging/logmanager.inl
    ${GHOUL_ROOT_DIR}/include/ghoul/logging/log.h
    ${GHOUL_ROOT_DIR}/include/ghoul/logging/textlog.h
    ${GHOUL_ROOT_DIR}/include/ghoul/logging/streamlog.h)
source_group(LogManager FILES ${GHOUL_HEADER_LOG})
    
set(GHOUL_HEADER_OPENGL
    ${GHOUL_ROOT_DIR}/include/ghoul/opengl/ghoul_gl.h
    ${GHOUL_ROOT_DIR}/include/ghoul/opengl/programobject.h
    ${GHOUL_ROOT_DIR}/include/ghoul/opengl/shaderobject.h
    ${GHOUL_ROOT_DIR}/include/ghoul/opengl/shadermanager.h
    ${GHOUL_ROOT_DIR}/include/ghoul/opengl/texture.h
    ${GHOUL_ROOT_DIR}/include/ghoul/opengl/texture.inl
    ${GHOUL_ROOT_DIR}/include/ghoul/opengl/texturemanager.h
    ${GHOUL_ROOT_DIR}/include/ghoul/opengl/texturereader.h
    ${GHOUL_ROOT_DIR}/include/ghoul/opengl/textureunit.h
    )
source_group(OpenGL FILES ${GHOUL_HEADER_OPENGL})    

set(GHOUL_HEADER_FILESYSTEM
    ${GHOUL_ROOT_DIR}/include/ghoul/filesystem/directory.h
    ${GHOUL_ROOT_DIR}/include/ghoul/filesystem/file.h
    ${GHOUL_ROOT_DIR}/include/ghoul/filesystem/filesystem.h
    )
source_group(FileSystem FILES ${GHOUL_HEADER_FILESYSTEM})
    
set(GHOUL_HEADER_MISC
	${GHOUL_ROOT_DIR}/include/ghoul/glm.h
    ${GHOUL_ROOT_DIR}/include/ghoul/misc/bufferlog.h
    ${GHOUL_ROOT_DIR}/include/ghoul/misc/crc32.h
    ${GHOUL_ROOT_DIR}/include/ghoul/misc/syscapabilities.h)
source_group(Misc FILES ${GHOUL_HEADER_MISC})    
    
set(GHOUL_HEADER
    ${GHOUL_HEADER_CRC32}
    ${GHOUL_HEADER_FILESYSTEM}
    ${GHOUL_HEADER_LOG}
    ${GHOUL_HEADER_MISC}
    ${GHOUL_HEADER_OPENGL}
    )

######
# Dependencies
######
set(GHOUL_DEPENDENCIES ${GHOUL_DEPENDENCIES} ${DEVIL_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})

include_directories(
    ${GHOUL_ROOT_DIR}/include
    ${GHOUL_ROOT_DIR}/include/ghoul)
    
if (CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")
    if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    else ()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif ()
    # C4201: nonstandard extension used : nameless struct/union    # Extension is ubiquitous
    # C4127: 'if' or 'while' loop evaluates to a constant          # Used for 'swallowing the semicolon" idiom
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4201 /wd4127")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    if(UNIX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-long-long -pedantic -Wextra")
    endif(UNIX)
endif ()
    
add_library(Ghoul ${GHOUL_SOURCE} ${GHOUL_HEADER})
target_link_libraries(Ghoul ${GHOUL_DEPENDENCIES})