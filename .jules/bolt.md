## 2024-03-24 - Zero-Allocation in FastParseDouble
**Learning:** Found an unnecessary heap allocation in `engine/include/string_helpers.h` (`Utils::FastParseDouble`) where `std::string` was being created just to pad zeros to strings like `".5"` or `"5."`. C++17 `std::from_chars` and `std::strtod` already handle these natively.
**Action:** Removed the `std::string` allocation, falling back directly to `std::from_chars` on `std::string_view` (or `std::strtod` on a stack-allocated buffer for older compilers), satisfying the strict Zero-Allocation mandate for core hot paths.
