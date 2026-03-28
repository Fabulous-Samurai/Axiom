# Getting Started with AXIOM

Follow this guide to set up your development environment and build the AXIOM ecosystem from source.

## 📋 Prerequisites

AXIOM is optimized for **Windows (MSYS2 / UCRT64)** and **Linux (Ubuntu 22.04+)**.

### Windows (MSYS2)
1. Install [MSYS2](https://www.msys2.org/).
2. Open the **UCRT64** terminal and run:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc \
             mingw-w64-ucrt-x86_64-cmake \
             mingw-w64-ucrt-x86_64-ninja \
             mingw-w64-ucrt-x86_64-qt6-base \
             mingw-w64-ucrt-x86_64-qt6-declarative \
             mingw-w64-ucrt-x86_64-qt6-5compat
   ```

## 🏗️ Building the Project

### 1. Clone the Repository
```bash
git clone https://github.com/your-org/axiom.git
cd axiom
```

### 2. Configure & Build
We recommend using the provided Python helper for the Dashboard and Studio:
```bash
# Build Dashboard & Studio in Mock Mode
python scripts/axiom_dashboard.py --mock --build
```

Alternatively, use standard CMake:
```bash
mkdir build && cd build
cmake -G Ninja ..
ninja axiom_dashboard axiom_studio
```

## 🚀 First Launch

### Launching the Dashboard (Monitor)
The Dashboard allows you to visualize system flows in real-time.
```bash
# Using the helper script
python scripts/axiom_dashboard.py --mock
```

### Launching AXIOM Studio (IDE)
Studio is the entry point for designing orchestrations.
```bash
./build/bin/axiom_studio.exe
```

## 🧪 Verification
To ensure everything is working correctly:
- Run the internal test suite: `ctest --output-on-failure`
- Check SonarQube quality gates (if configured): `python scripts/check_quality.py`

---
[← Back to Home](Home.md) | [Next: Core Architecture →](Core-Engine-Architecture.md)
