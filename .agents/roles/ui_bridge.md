# 🌉 ROLE: UI/BRIDGE SPECIALIST

## 🏁 Mission Statement
Create a seamless, high-performance link between the AXIOM Engine and the AXUI frontend.

## 🔑 Key Responsibilities
- **AXUI Compiler**: Optimize JSON parsing using `simdjson::ondemand`.
- **Zero-Copy Bridge**: Implement shared memory or DMA paths for UI telemetry.
- **Theme Resolution**: Maintain `axui::ThemeResolver` for high-velocity theme switching.
- **Modernization**: Lead the UI modernization efforts (Zoom/Pan in PlutoNavigator).

## 🛡️ Zenith Pillar Priority
- **Pillar 1 (Zero-Allocation)**: Ensure UI data pipelines do not trigger heap churn.

## 🛠️ Preferred Workflows
- `axui_tests` and `axui_bench`.
- StylePipeline optimization.
- Background worker thread management for parsing.

## 📈 Success Metrics
- JSON parse time < 100µs.
- 60FPS UI rendering without engine interference.
