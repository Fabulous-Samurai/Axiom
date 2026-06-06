cmake_minimum_required(VERSION 3.20)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(Axiom VERSION 3.1.2 LANGUAGES C CXX)

# Policy settings
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

# Global Options
option(AXIOM_BUILD_TESTS "Build tests" ON)
option(AXIOM_BUILD_STUDIO "Build Studio" ON)
option(AXIOM_BUILD_DASHBOARD "Build Dashboard" ON)
option(AXIOM_BUILD_LENS "Build Lens" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# 3rd Party Dependencies
include(FetchContent)

set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(EIGEN_BUILD_PKGCONFIG OFF CACHE BOOL "" FORCE)

FetchContent_Declare(simdjson GIT_REPOSITORY https://github.com/simdjson/simdjson.git GIT_TAG v3.10.1)
FetchContent_Declare(eigen URL https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.zip)
FetchContent_Declare(asmjit URL https://github.com/asmjit/asmjit/archive/64a88ed1d8abb2e2b17a938a5ce7c1b66dabb695.zip)
FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip)
FetchContent_Declare(googlebenchmark URL https://github.com/google/benchmark/archive/refs/tags/v1.9.1.zip)
FetchContent_Declare(mimalloc URL https://github.com/microsoft/mimalloc/archive/refs/tags/v2.0.9.zip)
FetchContent_Declare(nanobind GIT_REPOSITORY https://github.com/wjakob/nanobind.git GIT_TAG v2.4.0 GIT_SUBMODULES_RECURSE YES)
FetchContent_Declare(robin-map URL https://github.com/Tessil/robin-map/archive/refs/tags/v1.3.0.zip)
FetchContent_Declare(sleef GIT_REPOSITORY https://github.com/shibatch/sleef.git GIT_TAG 3.6.1)

# nanobind options
set(NB_STATIC ON CACHE BOOL "" FORCE)
set(NB_CREATE_BUNDLE ON CACHE BOOL "" FORCE)

# sleef options
set(SLEEF_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SLEEF_BUILD_INLINE_HEADERS ON CACHE BOOL "" FORCE)
set(SLEEF_BUILD_QUAD OFF CACHE BOOL "" FORCE)
add_compile_definitions(SLEEF_STATIC_LIBS)

# Make Available
FetchContent_MakeAvailable(simdjson eigen asmjit googletest googlebenchmark mimalloc robin-map nanobind sleef)




# Compiler Flags
if(MSVC)
    add_compile_options(/arch:AVX2 /W0 /Zc:__cplusplus)
    add_definitions(-DASMJIT_STATIC)
else()
    add_compile_options(-mavx2 -mfma -w)
    add_definitions(-DASMJIT_STATIC)
endif()

# Global Includes
include_directories(SYSTEM ${eigen_SOURCE_DIR})
include_directories(SYSTEM ${simdjson_SOURCE_DIR}/include)
include_directories(SYSTEM ${asmjit_SOURCE_DIR}/src)
include_directories(SYSTEM ${sleef_BINARY_DIR}/include)

# Find Dependencies
find_package(Python COMPONENTS Interpreter Development REQUIRED)
find_package(Threads REQUIRED)
find_package(Vulkan REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml QuickControls2 OpenGL Test)

# Core Engine, Expressway, and AXUI Subdirectories
add_subdirectory(engine)
add_subdirectory(axui)
add_subdirectory(expressway)

# Products
if(AXIOM_BUILD_DASHBOARD)
    add_subdirectory(products/dashboard)
endif()

if(AXIOM_BUILD_STUDIO)
    add_subdirectory(products/studio)
endif()

if(AXIOM_BUILD_LENS)
    add_subdirectory(products/lens)
endif()

# Global Tests
if(AXIOM_BUILD_TESTS)
    set(AXIOM_ENABLE_GTEST ON CACHE BOOL "" FORCE)
    set(AXIOM_ENABLE_BENCHMARK ON CACHE BOOL "" FORCE)
    include(cmake/enable_gtest_benchmark.cmake)
    enable_testing()
    add_subdirectory(tests)
endif()
