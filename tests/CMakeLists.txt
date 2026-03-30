cmake_minimum_required(VERSION 3.12)

project(axiom_engine_demos)

if(NOT AXIOM_ENABLE_GTEST)
    message(STATUS "GTest disabled; skipping tests subdirectory")
    return()
endif()

# Helper macro to force console mode and correct entry point on Windows
macro(axiom_force_console target)
    if(WIN32)
        set_target_properties(${target} PROPERTIES WIN32_EXECUTABLE FALSE)
        # Force entry point to main to skip WinMain requirement
        target_link_options(${target} PRIVATE "-mconsole" "-Wl,-e,mainCRTStartup")
    endif()
endmacro()

# ─── Benchmarks ──────────────────────────────────────────────────
add_executable(benchmark_suite benchmark_suite.cpp)
axiom_force_console(benchmark_suite)
target_link_libraries(benchmark_suite PRIVATE axiom_engine_ipc axiom_engine_lib axiom_engine_core benchmark::benchmark)

add_executable(parity_suite_benchmark multi_arch/parity_suite_benchmark.cpp)
axiom_force_console(parity_suite_benchmark)
target_link_libraries(parity_suite_benchmark PRIVATE axiom_engine_ipc axiom_engine_lib axiom_engine_core benchmark::benchmark)

find_package(Qt6 COMPONENTS Core REQUIRED)
add_executable(axui_style_pipeline_bench benchmarks/axui_style_pipeline_bench.cpp)
axiom_force_console(axui_style_pipeline_bench)
target_include_directories(axui_style_pipeline_bench PRIVATE ../axui/runtime/include)
target_link_libraries(axui_style_pipeline_bench PRIVATE axui_runtime Qt6::Core axiom_engine_ipc axiom_engine_lib axiom_engine_core benchmark::benchmark)

# ─── JIT Tests ──────────────────────────────────────────────────
add_executable(jit_correctness_test jit_correctness_test.cpp)
axiom_force_console(jit_correctness_test)
target_link_libraries(jit_correctness_test PRIVATE axiom_engine_ipc axiom_engine_lib axiom_engine_core GTest::gtest_main)

add_executable(differential_suite differential_suite.cpp)
axiom_force_console(differential_suite)
target_link_libraries(differential_suite PRIVATE axiom_engine_ipc axiom_engine_lib axiom_engine_core GTest::gtest_main)

add_executable(zenith_integrity_tests zenith_integrity_tests.cpp)
axiom_force_console(zenith_integrity_tests)
target_link_libraries(zenith_integrity_tests PRIVATE axiom_engine_ipc axiom_engine_lib axiom_engine_core GTest::gtest_main)

# ─── Standalone Calculus Suite ──────────────────────────────────
add_executable(test_calculus_suite test_calculus_suite.cpp)
axiom_force_console(test_calculus_suite)
target_link_libraries(test_calculus_suite PRIVATE axiom_engine_ipc axiom_engine_lib axiom_engine_core)

# ─── Sandbox Stress Test ────────────────────────────────────────
add_executable(sandbox_stress_test sandbox_stress_test.cpp)
axiom_force_console(sandbox_stress_test)
target_include_directories(sandbox_stress_test PRIVATE ../engine/include)
target_link_libraries(sandbox_stress_test PRIVATE axiom_engine_ipc axiom_engine_lib axiom_engine_core Threads::Threads)

# ─── Zenith JIT Fuzzer ─────────────────────────────────────────────────
add_executable(zenith_jit_fuzzer fuzz/zenith_jit_fuzzer.cpp)
axiom_force_console(zenith_jit_fuzzer)
target_link_libraries(zenith_jit_fuzzer PRIVATE axiom_engine_ipc axiom_engine_lib axiom_engine_core)

add_subdirectory(unittest)
