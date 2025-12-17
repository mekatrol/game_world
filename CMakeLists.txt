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

# Packages (vcpkg)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(msdfgen CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Freetype CONFIG REQUIRED)
find_package(PNG CONFIG REQUIRED)

# App (runtime)
add_executable(game
    src/main.cpp
    renderer/shader.hpp
    renderer/shader.cpp
    renderer/sprite_renderer.hpp
    renderer/sprite_renderer.cpp
    util/texture.hpp
    util/texture.cpp
    util/sprite_sheet.hpp
    util/msdf_font.hpp
    util/animation_library.hpp
    util/animation_library.cpp
)

target_include_directories(game PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(game PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
)

target_link_libraries(game PRIVATE
    glad
    glfw
    OpenGL::GL
    msdfgen::msdfgen
    nlohmann_json::nlohmann_json
    Freetype::Freetype
    PNG::PNG
)

# Atlas generator (offline tool)
add_executable(atlas_generator
    util/atlas_generator.cpp
)

target_include_directories(atlas_generator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(atlas_generator PRIVATE
    msdfgen::msdfgen
    nlohmann_json::nlohmann_json
    Freetype::Freetype
    PNG::PNG
)
