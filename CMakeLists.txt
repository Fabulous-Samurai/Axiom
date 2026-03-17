cmake_minimum_required(VERSION 3.12)
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

# FORCE NINJA GENERATOR - MAXIMUM SPEED BUILD SYSTEM
set(CMAKE_GENERATOR "Ninja" CACHE STRING "Generator" FORCE)

# Project and Version
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
project(axiom_engine VERSION 3.1.1)
find_package(Threads REQUIRED)
include(FetchContent)

# Eigen3
FetchContent_Declare(
    eigen
    URL https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(eigen)
include_directories(SYSTEM ${eigen_SOURCE_DIR})
if(NOT TARGET Eigen3::Eigen)
    add_library(Eigen3::Eigen INTERFACE IMPORTED)
    target_link_libraries(Eigen3::Eigen INTERFACE Eigen)
endif()


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SIMD / AVX configuration
option(AXIOM_ENABLE_SIMD_AVX2 "Enable AVX2 SIMD instructions" ON)
option(AXIOM_ENABLE_SIMD_FMA "Enable FMA instructions" ON)
option(AXIOM_ENABLE_SIMD_AVX512 "Enable AVX-512 SIMD instructions" OFF)
option(AXIOM_ENABLE_SIMD_AVX_VNNI "Enable AVX-VNNI integer dot-product instructions (Alder Lake+/Zen4+)" OFF)

# Auto-install optional Python GUI dependencies via pip
option(AXIOM_AUTO_INSTALL_PYTHON_DEPS "Install optional Python GUI dependencies via pip" ON)
option(AXIOM_ENABLE_EMBEDDED_PYTHON_ENGINE "Build embedded PythonEngine/PythonParser/PythonREPL sources" OFF)
option(AXIOM_ENABLE_CXX20_MODULES "Enable incremental C++20 modules migration (experimental)" OFF)
option(AXIOM_ENABLE_NANOBIND "Enable nanobind FFI integration" ON)
option(AXIOM_ENABLE_DOXYGEN "Enable Doxygen API documentation target" OFF)
option(AXIOM_ENABLE_HARMONIC_ARENA "Enable HarmonicArena lock-free fast-path backend" OFF)
option(AXIOM_ENABLE_MOLD_LINKER "Try to use mold linker for faster incremental links" OFF)
option(AXIOM_ENABLE_SIZE_GUARDS "Enable section GC and symbol stripping in Release" ON)
option(AXIOM_AUTO_SIGN_WINDOWS "Automatically Authenticode-sign Windows binaries after build" ON)
set(AXIOM_WINDOWS_SIGNING_SUBJECT "CN=AXIOM Local Dev Code Signing" CACHE STRING "Subject for local Windows code-signing certificate")
set(AXIOM_PY_REQUIREMENTS "${CMAKE_SOURCE_DIR}/requirements-optional.txt")

# Detect common CI environments and adjust unsafe, machine-specific defaults.
set(AXIOM_RUNNING_IN_CI OFF)
if(DEFINED ENV{GITHUB_ACTIONS})
    if($ENV{GITHUB_ACTIONS})
        set(AXIOM_RUNNING_IN_CI ON)
    endif()
elseif(DEFINED ENV{CI})
    if($ENV{CI})
        set(AXIOM_RUNNING_IN_CI ON)
    endif()
endif()

# NINJA PARALLEL BUILD OPTIMIZATION
include(ProcessorCount)
ProcessorCount(CPU_COUNT)
if(NOT CPU_COUNT EQUAL 0)
    set(CMAKE_BUILD_PARALLEL_LEVEL ${CPU_COUNT})
else()
    set(CMAKE_BUILD_PARALLEL_LEVEL 8)
endif()

# NINJA-ONLY OPTIMIZATION FLAGS
if(AXIOM_RUNNING_IN_CI)
    set(CMAKE_CXX_FLAGS_RELEASE "-O0 -g -DNDEBUG -ffast-math " CACHE STRING "Release flags" FORCE)
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O0 -g -march=native -mtune=native -DNDEBUG -ffast-math " CACHE STRING "Release flags" FORCE)
endif()
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g" CACHE STRING "Debug flags" FORCE)

# Force release mode for maximum speed
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# GoogleTest / GoogleBenchmark integration helper
option(AXIOM_ENABLE_GTEST "Enable GoogleTest integration" ON)
option(AXIOM_ENABLE_BENCHMARK "Enable GoogleBenchmark integration" ON)
include(cmake/enable_gtest_benchmark.cmake)

# Find Python
find_package(Python COMPONENTS Interpreter Development REQUIRED)

# DEPENDENCIES (FetchContent)
include(FetchContent)

# AsmJit
FetchContent_Declare(
    asmjit
    URL      https://github.com/asmjit/asmjit/archive/64a88ed1d8abb2e2b17a938a5ce7c1b66dabb695.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(ASMJIT_STATIC ON CACHE BOOL "" FORCE)

# GoogleTest
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# GoogleBenchmark
FetchContent_Declare(
    googlebenchmark
    URL https://github.com/google/benchmark/archive/refs/tags/v1.9.1.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(HAVE_PTHREAD_AFFINITY OFF CACHE BOOL "" FORCE)
set(BENCHMARK_HAS_PTHREAD_AFFINITY OFF CACHE BOOL "" FORCE)
set(PTHREAD_AFFINITY OFF CACHE BOOL "" FORCE)

# Mimalloc
FetchContent_Declare(
    mimalloc
    URL https://github.com/microsoft/mimalloc/archive/refs/tags/v2.0.9.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Enforce Zero-Exception Policy
add_compile_definitions(
    GTEST_HAS_EXCEPTIONS=0
    BENCHMARK_HAS_EXCEPTIONS=0
)

FetchContent_MakeAvailable(asmjit googletest googlebenchmark mimalloc)

# Core library sources
set(AXIOM_CORE_SOURCES
    src/dynamic_calc.cpp
    src/algebraic_parser.cpp
    src/linear_system_parser.cpp
    src/statistics_parser.cpp
    src/symbolic_parser.cpp
    src/string_helpers.cpp
    src/unit_manager.cpp
    src/unit_parser.cpp
    src/statistics_engine.cpp
    src/symbolic_engine.cpp
    src/plot_engine.cpp
    src/plot_parser.cpp
    src/kensui_bridge.cpp
    src/zenith_jit.cpp
    src/pmu_orchestrator.cpp
    src/python_engine.cpp
    src/python_parser.cpp
    src/python_repl.cpp
    src/arena_allocator.cpp
    src/eigen_engine.cpp
    src/cpu_optimization.cpp
    src/mantis_heuristic.cpp
    src/telemetry.cpp
    src/ingress.cpp
    src/cognitive_ecu.cpp
    src/secure_vault.cpp
    src/crash_dump.cpp
)

add_library(axiom_engine SHARED ${AXIOM_CORE_SOURCES})
target_include_directories(axiom_engine PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_compile_definitions(axiom_engine PRIVATE AXIOM_ENGINE_EXPORTS)
target_link_libraries(axiom_engine PUBLIC Python::Python PRIVATE asmjit::asmjit Threads::Threads)
target_include_directories(axiom_engine PRIVATE ${Python_INCLUDE_DIRS})

add_executable(axiom src/main.cpp)
target_link_libraries(axiom PRIVATE axiom_engine Threads::Threads asmjit::asmjit Python::Python)
target_include_directories(axiom PRIVATE ${Python_INCLUDE_DIRS})

# Tests
enable_testing()

add_executable(run_tests tests/tests.cpp)
target_link_libraries(run_tests PRIVATE axiom_engine Threads::Threads asmjit::asmjit Python::Python GTest::gtest_main)

add_executable(axiom_benchmark tests/benchmark_suite.cpp)
target_link_libraries(axiom_benchmark PRIVATE axiom_engine Threads::Threads asmjit::asmjit Python::Python benchmark::benchmark_main)

add_executable(giga_test_suite tests/giga_test_suite.cpp)
target_link_libraries(giga_test_suite PRIVATE axiom_engine Threads::Threads asmjit::asmjit Python::Python GTest::gtest_main benchmark::benchmark_main)

# Additional standalone tests
add_executable(alignment_test tests/alignment_test.cpp)
target_link_libraries(alignment_test PRIVATE GTest::gtest_main)

add_executable(harmonic_arena_test tests/harmonic_arena_test.cpp)
target_link_libraries(harmonic_arena_test PRIVATE GTest::gtest_main)

add_executable(harmonic_string_test tests/harmonic_string_test.cpp)
target_link_libraries(harmonic_string_test PRIVATE GTest::gtest_main)

add_executable(enclave_test tests/enclave_test.cpp)
target_link_libraries(enclave_test PRIVATE axiom_engine GTest::gtest_main)

add_executable(ingress_bench tests/benchmarks/ingress_bench.cpp)
target_link_libraries(ingress_bench PRIVATE axiom_engine benchmark::benchmark_main Threads::Threads)

# Register with CTest
add_test(NAME run_tests COMMAND run_tests)
add_test(NAME giga_test_suite COMMAND giga_test_suite)
add_test(NAME alignment_test COMMAND alignment_test)
add_test(NAME harmonic_arena_test COMMAND harmonic_arena_test)
add_test(NAME harmonic_string_test COMMAND harmonic_string_test)
add_test(NAME enclave_test COMMAND enclave_test)

# SIMD flags (simplified)
if(AXIOM_ENABLE_SIMD_AVX2)
    add_compile_options(-mavx2 -mfma)
endif()

# Include asmjit headers globally
include_directories(SYSTEM "${asmjit_SOURCE_DIR}")
