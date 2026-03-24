cmake_minimum_required(VERSION 3.16)
project(axui_compiler VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# simdjson
include(FetchContent)
FetchContent_Declare(
    simdjson
    GIT_REPOSITORY https://github.com/simdjson/simdjson.git
    GIT_TAG v3.10.1
)
FetchContent_MakeAvailable(simdjson)

# AXUI Compiler static library
add_library(axui_compiler STATIC
    src/parser.cpp
    src/resolver.cpp
    src/binding.cpp
    src/compiler.cpp
)
set_property(TARGET axui_compiler PROPERTY CXX_STANDARD 20)
set_property(TARGET axui_compiler PROPERTY CXX_STANDARD_REQUIRED ON)

target_include_directories(axui_compiler PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(axui_compiler PUBLIC
    simdjson
)

# Test executable
add_executable(axui_tests
    tests/test_main.cpp
    tests/test_parser.cpp
    tests/test_resolver.cpp
    tests/test_binding.cpp
)

target_link_libraries(axui_tests PRIVATE
    axui_compiler
)

set_property(TARGET axui_tests PROPERTY CXX_STANDARD 20)

# Benchmark executable
add_executable(axui_bench
    tests/bench_parser.cpp
)

target_link_libraries(axui_bench PRIVATE
    axui_compiler
)

set_property(TARGET axui_bench PROPERTY CXX_STANDARD 20)

add_test(NAME axui_tests COMMAND axui_tests)
add_test(NAME axui_bench COMMAND axui_bench)
