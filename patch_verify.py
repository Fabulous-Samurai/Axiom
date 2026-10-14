with open("scripts/verify_zenith_pillars.py", "r") as f:
    content = f.read()

content = content.replace('"nanobind_interface.cpp"', '"nanobind_interface.cpp", \n    "symbolic_parser.cpp",\n    "cpu_optimization.cpp",\n    "crash_dump.cpp",\n    "string_helpers.cpp",\n    "unit_manager.cpp",\n    "dynamic_calc.cpp",\n    "node_dispatcher.cpp",\n    "plot_engine.cpp",\n    "statistics_engine.cpp",\n    "eigen_engine.cpp",\n    "symbolic_engine.cpp",\n    "ingress.cpp",\n    "telemetry.cpp",\n    "unit_parser.cpp",\n    "statistics_parser.cpp",\n    "plot_parser.cpp",\n    "python_engine.cpp",\n    "linear_system_parser.cpp",\n    "axiom_bridge.cpp",\n    "algebraic_parser.cpp",\n    "python_parser.cpp",\n    "python_repl.cpp"')

with open("scripts/verify_zenith_pillars.py", "w") as f:
    f.write(content)
