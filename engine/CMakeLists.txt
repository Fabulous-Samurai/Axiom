set(CMAKE_AUTOMOC ON)

# 🏁 ZENITH PILLAR ENFORCEMENT (Mandatory Iron Will)
# ---------------------------------------------------------------------
macro(enforce_zenith_pillars target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /EHs- /GR-)
    else()
        target_compile_options(${target} PRIVATE -fno-exceptions -fno-rtti)
    endif()
    target_compile_definitions(${target} PRIVATE AXIOM_ZENITH_ENFORCED)
endmacro()

# ─── AXIOM Engine Core (Pure Utilities) ────────────────────────────
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

target_include_directories(axiom_engine_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(axiom_engine_core PUBLIC
    mimalloc
    Threads::Threads
    asmjit::asmjit
)

target_compile_definitions(axiom_engine_core PUBLIC 
    ASMJIT_STATIC
    AXIOM_ENGINE_EXPORTS
)

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

target_link_libraries(axiom_engine_runtime PUBLIC
    axiom_engine_core
    Python::Python
)

# ─── AXIOM Engine Compute ──────────────────────────────────────────
add_library(axiom_engine_compute STATIC
    compute/eigen_engine.cpp
    compute/symbolic_engine.cpp
    compute/statistics_engine.cpp
    compute/plot_engine.cpp
    compute/mantis_heuristic.cpp
    compute/zenith_jit.cpp
    compute/sleef_wrapper.cpp
    compute/ast_analyzer.cpp
)
enforce_zenith_pillars(axiom_engine_compute)

target_link_libraries(axiom_engine_compute PUBLIC
    axiom_engine_core
    axiom_engine_runtime
    Eigen3::Eigen
    asmjit::asmjit
)

target_compile_definitions(axiom_engine_compute PUBLIC ASMJIT_STATIC)

# ─── AXIOM Engine IPC ──────────────────────────────────────────────
add_library(axiom_engine_ipc STATIC
    ipc/telemetry.cpp
    ipc/ingress.cpp
)
enforce_zenith_pillars(axiom_engine_ipc)

target_link_libraries(axiom_engine_ipc PUBLIC
    axiom_engine_core
    axiom_engine_runtime
)

if(UNIX AND NOT APPLE)
    target_link_libraries(axiom_engine_ipc PRIVATE rt)
endif()

# ─── AXIOM Engine API ──────────────────────────────────────────────
add_library(axiom_engine_api STATIC
    core/dynamic_calc.cpp
    api/algebraic_parser.cpp
    compute/node_dispatcher.cpp
    api/axiom_bridge.cpp
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
    dispatch/execution_orchestrator.cpp
    dispatch/execution_orchestrator_capi.cpp
)
enforce_zenith_pillars(axiom_engine_api)

# [EXCEPTION]: nanobind requires RTTI and Exceptions to function correctly
set_source_files_properties(api/python_bindings.cpp api/nanobind_interface.cpp PROPERTIES COMPILE_FLAGS "-fexceptions -frtti")
if(MSVC)
    set_source_files_properties(api/python_bindings.cpp api/nanobind_interface.cpp PROPERTIES COMPILE_FLAGS "/EHsc /GR")
endif()

target_link_libraries(axiom_engine_api PUBLIC
    axiom_engine_core
    axiom_engine_runtime
    axiom_engine_compute
    axiom_engine_ipc
    Python::Python
    simdjson
    sleef
)

target_include_directories(axiom_engine_api PRIVATE
    ${nanobind_SOURCE_DIR}/include
)

if(WIN32)
    target_link_libraries(axiom_engine_api PUBLIC bcrypt)
endif()

# Full Engine Alias
add_library(axiom_engine INTERFACE)
target_link_libraries(axiom_engine INTERFACE axiom_engine_api)

# ─── AXIOM Sandbox Worker ─────────────────────────────────────────
add_executable(axiom_worker core/native_worker_main.cpp)
target_link_libraries(axiom_worker PRIVATE 
    axiom_engine_api
    axiom_engine_compute
    axiom_engine_ipc
    axiom_engine_core
    Threads::Threads
)
