## 2025-01-20 - [Zero-Allocation Float Parsing]
**Learning:** `std::stod` overhead with temporary `std::string` allocation is a major performance and Pillar 5 (Zero-Exception) bottleneck for parser-heavy workloads.
**Action:** Replace `std::stod` inside `FastParseDouble` with C++17 `std::from_chars` and exception-free `std::strtod` fallback using local stack buffers to completely eliminate `std::string` allocations.
