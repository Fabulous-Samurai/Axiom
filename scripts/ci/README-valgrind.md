# Valgrind Analysis

This document explains how to use Valgrind for memory analysis in the AXIOM Engine project.

## Steps

1. Build the project:
   ```bash
   cmake --build build --config Debug
   ```

2. Run Valgrind:
   ```bash
   valgrind ./build/tests
   ```

3. Analyze the output for memory leaks and errors.

## Notes

- Ensure Valgrind is installed on your system.
- Use the `--leak-check=full` option for detailed leak reports.