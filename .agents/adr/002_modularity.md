# ADR 002: Modular Sub-Library Structure

- **Date**: 2026-03-23
- **Author**: Antigravity (Architect)
- **Status**: Accepted

## Context

The `axiom_engine` library has become a monolith containing everything from low-level allocators to high-level Python bindings. This makes it difficult to maintain, test in isolation, and enforce architectural boundaries.

## Decision

We split `axiom_engine` into several internal static libraries:

1.  **axiom_common**: Core utilities (ArenaAllocator, SIMD, String helpers).
2.  **axiom_telemetry**: Performance monitoring and RDTSC timing.
3.  **axiom_pluto**: High-performance engine logic (Mantis, Pluto controller).
4.  **axiom_io**: Bridges, parsers, and external integrations (Python, Plots).

These libraries will be linked into the final `axiom_engine` shared library.

## Consequences

- **Improved Decoupling**: We can now enforce that `axiom_common` does not depend on higher-level logic.
- **Faster Incremental Builds**: Changes in high-level bridges won't require re-linking core logic.
- **Easier Testing**: Unit tests can now link against specific sub-libraries.
