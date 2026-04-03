## 2024-03-20 - [Optimize Python Parser String Construction]
**Learning:** Found that `PythonParser` in `engine/api/python_parser.cpp` used inefficient string concatenation via `operator+` inside hot loops. String append operations triggered unnecessary allocations and temporaries.
**Action:** Replaced `operator+` with explicit `std::string::reserve()` and chained `.append()` to avoid allocations. This fits the "Zero-Allocation" ethos while remaining performant.
