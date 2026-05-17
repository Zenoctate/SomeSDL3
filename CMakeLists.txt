cmake_minimum_required(VERSION 3.31.9)
project(simple_sdl)

# Output Directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)         # SDL
add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)   # SDL_image

include_directories(include/)

add_executable(simple_sdl entry.c src/Vector.c src/Control.c src/Gameplay.c lib/SDL_Stuff.c lib/math.c)
target_link_libraries(simple_sdl PRIVATE SDL3::SDL3 SDL3_image::SDL3_image)