cmake_minimum_required(VERSION 3.31.9)
project(simple_sdl)

# Output Directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)         # SDL
add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)   # SDL_image
add_subdirectory(vendored/SDL_ttf EXCLUDE_FROM_ALL)     # SDL_ttf

include_directories(include/)

add_executable(simple_sdl entry.c src/Vector.c src/Control.c src/Gameplay.c src/User_Interface.c)
target_link_libraries(simple_sdl PRIVATE SDL3::SDL3 SDL3_ttf::SDL3_ttf SDL3_image::SDL3_image)