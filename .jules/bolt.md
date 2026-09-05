## 2024-05-15 - FastParseDouble Exception Elimination
**Learning:** Using std::stod inside a try/catch block as a fallback for missing std::from_chars in FastParseDouble creates a severe performance bottleneck. IsNumber calls this frequently, and when a string isn't a number, it triggers a C++ exception which is extremely expensive in latency and violates the Zero-Exception (Pillar 5) mandate.
**Action:** Use the C-style std::strtod with proper errno and pointer validation instead of std::stod/try/catch for exception-free, high-performance string-to-number fallback paths.
