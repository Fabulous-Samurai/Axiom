## 2024-05-18 - C++ Enums vs std::array size hardcoding
**Learning:** Hardcoding `kModeSlots` to `7` in `dynamic_calc.h` when `CalculationMode` enum has 7 entries is brittle. If someone adds a mode, it could cause silent out-of-bounds or missed parser initialization.
**Action:** No direct optimization here, but good to note. Looking for true performance bottlenecks.

## 2024-05-18 - Pass-by-value rvalue modification in BoxPlot
**Learning:** In C++, changing `const Vector&` parameter signatures to pass-by-value `Vector` allows in-place mutation (like sorting) and avoids heap allocations when callers pass rvalues, saving an extra heap allocation compared to copy-constructing inside the method.
**Action:** In `PlotEngine::BoxPlot`, change the signature to pass-by-value `Vector data` and use it directly instead of copying `Vector sorted = data;`.

## 2024-05-18 - NaN sort crash
**Learning:** When sorting floating-point arrays in C++ (e.g., `std::sort`), prevent `NaN` values from violating strict weak ordering, which can cause crashes. Use `std::isnan` to explicitly check for `NaN` rather than `!std::isfinite`, because `Infinity` is safely orderable.
**Action:** Change `std::sort(sorted.begin(), sorted.end())` to `std::sort(data.begin(), data.end(), [](double a, double b) { return std::isnan(a) ? false : (std::isnan(b) ? true : a < b); });` or similar logic.
Wait, if it's NaN vs NaN, it should return false to maintain strict weak ordering.
