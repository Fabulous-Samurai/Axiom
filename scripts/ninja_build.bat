@echo off
echo 🚀 AXIOM ENGINE v3.0 - CENTRALIZED BUILD SYSTEM
echo ========================================

REM Check if CMakePresets.json exists
if not exist "CMakePresets.json" (
    echo ❌ Error: CMakePresets.json not found!
    exit /b 1
)

echo 🏗️ Configuring with preset: default-ninja...
cmake --preset default-ninja

echo 🏎️ Building AXIOM Engine...
cmake --build out\default-ninja -j %NUMBER_OF_PROCESSORS%

echo ✅ AXIOM Engine v3.0 build complete!
echo 📍 Executable: out\default-ninja\axiom.exe
echo 🧪 Test suite: out\default-ninja\run_tests.exe
