cmake_minimum_required(VERSION 3.7)

project(LiveWallpaper)

find_package(SDL2 REQUIRED)
find_package(X11 REQUIRED)

add_executable(LiveWallpaper main.cpp)
target_link_libraries(LiveWallpaper ${SDL2_LIBRARIES} ${X11_LIBRARIES})

target_compile_options(LiveWallpaper PRIVATE -Wall -Wextra)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)