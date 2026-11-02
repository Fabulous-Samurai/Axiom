## 2024-05-24 - Zero-Allocation Mode Calculation
**Learning:** `std::map` performs poorly for frequency analysis due to constant heap allocations. Changing `const Vector&` parameter signatures to pass-by-value `Vector` allows in-place mutation (like sorting) and avoids heap allocations when callers pass rvalues.
**Action:** Always favor an allocation-free copy sort (e.g., `std::ranges::sort` on a `FixedVector` or regular `Vector`) and linearly iterate to count consecutive identical elements. Handle NaN sorting carefully to avoid violating strict weak ordering.
