## YYYY-MM-DD - Zero-Allocation in FastParseDouble
**Learning:** Creating temporary std::string copies to pad zeroes (e.g., '.5' -> '0.5') before std::from_chars is unnecessary overhead and violates zero-allocation constraints, because std::from_chars natively supports these formats.
**Action:** Rely on std::from_chars directly to avoid dynamic allocations on critical paths.
