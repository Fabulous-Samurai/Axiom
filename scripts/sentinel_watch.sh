#!/bin/bash
# scripts/sentinel_watch.sh

echo "═══ SENTINEL WATCH CYCLE ═══"
echo "Timestamp: $(date '+%Y-%m-%d %H:%M:%S')"
echo ""

# GitHub Actions son workflow durumunu çek
gh run list --limit 5 --json status,conclusion,name,createdAt \
  2>/dev/null || echo "GitHub CLI not available, using local build"

# Lokal build durumu
echo "=== LOCAL BUILD STATUS ==="

# C++ Build
echo "[1/7] C++ Build..."
BUILD_LOG=$(cmake --build build --config Release -j$(nproc) 2>&1)
BUILD_EXIT=$?
BUILD_WARNINGS=$(echo "$BUILD_LOG" | grep -c "warning:")
BUILD_ERRORS=$(echo "$BUILD_LOG" | grep -c "error:")
echo "  Exit: $BUILD_EXIT | Warnings: $BUILD_WARNINGS | Errors: $BUILD_ERRORS"

# C++ Tests
echo "[2/7] C++ Tests..."
TEST_LOG=$(./build/run_tests 2>&1)
TEST_EXIT=$?
TEST_PASSED=$(echo "$TEST_LOG" | grep -oP '\d+ passed' | grep -oP '\d+' || echo "0")
TEST_FAILED=$(echo "$TEST_LOG" | grep -oP '\d+ failed' | grep -oP '\d+' || echo "0")
echo "  Exit: $TEST_EXIT | Passed: $TEST_PASSED | Failed: $TEST_FAILED"

# Giga Tests
echo "[3/7] Giga Test Suite..."
GIGA_LOG=$(timeout 120 ./build/giga_test_suite 2>&1)
GIGA_EXIT=$?
echo "  Exit: $GIGA_EXIT"

# AST Drills
echo "[4/7] AST Drills..."
AST_LOG=$(./build/ast_drills 2>&1)
AST_EXIT=$?
echo "  Exit: $AST_EXIT"

# Python Import
echo "[5/7] Python Binding..."
PY_LOG=$(python -c "import axiom_native; print('OK:', axiom_native.evaluate('2+2', 'algebraic').value)" 2>&1)
PY_EXIT=$?
echo "  Exit: $PY_EXIT | Output: $PY_LOG"

# Python Tests
echo "[6/7] Python Tests..."
PYTEST_LOG=$(timeout 120 pytest tests/ -x -q --tb=short 2>&1)
PYTEST_EXIT=$?
PYTEST_PASSED=$(echo "$PYTEST_LOG" | grep -oP '\d+ passed' | grep -oP '\d+' || echo "0")
PYTEST_FAILED=$(echo "$PYTEST_LOG" | grep -oP '\d+ failed' | grep -oP '\d+' || echo "0")
echo "  Exit: $PYTEST_EXIT | Passed: $PYTEST_PASSED | Failed: $PYTEST_FAILED"

# Performance Quick Check
echo "[7/7] Performance Sanity..."
PERF_LOG=$(python -c "
import axiom_native, time
start = time.perf_counter()
for _ in range(10000):
    axiom_native.evaluate('2+2', 'algebraic')
elapsed = time.perf_counter() - start
throughput = 10000 / elapsed
print(f'Throughput: {throughput:,.0f} ops/sec')
if throughput < 1500000:
    print('REGRESSION DETECTED')
    exit(1)
print('OK')
" 2>&1)
PERF_EXIT=$?
echo "  $PERF_LOG"

echo ""
echo "═══ WATCH SUMMARY ═══"
echo "Build:      $([ $BUILD_EXIT -eq 0 ] && echo '✅' || echo '❌') (warnings: $BUILD_WARNINGS)"
echo "Tests:      $([ $TEST_EXIT -eq 0 ] && echo '✅' || echo '❌') ($TEST_PASSED passed, $TEST_FAILED failed)"
echo "Giga:       $([ $GIGA_EXIT -eq 0 ] && echo '✅' || echo '❌')"
echo "AST:        $([ $AST_EXIT -eq 0 ] && echo '✅' || echo '❌')"
echo "Python:     $([ $PY_EXIT -eq 0 ] && echo '✅' || echo '❌')"
echo "Pytest:     $([ $PYTEST_EXIT -eq 0 ] && echo '✅' || echo '❌') ($PYTEST_PASSED passed, $PYTEST_FAILED failed)"
echo "Performance:$([ $PERF_EXIT -eq 0 ] && echo '✅' || echo '❌')"

# Toplam skor
TOTAL_CHECKS=7
PASSED_CHECKS=0
[ $BUILD_EXIT -eq 0 ] && ((PASSED_CHECKS++))
[ $TEST_EXIT -eq 0 ] && ((PASSED_CHECKS++))
[ $GIGA_EXIT -eq 0 ] && ((PASSED_CHECKS++))
[ $AST_EXIT -eq 0 ] && ((PASSED_CHECKS++))
[ $PY_EXIT -eq 0 ] && ((PASSED_CHECKS++))
[ $PYTEST_EXIT -eq 0 ] && ((PASSED_CHECKS++))
[ $PERF_EXIT -eq 0 ] && ((PASSED_CHECKS++))

echo ""
echo "HEALTH: $PASSED_CHECKS/$TOTAL_CHECKS"

if [ $PASSED_CHECKS -eq $TOTAL_CHECKS ]; then
    echo "STATUS: 🟢 ALL HEALTHY"
    exit 0
elif [ $PASSED_CHECKS -ge 5 ]; then
    echo "STATUS: 🟡 NEEDS ATTENTION"
    exit 1
else
    echo "STATUS: 🔴 CRITICAL"
    exit 2
fi
