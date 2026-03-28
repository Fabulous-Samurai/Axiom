# Roadmap & FAQ

Looking ahead at the evolution of the AXIOM ecosystem and addressing common questions.

---

## 10. Roadmap & Future Plans

The development of AXIOM is structured into multi-phase sprints focusing on orchestration intelligence and universal access.

### Phase 7: AXIOM Studio (Current Focus)
- Full-featured IDE for designing `SelectiveDispatcher` topologies.
- Drag-and-drop stage orchestration.
- Live-reloading of data flows via high-speed IPC.

### Phase 8: Cloud Orchestration
- Remote monitoring of distributed AXIOM instances.
- Zero-Trust telemetry sync between edge and cloud.
- Unified dashboard for swarm management.

### V4.0: Autonomous AI Swarm
- Integration of predictive AI models to automatically re-route tasks based on forecasted bottlenecks.
- Self-healing pipeline topologies.
- Autonomous load balancing across hybrid-cloud environments.

---

## 11. FAQ & Troubleshooting

### Q: Why do I see "AXChart is not a type" errors?
**A:** This usually means the `axui_components.qrc` was not correctly linked or the QML search path is missing the `components` directory. Ensure you build with `ninja` after a clean `cmake` configuration.

### Q: Does AXIOM support Linux?
**A:** Yes! While we highlight MSYS2 for Windows, AXIOM builds natively on any modern Linux distribution with G++ 11+ and Qt 6.5+.

### Q: How can I reduce telemetry jitter?
**A:** Ensure the `AxiomDashboard` is receiving EMA-smoothed data from the C++ backend. Check `dashboard_manager.cpp` to verify the alpha constant for the Exponential Moving Average filter is set correctly (default: 0.2).

### Q: IPC connection fails with port 5555.
**A:** Ensure no other service is occupying the port. You can change the port via the `AXIOM_IPC_PORT` environment variable.

---
[← Back to Performance](Benchmarks-&-Development.md) | [Home](Home.md)
