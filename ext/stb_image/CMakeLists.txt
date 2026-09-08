# The stb headers are provided by vcpkg; only the translation unit that instantiates them
# lives here, as it selects the stb feature macros
add_library(stb_image OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/stb_image.c)
target_include_directories(stb_image SYSTEM PRIVATE ${STB_INCLUDE_DIR})
