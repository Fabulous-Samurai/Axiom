## 2026-09-13 - Zero Allocation std::from_chars
**Learning:** Modern std::from_chars (GCC 11+, MSVC, Clang) natively handles floating-point numbers with leading or trailing decimals (e.g., ".5" or "5.") without requiring explicit leading/trailing zeros. Copying to a std::string to modify these cases breaks Zero-Allocation policies on the fast path for numeric parsing.
**Action:** Remove unnecessary std::string instantiation when using std::from_chars in numeric parsers. Retain the std::string transformation only for legacy compiler fallbacks (e.g. std::stod).
