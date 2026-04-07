## 2024-05-18 - [Optimize python string parsing]
**Learning:** Found an unnecessary instantiation and allocation loop for replacing python imports (`std::string pattern` and `std::string replacement` in a loop). By prepending the instantiation with a `if (processed.find(func) != std::string::npos)` block, we avoid unnecessary memory allocation overhead if the string doesn't contain the function.
**Action:** Always check if string pattern matches before instantiating new strings to replace it.

## 2024-05-18 - [Fix SonarCloud code duplication]
**Learning:** SonarCloud complains about duplication when identical code structures (like nested loops and if blocks for string replacement) are copy-pasted across multiple functions (e.g., NumPy/SciPy modes).
**Action:** Extract repetitive string replacement logic into a `static void` helper function (e.g., `ReplaceFunctionCalls`) that takes a list of targets and specific prefixes/suffixes.
