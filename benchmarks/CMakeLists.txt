cmake_minimum_required(VERSION 3.12)

if(NOT AXIOM_ENABLE_BENCHMARK)
    message(STATUS "Benchmarking disabled; skipping benchmarks subdirectory")
    return()
endif()

project(axiom_benchmarks)

add_subdirectory(sample)

add_executable(zero_allocation_benchmark zero_allocation_benchmark.cpp)
target_link_libraries(zero_allocation_benchmark benchmark::benchmark)
