cmake_minimum_required(VERSION 3.7)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(LiveWallpaper)

find_package(SDL2 REQUIRED)
find_package(X11 REQUIRED)

add_executable(LiveWallpaper
    src/main.cpp
    src/particles/particle.cpp
    src/window/desktop_window.cpp
)

target_link_libraries(LiveWallpaper ${SDL2_LIBRARIES} ${X11_LIBRARIES})
target_include_directories(LiveWallpaper PRIVATE src)

target_compile_options(LiveWallpaper PRIVATE -Wall -Wextra)