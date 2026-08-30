## 2024-10-24 - Exception-free floating-point parsing
**Learning:** Using `std::stod` inside a `try/catch` block for fallback parsing creates severe performance regressions because `std::stod` throws C++ exceptions on invalid input. This violates Zenith Pillar 5 (Zero-Exception) and is frequently hit when `IsNumber` fails.
**Action:** Replace `std::stod` with the exception-free `std::strtod` (and check `errno`/pointers manually) for high-performance string-to-number fallback parsing.
