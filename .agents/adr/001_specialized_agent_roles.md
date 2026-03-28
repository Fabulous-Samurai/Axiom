# ADR 001: Specialized Agent Roles (Zenith Expanded)

- **Date**: 2026-03-28 (Updated)
- **Author**: Antigravity (Architect) / Gemini CLI
- **Status**: Accepted

## Context

The AXIOM Zenith engine's complexity requires more granular role boundaries to prevent overlapping efforts and ensure strict adherence to Zenith Pillars across different domains (Security, UI, Performance, etc.).

## Decision

We define nine specialized roles. Every agent MUST identify as one of these roles or be assigned one. Detailed specifications for each role are maintained in `.agents/roles/*.md`.

### Core Roles
1.  **Orchestrator (Lead)**: Strategy, task delegation, and final "GO/NO-GO" authority.
2.  **Architect**: High-level design, ADR management, and structural integrity.
3.  **Engine (Core)**: Performance logic, assembly optimization, and Zero-Allocation enforcement.
4.  **Security Hardener**: Phase H (Enclaves), memory isolation, and TLA+ security modeling.

### Support & Validation Roles
5.  **QA/SDET**: Testing, adversarial simulations, and CI/CD stability.
6.  **Performance Auditor**: RDTSC cycle monitoring, jitter analysis, and bottleneck detection.
7.  **UI/Bridge Specialist**: AXUI integration, zero-copy data transfer, and theme resolution.
8.  **DevSecOps**: CI/CD automation, environment bootstrapping, and supply chain security.
9.  **Janitor**: Refactoring, linting, and documentation maintenance.

> [!IMPORTANT]
> **Universal Specialist**: **Gemini CLI** is authorized to operate across ALL roles and can transition between them as requested by the user.

## Consequences

- **Role-Based Context**: Agents can be "booted" with a specific role file to narrow their focus.
- **Improved Accountability**: Log entries will clearly reflect which domain an agent operated in.
- **Granular Protocol Enforcement**: Each role has specific Zenith Pillar priorities.
