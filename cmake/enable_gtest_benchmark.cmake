# Minimal stub for projects that reference this include.
# This file provides a no-op function and a cache variable so CMake configure doesn't fail
# Replace with the real implementation when available.

if(NOT DEFINED AXIOM_ENABLE_GTEST_BENCHMARK)
  option(AXIOM_ENABLE_GTEST_BENCHMARK "Enable gtest & benchmark integration (stub)" OFF)
endif()

function(enable_gtest_benchmark)
  # Stub: optionally configure tests/benchmark integration here.
  message(STATUS "enable_gtest_benchmark() called — using stub implementation")
endfunction()
include(FetchContent)

option(AXIOM_ENABLE_GTEST "Enable GoogleTest integration" OFF)
option(AXIOM_ENABLE_BENCHMARK "Enable GoogleBenchmark integration" OFF)

if(AXIOM_ENABLE_GTEST)
    # find_package(GTest QUIET)
    if(NOT GTest_FOUND AND NOT TARGET gtest)
        message(STATUS "GoogleTest being fetched via FetchContent in main CMakeLists.txt or here...")
        # FetchContent_Declare moved to main CMakeLists.txt for consistency
    endif()
endif()

if(AXIOM_ENABLE_BENCHMARK)
    # find_package(benchmark QUIET)
    if(NOT benchmark_FOUND AND NOT TARGET benchmark::benchmark)
        message(STATUS "Google Benchmark being fetched via FetchContent in main CMakeLists.txt or here...")
        set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark tests" FORCE)
        set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable benchmark gtest tests" FORCE)
    endif()
endif()

function(axiom_add_gtest target sources)
    if(NOT AXIOM_ENABLE_GTEST)
        message(FATAL_ERROR "AXIOM_ENABLE_GTEST is OFF; enable to add tests")
    endif()
    message(STATUS "Adding GoogleTest target: ${target}")
    add_executable(${target} ${sources})
    target_link_libraries(${target} PRIVATE GTest::gtest_main Threads::Threads)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR})
    add_test(NAME ${target} COMMAND ${target})
    message(STATUS "Test registered with CTest: ${target}")
endfunction()

function(axiom_add_benchmark target sources)
    if(NOT AXIOM_ENABLE_BENCHMARK)
        message(FATAL_ERROR "AXIOM_ENABLE_BENCHMARK is OFF; enable to add benchmarks")
    endif()
    add_executable(${target} ${sources})
    target_link_libraries(${target} PRIVATE benchmark::benchmark Threads::Threads)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR})
endfunction()
