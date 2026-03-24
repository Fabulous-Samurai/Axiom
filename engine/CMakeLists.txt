# ─── AXIOM Engine Core ─────────────────────────────────────────────
add_library(axiom_engine_core STATIC
    core/arena_allocator.cpp
    core/daemon_engine.cpp
    core/pmu_orchestrator.cpp
    core/pluto_controller.cpp
    core/crash_dump.cpp
    core/cognitive_commander.cpp
    core/cognitive_ecu.cpp
    core/dynamic_calc.cpp
    core/unit_manager.cpp
    core/secure_vault.cpp
    core/string_helpers.cpp
    core/cpu_optimization.cpp
)

target_include_directories(axiom_engine_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(axiom_engine_core PUBLIC
    mimalloc
    Threads::Threads
)

# ─── AXIOM Engine Compute ──────────────────────────────────────────
add_library(axiom_engine_compute STATIC
    compute/eigen_engine.cpp
    compute/symbolic_engine.cpp
    compute/statistics_engine.cpp
    compute/plot_engine.cpp
    compute/mantis_heuristic.cpp
    compute/zenith_jit.cpp
)

target_link_libraries(axiom_engine_compute PUBLIC
    axiom_engine_core
    Eigen3::Eigen
    asmjit::asmjit
)

# ─── AXIOM Engine IPC ──────────────────────────────────────────────
add_library(axiom_engine_ipc STATIC
    ipc/telemetry.cpp
    ipc/ingress.cpp
)

target_link_libraries(axiom_engine_ipc PUBLIC
    axiom_engine_core
)

# ─── AXIOM Engine API ──────────────────────────────────────────────
add_library(axiom_engine_api STATIC
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
    api/kensui_bridge.cpp
)

target_link_libraries(axiom_engine_api PUBLIC
    axiom_engine_core
    axiom_engine_compute
    axiom_engine_ipc
    Python::Python
    simdjson
)

if(WIN32)
    target_link_libraries(axiom_engine_api PUBLIC bcrypt)
endif()

# Full Engine Alias
add_library(axiom_engine INTERFACE)
target_link_libraries(axiom_engine INTERFACE axiom_engine_api)
