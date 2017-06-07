cmake_minimum_required(VERSION 3.0)
project(Lua)

set(LUA_ROOT_DIR ${PROJECT_SOURCE_DIR})

# LUA_USE_APICHECK  in full debug modes

file(GLOB LUA_SOURCE ${LUA_ROOT_DIR}/src/*.c)
file(GLOB LUA_HEADER ${LUA_ROOT_DIR}/src/*.h)

list(REMOVE_ITEM LUA_SOURCE "${LUA_ROOT_DIR}/src/lua.c" "${LUA_ROOT_DIR}/src/luac.c")

add_library(Lua STATIC ${LUA_SOURCE})

if (WIN32)
    target_compile_definitions(Lua PRIVATE "_CRT_SECURE_NO_WARNINGS")
elseif (UNIX)
    target_compile_definitions(Lua PRIVATE "LUA_USE_POSIX")
endif()


target_include_directories(Lua PUBLIC "${LUA_ROOT_DIR}/src")
