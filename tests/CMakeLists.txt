cmake_minimum_required(VERSION 3.12)

project(axiom_engine_demos)

if(NOT AXIOM_ENABLE_GTEST)
    message(STATUS "GTest disabled; skipping tests subdirectory")
    return()
endif()

# Use the macro to add the benchmark
axiom_add_benchmark(benchmark_suite benchmark_suite.cpp)

# Force Console mode on Windows to avoid WinMain issues
if(WIN32)
    target_link_options(benchmark_suite PRIVATE "-mconsole")
endif()

# Reorder and duplicate to resolve circular refs between core and api
target_link_libraries(benchmark_suite PRIVATE 
    axiom_engine_api
    axiom_engine_compute
    axiom_engine_ipc
    axiom_engine_core
)

add_subdirectory(unittest)

# ─── JIT Tests ──────────────────────────────────────────────────
add_executable(jit_correctness_test jit_correctness_test.cpp)
target_link_libraries(jit_correctness_test PRIVATE 
    axiom_engine_api
    axiom_engine_compute
    axiom_engine_ipc
    axiom_engine_core
    GTest::gtest_main
)

add_executable(differential_suite differential_suite.cpp)
target_link_libraries(differential_suite PRIVATE 
    axiom_engine_api
    axiom_engine_compute
    axiom_engine_ipc
    axiom_engine_core
    GTest::gtest_main
)

add_executable(zenith_integrity_tests zenith_integrity_tests.cpp)
target_link_libraries(zenith_integrity_tests PRIVATE 
    axiom_engine_api
    axiom_engine_compute
    axiom_engine_ipc
    axiom_engine_core
    GTest::gtest_main
)

# â”€â”€â”€ Sandbox Stress Test â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
add_executable(sandbox_stress_test sandbox_stress_test.cpp)
target_include_directories(sandbox_stress_test PRIVATE ../engine/include)
target_link_libraries(sandbox_stress_test PRIVATE 
    axiom_engine_api
    axiom_engine_compute
    axiom_engine_ipc
    axiom_engine_core
    Threads::Threads
)

# â”€â”€â”€ Multi-Arch Parity Benchmark â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
axiom_add_benchmark(parity_suite_benchmark multi_arch/parity_suite_benchmark.cpp)
if(WIN32)
    target_link_options(parity_suite_benchmark PRIVATE "-mconsole")
endif()
target_link_libraries(parity_suite_benchmark PRIVATE
    axiom_engine_api
    axiom_engine_compute
    axiom_engine_ipc
    axiom_engine_core
)

# ─── StylePipeline Benchmark ───────────────────────────────────────────
# We need Qt to run QCoreApplication
find_package(Qt6 COMPONENTS Core REQUIRED)
axiom_add_benchmark(axui_style_pipeline_bench benchmarks/axui_style_pipeline_bench.cpp)
if(WIN32)
    target_link_options(axui_style_pipeline_bench PRIVATE "-mconsole")
endif()
target_include_directories(axui_style_pipeline_bench PRIVATE ../axui/runtime/include)
target_link_libraries(axui_style_pipeline_bench PRIVATE
    axui_runtime
    Qt6::Core
    axiom_engine_ipc
)

# ─── Zenith JIT Fuzzer ─────────────────────────────────────────────────
# This is usually compiled with Clang and -fsanitize=fuzzer
add_executable(zenith_jit_fuzzer fuzz/zenith_jit_fuzzer.cpp)
target_link_libraries(zenith_jit_fuzzer PRIVATE
    axiom_engine_api
    axiom_engine_compute
    axiom_engine_ipc
    axiom_engine_core
)
# Note: To build the fuzzer properly, add -fsanitize=fuzzer to CXX flags when configuring CMake.

