## 2024-05-24 - Zero-Exception Rule & Fast-Path Parsing Optimization
**Learning:** Using `std::stod` with a `try/catch` block for fallback path string-to-number parsing creates severe performance regressions because `std::stod` throws exceptions on invalid inputs. The codebase strictly mandates 'Zenith Pillar 5: Zero-Exception & No RTTI' in its `AGENTS.md` equivalent, meaning `try/catch` is forbidden in Core modules.
**Action:** Replace `std::stod` inside a `try/catch` block with `std::strtod` which provides an exception-free, high-performance fallback for floating-point parsing.
