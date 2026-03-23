#!/bin/bash
# scripts/sentinel_validate.sh

validate_repair() {
    local REPAIR_TYPE=$1
    echo "═══ VALIDATING REPAIR: $REPAIR_TYPE ═══"

    VALID=true

    # 1. Build kontrolü
    echo "[1/5] Build check..."
    cmake --build build -j$(nproc) 2>&1 | tail -3
    if [ $? -ne 0 ]; then
        echo "  ❌ Build STILL FAILING"
        VALID=false
    else
        echo "  ✅ Build OK"
    fi

    # 2. Etkilenen test
    echo "[2/5] Affected test check..."
    ./build/run_tests 2>&1 | tail -5
    if [ $? -ne 0 ]; then
        echo "  ❌ Tests STILL FAILING"
        VALID=false
    else
        echo "  ✅ Tests OK"
    fi

    # 3. Python binding
    echo "[3/5] Python binding check..."
    python -c "import axiom_native; print('OK')" 2>&1
    if [ $? -ne 0 ]; then
        echo "  ❌ Python binding BROKEN"
        VALID=false
    else
        echo "  ✅ Python OK"
    fi

    # 4. Performance regression check
    echo "[4/5] Performance check..."
    python -c "
import axiom_native, time
start = time.perf_counter()
for _ in range(10000):
    axiom_native.evaluate('2+2', 'algebraic')
elapsed = time.perf_counter() - start
throughput = 10000 / elapsed
if throughput < 1500000:
    print(f'  ❌ REGRESSION: {throughput:,.0f} ops/sec')
    exit(1)
print(f'  ✅ Performance OK: {throughput:,.0f} ops/sec')
" 2>&1
    if [ $? -ne 0 ]; then
        VALID=false
    fi

    # 5. AXUI build (varsa)
    echo "[5/5] AXUI build check..."
    if [ -f "axui/compiler/CMakeLists.txt" ]; then
        cmake --build axui/compiler/build 2>&1 | tail -3
        if [ $? -ne 0 ]; then
            echo "  ❌ AXUI build BROKEN"
            VALID=false
        else
            echo "  ✅ AXUI OK"
        fi
    else
        echo "  ⏭️ AXUI not yet configured, skipping"
    fi

    echo ""
    if [ "$VALID" = true ]; then
        echo "═══ VALIDATION: ✅ PASS ═══"
        echo "Repair successful. Committing..."
        return 0
    else
        echo "═══ VALIDATION: ❌ FAIL ═══"
        echo "Repair did not fix the issue. Rolling back..."
        git checkout -- .
        return 1
    fi
}
