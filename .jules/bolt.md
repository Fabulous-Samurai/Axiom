## 2024-05-15 - FastParseDouble Temporary String Allocation
**Learning:** `std::from_chars` perfectly handles leading (`.5`) and trailing (`5.`) decimal representations natively without string manipulation. Pad operations requiring `std::string` allocations are completely unnecessary overhead.
**Action:** Removed the temporary string padding and instead relied on `std::from_chars`' native support, drastically minimizing heap allocations per scalar parse in `FastParseDouble`.
