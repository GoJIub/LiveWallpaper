cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(LiveWallpaper)

include(FetchContent)
include(CTest)

find_package(SDL2 REQUIRED)
find_package(X11 REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

if(TARGET CURL::libcurl)
    set(CURL_TARGET CURL::libcurl)
else()
    set(CURL_TARGET ${CURL_LIBRARIES})
endif()

add_library(LiveWallpaperCore
    src/particles/particle.cpp
    src/data/system_data.cpp
    src/data/weather.cpp
    src/config/config.cpp
)

target_link_libraries(LiveWallpaperCore
    ${SDL2_LIBRARIES}
    ${CURL_TARGET}
    nlohmann_json::nlohmann_json
)
target_include_directories(LiveWallpaperCore PUBLIC src ${CURL_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS})
target_compile_definitions(LiveWallpaperCore PRIVATE
    INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
    DATA_DIR="${CMAKE_INSTALL_PREFIX}/share/livewallpaper"
)
target_compile_options(LiveWallpaperCore PRIVATE -Wall -Wextra)

add_executable(LiveWallpaper
    src/main.cpp
    src/window/desktop_window.cpp
    src/signal_handler/signal_handler.cpp
)

configure_file(
    ${CMAKE_SOURCE_DIR}/config.default.json
    ${CMAKE_BINARY_DIR}/config.default.json
    COPYONLY
)

target_link_libraries(LiveWallpaper
    LiveWallpaperCore
    ${SDL2_LIBRARIES}
    ${X11_LIBRARIES}
)
target_include_directories(LiveWallpaper PRIVATE src)
target_compile_definitions(LiveWallpaper PRIVATE
    INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
    DATA_DIR="${CMAKE_INSTALL_PREFIX}/share/livewallpaper"
)

target_compile_options(LiveWallpaper PRIVATE -Wall -Wextra)

install(FILES config.default.json DESTINATION share/livewallpaper)
install(TARGETS LiveWallpaper DESTINATION bin)
install(FILES livewallpaper.service DESTINATION share/livewallpaper)
install(PROGRAMS install.sh DESTINATION share/livewallpaper)
install(PROGRAMS uninstall.sh DESTINATION share/livewallpaper)

if(BUILD_TESTING)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(LiveWallpaperTests
        tests/config_manager_test.cpp
        tests/particle_test.cpp
        tests/system_data_test.cpp
        tests/weather_data_test.cpp
    )

    target_link_libraries(LiveWallpaperTests
        LiveWallpaperCore
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(LiveWallpaperTests)
endif()

find_program(VALGRIND valgrind)
if(VALGRIND)
    add_custom_target(valgrind-memcheck
        COMMAND ${VALGRIND} --tool=memcheck --leak-check=full --show-leak-kinds=all
            $<TARGET_FILE:LiveWallpaper>
        DEPENDS LiveWallpaper
    )

    add_custom_target(valgrind-massif
        COMMAND ${VALGRIND} --tool=massif --massif-out-file=massif.out
            $<TARGET_FILE:LiveWallpaper>
        COMMAND ms_print massif.out > massif.txt
        DEPENDS LiveWallpaper
    )

    add_custom_target(valgrind-callgrind
        COMMAND ${VALGRIND} --tool=callgrind
            $<TARGET_FILE:LiveWallpaper>
        DEPENDS LiveWallpaper
    )
endif()
