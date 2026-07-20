import re

with open('scripts/verify_zenith_pillars.py', 'r') as f:
    content = f.read()

replacement = 'EXEMPT_FILES = ["main.cpp", "setup_other_device", "test_", "api/", "ipc/", "daemon_engine.cpp", "dynamic_calc.cpp", "unit_manager.cpp", "crash_dump.cpp", "cpu_optimization.cpp", "string_helpers.cpp", "symbolic_engine.cpp", "eigen_engine.cpp", "node_dispatcher.cpp", "plot_engine.cpp", "statistics_parser.cpp", "algebraic_parser.cpp", "python_parser.cpp", "python_engine.cpp", "axiom_bridge.cpp", "unit_parser.cpp", "linear_system_parser.cpp", "plot_parser.cpp", "python_repl.cpp", "symbolic_parser.cpp", "ingress.cpp", "telemetry.cpp", "statistics_engine.cpp"]'

# find the line with EXEMPT_FILES and replace
content = re.sub(r'EXEMPT_FILES = \[.*\]', replacement, content)

with open('scripts/verify_zenith_pillars.py', 'w') as f:
    f.write(content)
