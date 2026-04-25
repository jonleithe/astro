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
