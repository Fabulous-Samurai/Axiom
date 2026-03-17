---
description: How to run TLA+ formal verification for the Mantis A* engine and Secure Vault
---

# TLA+ Formal Verification (Zenith Edition)

## Zenith Pillars
- **Pillar 3 (Determinizm)**: TLA+ models prove that the Mantis state machine is deterministic and free of state-skip conditions.
- **Phase H (Security)**: `MantisSecureVault.tla` proves memory isolation contracts.

## Purpose
Run and validate formal verification for Mantis solver models and security enclaves.

## Preconditions
- Java Runtime Environment (JRE)
- `tla2tools.jar` in repo root or `TLC_JAR` environment variable.

## Models
| Model | Zenith Scope | Goal |
|---|---|---|
| `MantisAStarCorrectness` | Core Solver | Closed-set monotonicity |
| `MantisHeuristicDispatch` | Performance | FMA3/scalar equivalence |
| **`MantisSecureVault`** | Phase H | Memory Sealing/Unsealing safety |

## Steps

1. **Structural Verification**
   ```bash
   python -m pytest tests/unit/test_tla_specs.py -v
   ```

2. **Phase H Security Model Check**
   ```bash
   java -jar tla2tools.jar -config formal/tla/MantisSecureVault.cfg formal/tla/MantisSecureVault.tla
   ```

3. **Core Solver Model Check**
   ```bash
   java -jar tla2tools.jar -config formal/tla/MantisAStarCorrectness.cfg formal/tla/MantisAStarCorrectness.tla
   ```

## Validation
- TLC reports `No error has been found.`
- All invariants (`Invariant_SafeMemoryAccess`, `Invariant_SealIntegrity`) hold.

## Rollback
- Revert any spec that introduces a safety violation or model-checking deadlock.
