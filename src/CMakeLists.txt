# [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md

set(AXIOM_CORE_SOURCES
    algebraic_parser.cpp
    arena_allocator.cpp
    cognitive_ecu.cpp
    cpu_optimization.cpp
    crash_dump.cpp
    daemon_engine.cpp
    dynamic_calc.cpp
    eigen_engine.cpp
    ingress.cpp
    kensui_bridge.cpp
    linear_system_parser.cpp
    mantis_heuristic.cpp
    plot_engine.cpp
    plot_parser.cpp
    pluto_controller.cpp
    pmu_orchestrator.cpp
    python_engine.cpp
    python_parser.cpp
    python_repl.cpp
    secure_vault.cpp
    statistics_engine.cpp
    statistics_parser.cpp
    string_helpers.cpp
    symbolic_engine.cpp
    symbolic_parser.cpp
    telemetry.cpp
    unit_manager.cpp
    unit_parser.cpp
    vulkan_expressway.cpp
    zenith_jit.cpp
)

if(AXIOM_ENABLE_NANOBIND)
    list(APPEND AXIOM_CORE_SOURCES nanobind_interface.cpp python_bindings.cpp)
    add_compile_definitions(ENABLE_NANOBIND)
endif()

# Explicitly ensure AXIOM_CORE_SOURCES is not empty before creating the library
list(LENGTH AXIOM_CORE_SOURCES SOURCES_LEN)
if(SOURCES_LEN EQUAL 0)
    message(FATAL_ERROR "AXIOM_CORE_SOURCES is unexpectedly empty!")
endif()

add_library(axiom_engine SHARED ${AXIOM_CORE_SOURCES})

target_include_directories(axiom_engine PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_include_directories(axiom_engine PRIVATE ${Python_INCLUDE_DIRS})

target_compile_definitions(axiom_engine PRIVATE AXIOM_ENGINE_EXPORTS)

target_link_libraries(axiom_engine 
    PUBLIC 
        Python::Python 
    PRIVATE 
        asmjit::asmjit 
        Threads::Threads 
        ${AXIOM_EXTRA_LIBS}
)

if(WIN32)
    target_link_libraries(axiom_engine PUBLIC bcrypt)
endif()
