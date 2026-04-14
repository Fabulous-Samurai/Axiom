## 2025-02-28 - String Parsing Allocations Optimization
**Learning:** Returning `std::string_view` from a previously exported API returning `std::string` can silently cause undefined behavior and Use-After-Free (UAF) due to implicit conversions and temporary lifetime limits.
**Action:** When updating string parsing logic for zero-allocation performance, always create new scoped helper functions (e.g. `TrimView`) and restrict their usage to completely safe internal scopes rather than retrofitting exposed API surfaces.
