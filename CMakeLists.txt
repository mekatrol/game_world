cmake_minimum_required(VERSION 3.20)
project(game LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLAD
add_library(glad STATIC
    external/glad/src/gl.c
)

target_include_directories(glad PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
)

# App
add_executable(game
    src/main.cpp
    renderer/shader.hpp
    renderer/shader.cpp
    renderer/sprite_renderer.hpp
    renderer/sprite_renderer.cpp
    util/texture.hpp
    util/texture.cpp
    util/sprite_sheet.hpp
)

# Make "renderer/..." and "util/..." includes work from project root
target_include_directories(game PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(game PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

target_link_libraries(game PRIVATE glad glfw OpenGL::GL)
