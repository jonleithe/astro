cmake_minimum_required(VERSION 3.16)

project(astro VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(astro
    main.cpp
    src/cli_file_io.cpp
    src/cli_input.cpp
    src/geo_point.cpp
    src/print_output.cpp
    src/sun_position.cpp
    src/moon_position.cpp
    src/planet_position.cpp
    src/terrain_horizon.cpp
    src/tui.cpp
)

add_executable(test_sun_position
    tests/test_sun_position.cpp
    src/geo_point.cpp
    src/sun_position.cpp
)

target_include_directories(astro PRIVATE ${CMAKE_SOURCE_DIR}/inc)
target_include_directories(test_sun_position PRIVATE ${CMAKE_SOURCE_DIR}/inc)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(astro PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(test_sun_position PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(astro PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)

enable_testing()
add_test(NAME sun_position_sanity COMMAND test_sun_position)

add_executable(test_geo_point tests/test_geo_point.cpp src/geo_point.cpp)
target_include_directories(test_geo_point PRIVATE ${CMAKE_SOURCE_DIR}/inc)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(test_geo_point PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_test(NAME geo_point_geometry COMMAND test_geo_point)

option(ASTRO_ENABLE_TUI "Build the optional ncurses terminal interface" ON)
if(ASTRO_ENABLE_TUI)
    find_package(Curses QUIET)
    if(CURSES_FOUND)
        target_compile_definitions(astro PRIVATE ASTRO_HAVE_CURSES)
        target_include_directories(astro PRIVATE ${CURSES_INCLUDE_DIRS})
        target_link_libraries(astro PRIVATE ${CURSES_LIBRARIES})
    else()
        message(STATUS "ncurses unavailable; building without TUI support")
    endif()
endif()

add_executable(test_moon_position tests/test_moon_position.cpp src/moon_position.cpp src/sun_position.cpp src/geo_point.cpp)
target_include_directories(test_moon_position PRIVATE ${CMAKE_SOURCE_DIR}/inc)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(test_moon_position PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_test(NAME moon_position_geometry COMMAND test_moon_position)

add_test(NAME cli_object_selection
    COMMAND ${CMAKE_COMMAND} -DASTRO=$<TARGET_FILE:astro> -P ${CMAKE_SOURCE_DIR}/tests/test_objects.cmake)

add_executable(test_terrain_horizon tests/test_terrain_horizon.cpp src/terrain_horizon.cpp src/geo_point.cpp)
target_include_directories(test_terrain_horizon PRIVATE ${CMAKE_SOURCE_DIR}/inc)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(test_terrain_horizon PRIVATE -Wall -Wextra -Wpedantic)
endif()
add_test(NAME terrain_horizon_geometry COMMAND test_terrain_horizon)

add_test(NAME terrain_cli
    COMMAND ${CMAKE_COMMAND} -DASTRO=$<TARGET_FILE:astro> -DSOURCE=${CMAKE_SOURCE_DIR} -P ${CMAKE_SOURCE_DIR}/tests/test_terrain_cli.cmake)
