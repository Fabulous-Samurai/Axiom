## 2024-10-22 - Zero-Allocation Mode Calculation
**Learning:** In AXIOM Engine, using std::map or std::unordered_map for frequency analysis violates Zenith Pillar 1 (Zero-Allocation) as it causes constant memory allocations, significantly reducing performance.
**Action:** Always favor an allocation-free copy sort (e.g., std::ranges::sort) and linearly iterate to count consecutive identical elements to calculate the mode without allocating map nodes.
