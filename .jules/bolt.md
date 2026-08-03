## 2026-08-03 - Zero-Allocation in Parsing Fast Paths
**Learning:** Even modern C++ features like `std::from_chars` can be bottlenecked if they are wrapped in defensive logic that forces a `std::string` allocation (e.g. `std::string str(sv)`) just to handle rare edge cases (like trailing/leading decimal points).
**Action:** When working with `std::string_view`, always check if the edge-case fallback is actually needed before performing a string allocation.
