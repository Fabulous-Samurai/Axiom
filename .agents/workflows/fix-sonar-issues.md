---
description: How to fix SonarQube code smells in the AXIOM Zenith Engine
---

# Fix SonarQube Code Smells (Zenith Edition)

## Zenith Pillars
- **Pillar 1 (Zero-Allocation)**: Immediate fix required for any `S110` (Dynamic allocation in hotspots).
- **Pillar 5 (Zero-Exception)**: Prioritize `S112` (Generic exceptions) and `S5917` (Missing noexcept).

## Purpose
Systematic remediation of architectural flaws and code smells, with a focus on维持 Zenith performance standards.

## Preconditions
- SonarCloud or local Sonar Scanner access.
- `output/files_with_issues.json` produced by CI.

## Workflow Rules (Zenith)
1. **Rule 1 (Pillar Enforcement)**: A code smell that violates a Zenith Pillar is a **Blocker**.
2. **Rule 2 (No Exceptions)**: Replacing an exception with a log-and-continue is NOT a fix. Use `std::optional` or `std::expected`.
3. **Rule 3 (Inlining)**: S134 (Nested loops) must be resolved via inlined pipeline helper functions.

## Remediation Phases
- **Phases 1-12 (Legacy)**: Standard code smells, threading fixes, and early-stage HPC optimizations.
- **Phase A (Remediation)**: Elimination of VTable dispatch and Vector sprawl.
- **Phases E-H (Zenith)**: Kernel-bypass, SLM memory pinning, and Secure Enclave code quality.

## Steps
1. Identify Zenith Pillar violations in the Sonar report.
2. Fix Pillar violations first using `ArenaAllocator` or `noexcept` patterns.
3. Apply standard Sonar remediations in logical batches.

## Validation
- `build/run_tests.exe` must pass with zero regressions.
- `axiom_benchmark.exe` must show NO latency degradation after the fix.

## Rollback
- Revert if a "logical fix" causes a >2% performance drop in RDTSC cycle counts.
