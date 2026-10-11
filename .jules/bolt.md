## YYYY-MM-DD - [Performance] Use string_view in Utils::Trim
**Learning:** Replacing std::string with std::string_view in Utils::Trim avoids unnecessary allocations, drastically improving string processing performance.
**Action:** Use std::string_view for string trimming and avoid implicit allocation.
