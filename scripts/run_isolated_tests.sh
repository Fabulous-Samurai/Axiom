#!/bin/bash

# Ensure the script is run with root privileges
echo "This script requires root privileges to set CPU affinity."
if [[ $EUID -ne 0 ]]; then
   echo "Please run as root."
   exit 1
fi

# Define the CPU core to isolate and pin tasks to
ISOLATED_CORE=3

# Isolate the core using isolcpus (requires kernel boot parameter)
echo "Ensure kernel boot parameter 'isolcpus=$ISOLATED_CORE' is set."

# Run the benchmark on the isolated core using taskset
echo "Running benchmark on isolated core $ISOLATED_CORE..."
taskset -c $ISOLATED_CORE ./build/axiom_benchmark.exe

# Run the tests on the isolated core using taskset
echo "Running tests on isolated core $ISOLATED_CORE..."
taskset -c $ISOLATED_CORE ./build/run_tests.exe

# Provide a warning about OS jitter
echo "Note: Running benchmarks or tests on non-isolated cores may introduce OS jitter."
echo "Ensure background services and Chrome are not running on the isolated core."
