cmake_minimum_required(VERSION 3.0)
project(renderdoc)

add_library(renderdoc INTERFACE)
target_include_directories(renderdoc INTERFACE ${PROJECT_SOURCE_DIR})
