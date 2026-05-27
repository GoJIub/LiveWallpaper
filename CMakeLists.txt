cmake_minimum_required(VERSION 3.7)

project(LiveWallpaper)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(LiveWallpaper main.cpp)
target_link_libraries(LiveWallpaper ${SDL2_LIBRARIES})

target_compile_options(LiveWallpaper PRIVATE -Wall -Wextra)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)