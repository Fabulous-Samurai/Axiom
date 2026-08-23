## 2024-08-23 - Optimizing FastParseDouble to comply with Zenith Pillar 1
**Learning:** Using std::from_chars does not require unconditionally instantiating a std::string from a std::string_view, which violates the zero-allocation Zenith Pillar. std::from_chars natively supports general number formatting (including leading/trailing decimal points).
**Action:** Implement an optimistic zero-allocation fast-path using std::from_chars directly on the std::string_view, and only fall back to allocating std::string (for edge cases like leading +) if the fast-path fails.
