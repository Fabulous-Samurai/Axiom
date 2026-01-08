# AXIOM PRO v3.0 Optimization Addendum

This addendum documents the newly added Fast Render mode and FPS status monitoring to improve interactive plotting performance.

## Fast Render Mode
- Purpose: Improve interactive responsiveness with very large datasets (>100k points).
- Technique: Data decimation to a target budget (~5k points) for display while maintaining overall shape.
- Behavior: When enabled, 2D plotting paths use decimated data and fixed axis limits to avoid repeated autoscaling.
- Toggle: Performance toolbar → "Fast Render" (⚡).

## FPS Status Monitor
- Implementation: A draw-event hook measures frame intervals using high-resolution timers.
- Display: Status bar shows `Render: <ms> | FPS: <value>` after each draw.
- Notes: `draw_idle()` schedules rendering; the FPS reflects actual render cadence.

## Quick Benchmark (Agg backend)
A simple harness compares full vs decimated draw times for 1M points:

```
python -m tests.performance.test_fast_render_mode
```
Expected output (varies by machine):
```
Full draw: 1200.5 ms
Decimated draw: 45.3 ms
Speedup: 26.5x
Points: full=1000000, decimated=5000
```

## Guidance
- Enable Fast Render when exploring or scrubbing large series; disable for final publication-quality exports.
- Use axis zoom/pan to focus regions; Fast Render respects current x/y ranges.
- For streaming or frequent updates, prefer `set_data()` on a persistent line plus `draw_idle()` (already in use).
