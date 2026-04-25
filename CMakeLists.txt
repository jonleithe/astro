cmake_minimum_required(VERSION 3.16)

project(astro VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(astro
    main.cpp
    src/geo_point.cpp
)

target_include_directories(astro PRIVATE ${CMAKE_SOURCE_DIR}/inc)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(astro PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(astro PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)
