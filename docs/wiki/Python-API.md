# Python API (nanobind)

AXIOM provides a high-performance Python bridge using **nanobind**, allowing for seamless integration into AI and data science workflows without sacrifice to throughput.

---

## 7.1 Installation

The AXIOM Python package is built as a native extension during the standard build process.

```bash
# Ensure you have nanobind installed in your env
pip install nanobind

# Build the bindings
mkdir build && cd build
cmake -DAXIOM_PYTHON_BINDINGS=ON ..
ninja axiom_python
```

---

## 7.2 Basic Usage

Ingesting data and starting the `SelectiveDispatcher` from Python:

```python
import axiom

# Initialize the engine
engine = axiom.Engine()

# Add a stage
engine.add_stage("input_filter", "cpu_simd")

# Ingest data
payload = {"sensor_id": 1, "value": 98.6}
engine.dispatch("input_filter", payload)

# Stop the engine
engine.stop()
```

---

## 7.3 Advanced Examples

### AI Orchestration (Pluto Swarm)
You can directly control the `Pluto Swarm` orchestration logic from Python to dynamically re-route tasks based on AI model outputs.

```python
# Create a swarm
swarm = axiom.PlutoSwarm(nodes=5)

# Update routing weights dynamically
swarm.set_weights({"node_1": 0.8, "node_2": 0.2})
```

---

## 7.4 API Reference
- `axiom.Engine`: Main entry point for the orchestration core.
- `axiom.SelectiveDispatcher`: Low-level control for task routing.
- `axiom.HarmonicArena`: Access to the arena-backed memory pools.

---
[← Back to User Interfaces](User-Interfaces.md) | [Next: Performance & Development →](Benchmarks-&-Development.md)
