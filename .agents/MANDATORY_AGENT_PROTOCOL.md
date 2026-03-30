# MANDATORY AGENT PROTOCOL v3.4 (The Synapse - Explicit Edition)

> [!MANDATORY]
> **OPERATION: ETERNAL VIGILANCE (Sürekli Uyanıklık - Self-Learning Feedback Loop)**
>
> This protocol mandates that all agents actively participate in the continuous improvement of AXIOM Zenith's architecture. Before initiating ANY code modification, you MUST perform the following three steps:
>
> 1.  **READ & INTERNALIZE Guidelines (`.agents/sync_learn_from_sonar.md`):**
>     *   **Purpose:** To prevent the recurrence of previously identified architectural violations, code smells, and security hotspots. This document is our collective memory of past challenges and their Zenith-compliant solutions.
>     *   **Action:** Thoroughly read and fully comprehend the guidelines. Understand the "why" behind each rule, connecting it to Zenith Pillars (Zero-Allocation, Zero-Exception, Determinism).
>
> 2.  **EXECUTE LOCAL RADAR (`.agents/local_radar_pillars.py`):**
>     *   **Purpose:** To proactively identify anti-patterns and potential violations *before* they enter the build pipeline. This is your personal architectural "sanity check."
>     *   **Action:** Run the script from your project root: `python3 .agents/local_radar_pillars.py`
>     *   **Result:** The script will scan your changes (and the surrounding context) for common pitfalls. Address any reported issues immediately.
>
> 3.  **UPDATE & EXPAND (The Feedback Loop - Contribute to Pluto's Intelligence):**
>     *   **Purpose:** To transform individual agent learning into collective project knowledge. This is crucial for **Pluto AI's future self-healing and code generation capabilities.**
>     *   **Action:** If, during your task, you discover a **new, unlisted type of architectural violation, recurring bug, or anti-pattern**, you are obligated to:
>         *   **a) Document the Lesson:** Add a new, detailed section to `.agents/sync_learn_from_sonar.md` explaining the problem, its impact on Zenith Pillars, and the correct "Zenith-compliant" solution.
>         *   **b) Enhance the Radar:** Add a new, robust regex pattern to `.agents/local_radar_pillars.py` to automatically detect this new issue in future scans. Ensure the regex is precise.
>     *   **Benefit:** Every new pattern captured here directly feeds Pluto's training data, making it a smarter, more reliable code architect.
>
> This protocol is non-negotiable. It ensures our system is not only compliant but also **self-learning and self-healing**, continuously improving its architectural integrity.

## 1. Compliance Mandates
- **Zenith Pillars**: All code must comply with the Zenith Pillars (Zero Allocation, No Exceptions, No RTTI).
- **Namespacing**: All new classes/structs MUST be within `namespace AXIOM`.
- **Modularity**: New components must be registered in `src/CMakeLists.txt`.

## 2. Pre-Flight Checklist
1. **CHECK `.agents/TASK_ORCHESTRATION.md` for your active DIRECTIVE (MANDATORY).**
2. Check `.agents/PROJECT_HEALTH.md` for active issues.
3. Review `.agents/AGENT_ACTIVITY_LOG.md` for recent changes.
4. Verify your role in `.agents/global_agent_process_protocol.md`.

## 3. Communication
- All major architectural shifts must be documented in a new ADR in `.agents/adr/`.
