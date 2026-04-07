
## 2024-05-18 - [Optimize python string parsing]
**Learning:** Found an unnecessary instantiation and allocation loop for replacing python imports (`std::string pattern` and `std::string replacement` in a loop). By prepending the instantiation with a `if (processed.find(func) != std::string::npos)` block, we avoid unnecessary memory allocation overhead if the string doesn't contain the function.
**Action:** Always check if string pattern matches before instantiating new strings to replace it.
