#!/bin/bash
# scripts/sentinel_detect.sh

detect_errors() {
    local LOG_FILE=$1

    echo "=== ERROR DETECTION ==="

    # BUILD ERRORS
    if grep -qE "error:" "$LOG_FILE"; then
        echo "🔴 P0: Build error detected"
        grep -n "error:" "$LOG_FILE" | head -10
        echo "CATEGORY: BUILD_ERROR"
        return 0
    fi

    # UNDEFINED REFERENCE
    if grep -qE "undefined reference to" "$LOG_FILE"; then
        echo "🔴 P0: Linker error — undefined reference"
        grep "undefined reference" "$LOG_FILE" | head -5
        echo "CATEGORY: LINKER_ERROR"
        echo "LIKELY_CAUSE: New file not added to CMakeLists.txt"
        echo "LIKELY_FIX: Add source file to target_sources()"
        return 0
    fi

    # INCLUDE NOT FOUND
    if grep -qE "fatal error:.*file not found|No such file or directory" "$LOG_FILE"; then
        echo "🔴 P0: Missing include"
        grep -E "fatal error:|No such file" "$LOG_FILE" | head -5
        echo "CATEGORY: MISSING_INCLUDE"
        echo "LIKELY_CAUSE: Header file missing or include path wrong"
        echo "LIKELY_FIX: Check include path or create missing header"
        return 0
    fi

    # PYTHON IMPORT ERROR
    if grep -qE "ModuleNotFoundError|ImportError" "$LOG_FILE"; then
        echo "🔴 P0: Python import failure"
        grep -E "ModuleNotFoundError|ImportError" "$LOG_FILE" | head -3
        echo "CATEGORY: PYTHON_IMPORT"
        echo "LIKELY_CAUSE: nanobind module not built or PYTHONPATH wrong"
        echo "LIKELY_FIX: Rebuild + export PYTHONPATH"
        return 0
    fi

    # TEST FAILURE
    if grep -qE "FAILED|FAIL|AssertionError|assert.*failed" "$LOG_FILE"; then
        echo "🟠 P1: Test failure detected"
        grep -B2 -A5 "FAILED\|FAIL\|AssertionError" "$LOG_FILE" | head -20
        echo "CATEGORY: TEST_FAILURE"
        return 1
    fi

    # COMPILER WARNING
    if grep -qE "warning:" "$LOG_FILE"; then
        WARNING_COUNT=$(grep -c "warning:" "$LOG_FILE")
        echo "🟡 P2: $WARNING_COUNT compiler warnings"
        grep "warning:" "$LOG_FILE" | sort -u | head -10
        echo "CATEGORY: COMPILER_WARNING"
        return 2
    fi

    # DEPRECATION
    if grep -qiE "deprecated|deprecation" "$LOG_FILE"; then
        echo "🟡 P2: Deprecation warning"
        grep -i "deprecated" "$LOG_FILE" | head -5
        echo "CATEGORY: DEPRECATION"
        return 2
    fi

    # PERFORMANCE REGRESSION
    if grep -qE "REGRESSION DETECTED" "$LOG_FILE"; then
        echo "🔴 P0: Performance regression!"
        grep "REGRESSION" "$LOG_FILE"
        echo "CATEGORY: PERF_REGRESSION"
        return 0
    fi

    echo "✅ No errors detected"
    return 3
}
