cmake_minimum_required(VERSION 3.12)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Core)

# ─── Runtime Static Library ───────────────────────────────────────
add_library(axui_runtime STATIC
    src/binding_engine.cpp
    src/dashboard_manager.cpp
    src/jit_execution_manager.cpp
    src/style_pipeline.cpp
    include/axui/dashboard_manager.h
    include/axui/binding_engine.h
    include/axui/jit_execution_manager.h
    include/axui/style_pipeline.h
)

target_include_directories(axui_runtime PUBLIC
    include
    ../compiler/include
    ../../engine/include
)

target_link_libraries(axui_runtime PUBLIC
    axui_compiler
    axiom_engine
    Qt6::Core
)

target_compile_definitions(axui_runtime PRIVATE AXIOM_ENGINE_EXPORTS)

set_property(TARGET axui_runtime PROPERTY CXX_STANDARD 20)

# ─── Runtime Test Executable ──────────────────────────────────────
add_executable(axui_binding_tests
    tests/test_binding.cpp
    tests/test_binding_main.cpp
)

target_link_libraries(axui_binding_tests PRIVATE
    axui_runtime
)

set_property(TARGET axui_binding_tests PROPERTY CXX_STANDARD 20)

# ─── Binding Benchmark Executable ─────────────────────────────────
add_executable(axui_binding_bench
    tests/bench_binding.cpp
)

target_link_libraries(axui_binding_bench PRIVATE
    axui_runtime
)

set_property(TARGET axui_binding_bench PROPERTY CXX_STANDARD 20)
