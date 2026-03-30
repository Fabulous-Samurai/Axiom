cmake_minimum_required(VERSION 3.16)
project(axui_module VERSION 0.1.0 LANGUAGES CXX)

option(AXUI_BUILD_TESTS "Build AXUI tests" ON)
option(AXUI_ENABLE_PCH "Enable AXUI Precompiled Headers" OFF)

find_package(Qt6 COMPONENTS Core Quick Test REQUIRED)

# --- AXUI Core Library (AppRunner, etc.) ---
add_library(axui_core STATIC
    core/app_runner.cpp
)
target_include_directories(axui_core PUBLIC include)
target_link_libraries(axui_core PRIVATE Qt6::Core Qt6::Quick)

# --- Subdirectories ---
add_subdirectory(compiler)
add_subdirectory(runtime)
add_subdirectory(qml)
add_subdirectory(widgets)

# --- Global Interface Library ---
add_library(axui INTERFACE)
target_link_libraries(axui INTERFACE 
    axui_core
    axui_compiler
    axui_runtime
    axui_qml
    axui_widgets
)
