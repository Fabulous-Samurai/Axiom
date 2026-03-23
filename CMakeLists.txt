cmake_minimum_required(VERSION 3.14)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

# Project and Version
project(axiom_engine VERSION 3.1.1)

# Enable Compilation Database
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# FORCE NINJA GENERATOR - MAXIMUM SPEED BUILD SYSTEM
set(CMAKE_GENERATOR "Ninja" CACHE STRING "Generator" FORCE)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Global Third-Party Configuration (MUST BE BEFORE FetchContent)
set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_PTHREAD_AFFINITY OFF CACHE BOOL "" FORCE)

if(APPLE)
    set(HAVE_PTHREAD_AFFINITY OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_HAS_PTHREAD_AFFINITY OFF CACHE BOOL "" FORCE)
    add_compile_definitions(BENCHMARK_HAS_PTHREAD_AFFINITY=0)
endif()
set(PTHREAD_AFFINITY OFF CACHE BOOL "" FORCE)

# Detect common CI environments
set(AXIOM_RUNNING_IN_CI OFF)
if(DEFINED ENV{GITHUB_ACTIONS} OR DEFINED ENV{CI})
    set(AXIOM_RUNNING_IN_CI ON)
endif()

# Dependencies setup
find_package(Threads REQUIRED)
include(FetchContent)

# Eigen3
FetchContent_Declare(
    eigen
    URL https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

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

# Mimalloc
FetchContent_Declare(
    mimalloc
    URL https://github.com/microsoft/mimalloc/archive/refs/tags/v2.0.9.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Nanobind (for Python FFI)
FetchContent_Declare(
    nanobind
    URL https://github.com/wjakob/nanobind/archive/refs/tags/v2.4.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Robin-map (Nanobind dependency)
FetchContent_Declare(
    robin-map
    URL https://github.com/Tessil/robin-map/archive/refs/tags/v1.3.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(eigen asmjit googletest googlebenchmark mimalloc nanobind)

include_directories(SYSTEM ${eigen_SOURCE_DIR})
if(NOT TARGET Eigen3::Eigen)
    add_library(Eigen3::Eigen INTERFACE IMPORTED)
    target_link_libraries(Eigen3::Eigen INTERFACE Eigen)
endif()

# Robin-map and nanobind include paths
include_directories(SYSTEM ${nanobind_SOURCE_DIR}/include)
include_directories(SYSTEM ${robin-map_SOURCE_DIR}/include)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SIMD / AVX configuration
option(AXIOM_ENABLE_SIMD_AVX2 "Enable AVX2 SIMD instructions" ON)
option(AXIOM_ENABLE_SIMD_FMA "Enable FMA instructions" ON)
option(AXIOM_ENABLE_SIMD_AVX512 "Enable AVX-512 SIMD instructions" OFF)
option(AXIOM_ENABLE_SIMD_AVX_VNNI "Enable AVX-VNNI integer dot-product instructions (Alder Lake+/Zen4+)" OFF)

# Process SIMD flags globally
if(AXIOM_ENABLE_SIMD_AVX2)
    if(MSVC)
        add_compile_options(/arch:AVX2)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
        add_compile_options(-mavx2 -mfma)
        add_compile_definitions(AXIOM_SIMD_AVX2_ENABLED AXIOM_SIMD_FMA_ENABLED)
    endif()
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
    add_compile_definitions(AXIOM_SIMD_NEON_ENABLED)
endif()


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
    set(CMAKE_CXX_FLAGS_RELEASE "-O0 -g -DNDEBUG -ffast-math" CACHE STRING "Release flags" FORCE)
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O0 -g -march=native -mtune=native -DNDEBUG -ffast-math" CACHE STRING "Release flags" FORCE)
endif()
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g" CACHE STRING "Debug flags" FORCE)

# Force default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Sanitizer Options
option(AXIOM_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(AXIOM_ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(AXIOM_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

if(AXIOM_ENABLE_ASAN)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()
if(AXIOM_ENABLE_TSAN)
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)
endif()
if(AXIOM_ENABLE_UBSAN)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
endif()

# GoogleTest / GoogleBenchmark integration helper
option(AXIOM_ENABLE_GTEST "Enable GoogleTest integration" ON)
option(AXIOM_ENABLE_BENCHMARK "Enable GoogleBenchmark integration" ON)
include(cmake/enable_gtest_benchmark.cmake)

# Find Python
find_package(Python COMPONENTS Interpreter Development REQUIRED)

# Enforce Zero-Exception Policy
add_compile_definitions(
    GTEST_HAS_EXCEPTIONS=0
    BENCHMARK_HAS_EXCEPTIONS=0
)

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
    src/pluto_controller.cpp
    src/telemetry.cpp
    src/ingress.cpp
    src/cognitive_ecu.cpp
    src/secure_vault.cpp
    src/crash_dump.cpp
)

if(AXIOM_ENABLE_NANOBIND)
    list(APPEND AXIOM_CORE_SOURCES src/nanobind_interface.cpp src/python_bindings.cpp)
    add_compile_definitions(ENABLE_NANOBIND)
endif()

add_library(axiom_engine SHARED ${AXIOM_CORE_SOURCES})
target_include_directories(axiom_engine PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_compile_definitions(axiom_engine PRIVATE AXIOM_ENGINE_EXPORTS)
target_link_libraries(axiom_engine PUBLIC Python::Python PRIVATE asmjit::asmjit Threads::Threads)
if(WIN32)
    target_link_libraries(axiom_engine PUBLIC bcrypt)
endif()
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

add_executable(differential_suite tests/differential_suite.cpp)
target_link_libraries(differential_suite PRIVATE axiom_engine GTest::gtest_main asmjit::asmjit Python::Python)

add_executable(arena_stress tests/concurrency/arena_stress.cpp)
target_link_libraries(arena_stress PRIVATE axiom_engine GTest::gtest_main)

add_executable(simd_matrix_test tests/hardware/simd_matrix_test.cpp)
target_link_libraries(simd_matrix_test PRIVATE axiom_engine GTest::gtest_main)

add_executable(ingress_bench tests/benchmarks/ingress_bench.cpp)
target_link_libraries(ingress_bench PRIVATE axiom_engine benchmark::benchmark_main Threads::Threads)

add_executable(stats_bench tests/benchmarks/stats_bench.cpp)
target_link_libraries(stats_bench PRIVATE axiom_engine benchmark::benchmark_main Threads::Threads)

# New Mantis/Pluto Tests
add_executable(test_ida_star tests/unit/test_ida_star.cpp)
target_link_libraries(test_ida_star PRIVATE axiom_engine GTest::gtest_main)

add_executable(test_pluto_core tests/unit/test_pluto_core.cpp)
target_link_libraries(test_pluto_core PRIVATE axiom_engine GTest::gtest_main)

add_executable(test_statistics_optimized tests/unit/test_statistics_optimized.cpp)
target_link_libraries(test_statistics_optimized PRIVATE axiom_engine GTest::gtest_main)

# Register with CTest
add_test(NAME run_tests COMMAND run_tests)
add_test(NAME test_ida_star COMMAND test_ida_star)
add_test(NAME test_pluto_core COMMAND test_pluto_core)
add_test(NAME giga_test_suite COMMAND giga_test_suite)
add_test(NAME alignment_test COMMAND alignment_test)
add_test(NAME harmonic_arena_test COMMAND harmonic_arena_test)
add_test(NAME enclave_test COMMAND enclave_test)
add_test(NAME differential_suite COMMAND differential_suite)
add_test(NAME arena_stress COMMAND arena_stress)
add_test(NAME simd_matrix_test COMMAND simd_matrix_test)

# Include asmjit headers globally
include_directories(SYSTEM \"${asmjit_SOURCE_DIR}/src\")

