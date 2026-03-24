# ADR 003: Performance Regression Guardrails

- **Date**: 2026-03-23
- **Author**: Antigravity (Architect)
- **Status**: Accepted

## Context

Performance is the primary requirement for AXIOM Zenith. However, manual benchmarking is infrequent, and regressions can easily be introduced into the `Evaluate` or `Ingress` hot-paths without detection.

## Decision

We implement an automated performance regression check in the Linux CI/CD pipeline:
1.  **Google Benchmark Output**: The `axiom_benchmark` target is executed, and results are exported in JSON format.
2.  **Regression Script**: A Python script (`scripts/perf/check_perf_regression.py`) compares these results against a baseline (`.agents/perf_baseline_linux.json`).
3.  **Threshold**: A default threshold of 10% is set. If any benchmark exceeds its baseline by more than this amount, the check fails.
4.  **Baseline Management**: If no baseline exists, the current run is saved as the new baseline.

## Consequences

- **Early Detection**: Performance degradation is caught within minutes of a PR being opened.
- **Data-Driven Decisions**: Performance impact is now quantified and visible in CI logs.
- **CI Dependency**: The Linux CI now requires a stable CPU environment for consistent results (handled by `-continue-on-error` for now until stable hardware is available).
