cmake_minimum_required(VERSION 3.7)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(LiveWallpaper)

find_package(SDL2 REQUIRED)
find_package(X11 REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

if(TARGET CURL::libcurl)
    set(CURL_TARGET CURL::libcurl)
else()
    set(CURL_TARGET ${CURL_LIBRARIES})
endif()

add_executable(LiveWallpaper
    src/main.cpp
    src/particles/particle.cpp
    src/window/desktop_window.cpp
    src/data/system_data.cpp
    src/data/weather.cpp
)

target_link_libraries(LiveWallpaper
    ${SDL2_LIBRARIES}
    ${X11_LIBRARIES}
    ${CURL_TARGET}
    nlohmann_json::nlohmann_json
)
target_include_directories(LiveWallpaper PRIVATE src ${CURL_INCLUDE_DIRS})

target_compile_options(LiveWallpaper PRIVATE -Wall -Wextra)
