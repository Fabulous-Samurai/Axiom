set(CMAKE_AUTOMOC ON)

# 🏁 ZENITH PILLAR ENFORCEMENT
macro(enforce_zenith_pillars target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /EHs- /GR-)
    else()
        target_compile_options(${target} PRIVATE -fno-exceptions -fno-rtti)
    endif()
    target_compile_definitions(${target} PRIVATE AXIOM_ZENITH_ENFORCED)
endmacro()

# ─── AXIOM Engine Core ────────────────────────────
add_library(axiom_engine_core STATIC
    core/arena_allocator.cpp
    core/memory_pool.cpp
    core/pmu_orchestrator.cpp
    core/pluto_controller.cpp
    core/crash_dump.cpp
    core/unit_manager.cpp
    core/string_helpers.cpp
    core/cpu_optimization.cpp
)
enforce_zenith_pillars(axiom_engine_core)
target_include_directories(axiom_engine_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(axiom_engine_core PUBLIC mimalloc Threads::Threads asmjit::asmjit)
target_compile_definitions(axiom_engine_core PUBLIC ASMJIT_STATIC AXIOM_ENGINE_EXPORTS)

# ─── AXIOM Engine Compute (The Sealed Block) ────────────────────────────
add_library(axiom_engine_lib STATIC
    core/dynamic_calc.cpp
    api/algebraic_parser.cpp
    api/linear_system_parser.cpp
    api/symbolic_parser.cpp
    api/statistics_parser.cpp
    api/plot_parser.cpp
    api/unit_parser.cpp
    api/python_parser.cpp
    api/python_engine.cpp
    api/python_repl.cpp
    api/python_bindings.cpp
    api/nanobind_interface.cpp
    api/axiom_bridge.cpp
    compute/node_dispatcher.cpp
    dispatch/execution_orchestrator.cpp
    dispatch/execution_orchestrator_capi.cpp
    compute/eigen_engine.cpp
    compute/symbolic_engine.cpp
    compute/statistics_engine.cpp
    compute/plot_engine.cpp
    compute/mantis_heuristic.cpp
    compute/zenith_jit.cpp
    compute/sleef_wrapper.cpp
    compute/ast_analyzer.cpp
)
enforce_zenith_pillars(axiom_engine_lib)
set_source_files_properties(api/python_bindings.cpp api/nanobind_interface.cpp PROPERTIES COMPILE_FLAGS "-fexceptions -frtti")
target_link_libraries(axiom_engine_lib PUBLIC axiom_engine_core Python::Python Eigen3::Eigen asmjit::asmjit simdjson sleef atomic)
target_include_directories(axiom_engine_lib PRIVATE ${nanobind_SOURCE_DIR}/include)

# [MANDATORY ALIASES]: Solve transitive dependency issues
add_library(axiom_engine_api ALIAS axiom_engine_lib)
add_library(axiom_engine_compute ALIAS axiom_engine_lib)

# ─── AXIOM Engine Runtime (Service Layer) ──────────────────────────
add_library(axiom_engine_runtime STATIC
    runtime/daemon_engine.cpp
    runtime/secure_vault.cpp
    runtime/cognitive_commander.cpp
    runtime/cognitive_ecu.cpp
    runtime/sentry.cpp
    runtime/sandbox_proxy.cpp
)
enforce_zenith_pillars(axiom_engine_runtime)
target_link_libraries(axiom_engine_runtime PUBLIC axiom_engine_lib)

# ─── AXIOM Engine IPC ──────────────────────────────────────────────
add_library(axiom_engine_ipc STATIC
    ipc/telemetry.cpp
    ipc/ingress.cpp
)
enforce_zenith_pillars(axiom_engine_ipc)
target_link_libraries(axiom_engine_ipc PUBLIC axiom_engine_runtime)

if(WIN32)
    target_link_libraries(axiom_engine_lib PUBLIC bcrypt)
endif()

# Full Engine Alias
add_library(axiom_engine INTERFACE)
target_link_libraries(axiom_engine INTERFACE axiom_engine_ipc axiom_engine_lib)

# ─── AXIOM Sandbox Worker ─────────────────────────────────────────
add_executable(axiom_worker core/native_worker_main.cpp)
target_link_libraries(axiom_worker PRIVATE axiom_engine Threads::Threads)
if(WIN32)
    set_target_properties(axiom_worker PROPERTIES WIN32_EXECUTABLE FALSE)
    target_link_options(axiom_worker PRIVATE "-mconsole")
endif()
