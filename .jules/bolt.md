## 2024-08-25 - std::from_chars Zero-Allocation Opt
**Learning:** `std::from_chars` can handle strings starting with `.` natively (e.g., `.5`), and strings ending with `.` natively (e.g., `5.`), avoiding the need for `std::string` heap allocations. It does not handle leading `+` signs, but we can advance the pointer to bypass `+` before calling `std::from_chars`.
**Action:** Use an optimistic fast-path with `std::from_chars` to avoid `std::string` allocations in `FastParseDouble`.
