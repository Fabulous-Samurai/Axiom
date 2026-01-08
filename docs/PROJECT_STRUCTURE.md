# AXIOM Engine - Project Structure

This document describes the organized directory structure of the AXIOM Engine project.

## Directory Layout

```
axiom_engine/
├── .build-output/          # Temporary build outputs (gitignored)
├── .vscode/                # VS Code configuration
├── build/                  # Build configuration files (tracked)
│   ├── cmake/              # CMake configuration files
│   │   └── version.h.in    # Version header template
│   └── scripts/            # Future build helper scripts
├── codeql-custom-queries-cpp/   # CodeQL C++ analysis queries
├── codeql-custom-queries-python/ # CodeQL Python analysis queries
├── core/                   # Core engine implementations
│   ├── dispatch/           # Dispatcher system
│   └── engine/             # Core engine components
├── docs/                   # All documentation
│   ├── api/                # API documentation
│   ├── qa/                 # QA reports and test suites
│   ├── reports/            # Development reports
│   └── user/               # User documentation
├── examples/               # Example code and demos
│   └── advanced/           # Advanced examples
├── gui/                    # GUI implementations
│   └── python/             # Python GUI applications
├── include/                # Public C++ header files
├── ninja-build/            # Ninja build directory (gitignored)
├── scripts/                # Build and utility scripts
│   ├── fast_build.ps1      # PowerShell fast build
│   ├── ninja_build.bat     # Windows Ninja build
│   └── ninja_build.sh      # Unix Ninja build
├── src/                    # C++ source files
├── tests/                  # Test files
│   ├── integration/        # Integration tests
│   └── unit/               # Unit tests
├── tools/                  # Development tools
│   ├── analysis/           # Analysis tools
│   └── visualization/      # Visualization tools
├── .gitignore              # Git ignore rules
├── CHANGELOG.md            # Project changelog
├── CMakeLists.txt          # Main CMake configuration
├── LICENSE                 # License file
└── README.md               # Project README
```

## Directory Purposes

### Source Code
- **src/** - C++ implementation files
- **include/** - Public C++ headers
- **core/** - Core engine subsystems

### Testing
- **tests/integration/** - Integration test files
- **tests/unit/** - Unit test files

### Documentation
- **docs/api/** - API documentation
- **docs/qa/** - Quality assurance reports and test suites
- **docs/reports/** - Development and progress reports
- **docs/user/** - User-facing documentation

### Build System
- **build/cmake/** - CMake configuration templates
- **scripts/** - Build automation scripts
- **.build-output/** - Temporary build artifacts (not tracked)
- **ninja-build/** - Ninja build directory (not tracked)

### GUI & Examples
- **gui/python/** - Python GUI applications
- **examples/** - Example code and demonstrations

### Quality & Analysis
- **codeql-custom-queries-cpp/** - C++ static analysis queries
- **codeql-custom-queries-python/** - Python static analysis queries
- **tools/** - Development and analysis tools

## Build Scripts Location

All build scripts are now located in the **scripts/** directory:
- Windows: `scripts/ninja_build.bat`, `scripts/fast_build.ps1`
- Unix/Linux: `scripts/ninja_build.sh`

## Notes

- Build outputs are automatically placed in `.build-output/` or `ninja-build/`
- Both are gitignored to keep the repository clean
- The `build/` directory is tracked and contains CMake configuration files
- Test files are organized by type: integration vs unit tests
- All documentation and reports are consolidated under `docs/`
