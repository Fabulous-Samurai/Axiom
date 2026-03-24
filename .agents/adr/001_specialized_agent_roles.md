# ADR 001: Specialized Agent Roles

- **Date**: 2026-03-23
- **Author**: Antigravity (Architect)
- **Status**: Accepted

## Context

The AXIOM Zenith engine is growing in complexity. Multiple agents working on the same codebase without clear role boundaries can lead to fragmented code quality and overlapping efforts.

## Decision

We define four specialized roles for AI agents. Every agent MUST identify as one of these roles when registering in `AGENT_ACTIVITY_LOG.md`:

1.  **Architect**: Focuses on design, `implementation_plan.md`, and high-level structure.
2.  **Engine**: Focuses on core performance logic, assembly optimization, and Zenith pillars (zero-allocation).
3.  **QA/SDET**: Focuses on writing tests, adversarial simulations, and CI/CD stability.
4.  **Janitor**: Focuses on refactoring, linting, dependency management, and documentation.
5.  **DevSecOps**: Focuses on CLI tools, CI/CD pipeline automation, and environment bootstrapping.

> [!IMPORTANT]
> **Universal Specialist**: **Gemini CLI** is designated as a Full-Spectrum Agent and is authorized to operate across ALL roles as needed to support the primary user workflow.

## Consequences

- Agents will have clearer focus and less overlap.
- Code reviews (if done by agents) will be more targeted.
- The project owner can assign specific tasks to specific agents based on these roles.
