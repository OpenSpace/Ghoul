cmake_minimum_required(VERSION 3.10)
project(stb_image)

add_library(stb_image ${PROJECT_SOURCE_DIR}/stb_image.c)
target_include_directories(stb_image PUBLIC ${PROJECT_SOURCE_DIR})
