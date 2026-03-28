cmake_minimum_required(VERSION 3.16)
project(axui_module VERSION 0.1.0 LANGUAGES CXX)

option(AXUI_BUILD_TESTS "Build AXUI tests" ON)
option(AXUI_ENABLE_PCH "Enable AXUI Precompiled Headers" OFF)

# Absolute path to the MinGW fix header
set(MINGW_FIX_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/compiler/include/axui/mingw_fix.h")

# if(MINGW)
#     message(STATUS "Applying global MinGW at_quick_exit workaround: ${MINGW_FIX_HEADER}")
#     add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-include${MINGW_FIX_HEADER}>")
# endif()

find_package(Qt6 COMPONENTS Core Quick Test REQUIRED)

# Simply delegate to subdirectories
add_subdirectory(compiler)
add_subdirectory(runtime)
add_subdirectory(qml)
add_subdirectory(widgets)


