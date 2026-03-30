# Learn From Sonar: Preventing Recurrent Zenith Pillar Violations

This document summarizes common pitfalls identified in past SonarCloud reports and provides guidelines to prevent their recurrence. Adhering to these principles is crucial for maintaining AXIOM Zenith's "Iron Will" architecture.

## 1. Zero-Allocation (Pillar 1) Violations
**Problem:** Accidental use of `std::vector`, `std::string`, `std::map`, `new`, `malloc` in performance-critical hot-paths.
**Lesson Learned:** These types introduce heap allocations, leading to non-deterministic latency and potential fragmentation.
**Guideline:**
- Use `AXIOM::FixedVector` for dynamic arrays.
- Use `std::string_view` or `AXIOM::FixedString` for string manipulation.
- Use `AXIOM::ArenaAllocator` for any dynamic object allocation within a defined lifecycle.
- Implement specialized, lock-free data structures (e.g., SPSC queues).
- **Radar Keyword:** `std::vector`, `std::string`, `std::map`, `new `, `malloc(`, `delete `, `free(`

## 2. Zero-Exception (Pillar 5) Violations
**Problem:** Usage of `throw`, `try`, `catch`, `dynamic_cast`, `typeid` in core engine modules.
**Lesson Learned:** Exceptions introduce non-deterministic control flow and performance overhead. RTTI breaks binary size and cache predictability.
**Guideline:**
- Use `std::optional` or `std::expected` (C++23) for error propagation.
- Use `static_cast` with `std::variant` dispatch where polymorphism is required.
- **Radar Keyword:** `throw `, `try \{`, `catch \(`, `dynamic_cast`, `typeid(`

## 3. Buffer Overflow & Unsafe C-Style String Handling
**Problem:** Use of `strncpy`, `strlen`, `strcat` without explicit bounds checking.
**Lesson Learned:** These functions are notorious for causing security vulnerabilities (buffer overflows).
**Guideline:**
- Prefer C++ alternatives like `std::string_view`, `AXIOM::FixedString`, or `std::string::copy` with explicit length.
- If C-style functions are unavoidable, always pair them with `_s` variants (on Windows) or manually implement bounds checks.
- **Radar Keyword:** `strncpy`, `strlen(`, `strcat`

## 4. Cognitive Complexity & Code Smells
**Problem:** Functions with high cognitive complexity, deeply nested loops/conditionals, and lack of `explicit` constructors.
**Lesson Learned:** Complex code is harder to test, maintain, and reason about, increasing the likelihood of bugs. Implicit conversions can lead to subtle logic errors.
**Guideline:**
- Break down large functions into smaller, single-responsibility helper methods.
- Use structured bindings (C++17) to improve readability of tuple/pair unpacking.
- Mark single-argument constructors as `explicit` unless an implicit conversion is explicitly desired (rare in Zenith).
- **Radar Keyword:** `if \(.*\)\s*\{\s*if \(.*\)\s*\{`, `for \(.*\)\s*\{\s*for \(.*\)\s*\{`, Missing `explicit`

## 5. Security (DoS & Weak Crypto)
**Problem:** Inefficient regex patterns, weak random number generators, running containers as `root`.
**Lesson Learned:** Performance-critical systems are vulnerable to DoS if regex engines can be forced into polynomial time. Weak PRNGs are insufficient for any security-sensitive operation. Running as `root` increases attack surface.
**Guideline:**
- Profile regex patterns for worst-case performance; use `re2` or other optimized libraries.
- For non-security PRNG, use `std::mt19937`. For security-sensitive, use `std::random_device`.
- Always run Docker containers with a non-root user.
- **Radar Keyword:** `rand\(`, `regex.*(\(.*\){2,})`, `FROM (ubuntu|fedora)` without subsequent `USER`

## 6. Linkage & Dependency Management (Structural Integrity)
**Problem:** Circular dependencies between modules (e.g., `api` calling `compute` while `compute` depends on `api`) and Windows entry point (`WinMain`) conflicts in tests.
**Lesson Learned:** Circular dependencies prevent static linking and cause "symbol not found" errors. Windows subsystems (GUI vs. Console) can lead to silent crashes or linker failures if not explicitly forced.
**Guideline:**
- Consolidate highly interdependent modules into a single library (e.g., `axiom_engine_lib`) to break cycles.
- Use `ALIAS` in CMake to support legacy library names while transitioning to consolidated blocks.
- Force `mainCRTStartup` and `-mconsole` for all test/benchmark targets on Windows to ensure consistent entry points.
- **Radar Keyword:** `ALIAS axiom_engine`, `mainCRTStartup`, `-mconsole`

---
**Last Updated:** 2026-03-30
**Authored by:** Orchestrator (Gemini CLI)
