# User Interfaces

AXIOM provides two distinct interfaces for system observability and orchestration management.

---

## 6.1 AXIOM Dashboard (Systems Flow Monitor)

The **AXIOM Dashboard** is a professional-grade diagnostic tool designed for high-frequency monitoring. It provides a real-time "Pipeline Topology" view of your engine's internal stages.

### Key Visual Analytics
- **EMA Smoothing:** All telemetry metrics (CPU, RAM, GPU, IPC) are processed through an Exponential Moving Average filter to eliminate jitter and provide stable trends.
- **Dynamic Throughput Heat:** The thickness of pipeline links reflects the real-time message volume, allowing for instant identification of high-traffic paths.
- **Health Score Mapping:** Stages dynamically change color from Green to Red based on a weighted health score derived from queue saturation and processing latency.
- **Backpressure Pulse:** When a stage's queue fills beyond 80%, the node exhibits a high-frequency vibration effect, serving as an instinctive physical signal of a bottleneck.

### Diagnostic Tools
- **Analytic Inspector:** Clicking any stage opens a detail panel providing deep-dive metrics and a 10-sample historical trend table.
- **Performance Overlay:** Use `Ctrl+Shift+D` to toggle a hidden debug overlay that monitors the dashboard's internal frame time and memory usage.

---

## 6.2 AXIOM Studio (Integrated Development Environment)

**AXIOM Studio** is the future-facing IDE for the AXIOM ecosystem. While currently in the architectural prototype phase, its mission is to provide a complete workspace for designing and deploying SelectiveDispatcher orchestrations.

### Architectural Features
- **Decoupled Workflow:** Studio is built as a separate executable target from the Dashboard, ensuring that IDE development does not impact the stability of the monitoring runtime.
- **IPC Ready:** Engineered to connect directly to the AXIOM Engine core via high-speed IPC, allowing for live configuration updates without process restarts.
- **Modern QML Design:** Built with a scalable, component-based QML architecture that supports fluid layout transitions and high-DPI displays.

---
[← Back to Security](Security-&-Formal-Verification.md) | [Next: Python API →](Python-API.md)
