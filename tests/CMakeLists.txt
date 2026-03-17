cmake_minimum_required(VERSION 3.12)

project(axiom_engine_demos)

# add_executable(harmonic_arena_demo harmonic_arena_demo.cpp)
# target_include_directories(harmonic_arena_demo PRIVATE ${CMAKE_SOURCE_DIR}/include)
# set_target_properties(harmonic_arena_demo PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)

# ASAN flags when supported (non-Windows)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    option(ENABLE_ASAN "Enable AddressSanitizer for demo target" OFF)
    if(ENABLE_ASAN)
        # target_compile_options(harmonic_arena_demo PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        # target_link_options(harmonic_arena_demo PRIVATE -fsanitize=address)
    endif()
endif()

if(NOT AXIOM_ENABLE_GTEST)
    message(STATUS "GTest disabled; skipping tests subdirectory")
    return()
endif()

add_subdirectory(unittest)
