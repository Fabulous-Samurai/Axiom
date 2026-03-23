#!/bin/bash
# scripts/sentinel_analyze.sh

analyze_build_error() {
    local ERROR_LINE=$1

    # Dosya ve satır çıkar
    FILE=$(echo "$ERROR_LINE" | grep -oP '[\w/]+\.(cpp|h|hpp)' | head -1)
    LINE=$(echo "$ERROR_LINE" | grep -oP ':\d+:' | tr -d ':' | head -1)

    echo "FILE: $FILE"
    echo "LINE: $LINE"

    # Son değişiklik
    echo "LAST CHANGE:"
    git log -1 --format="  Commit: %h%n  Author: %an%n  Message: %s%n  Date: %ar" -- "$FILE"

    # Değişiklik diff
    echo "DIFF:"
    git diff HEAD~1 -- "$FILE" | head -30

    # Etkilenen fonksiyon
    if [ -n "$LINE" ] && [ -f "$FILE" ]; then
        echo "CONTEXT (±5 lines):"
        sed -n "$((LINE-5)),$((LINE+5))p" "$FILE" 2>/dev/null
    fi
}

analyze_test_failure() {
    local TEST_NAME=$1
    local TEST_LOG=$2

    echo "FAILED TEST: $TEST_NAME"

    # Test dosyasını bul
    TEST_FILE=$(find tests/ -name "*.cpp" -o -name "*.py" | xargs grep -l "$TEST_NAME" 2>/dev/null | head -1)
    echo "TEST FILE: $TEST_FILE"

    # Beklenen vs gerçek
    echo "EXPECTED vs ACTUAL:"
    echo "$TEST_LOG" | grep -A3 -B1 "$TEST_NAME" | head -10

    # İlgili kaynak dosyaları
    echo "RELATED SOURCE FILES (recently changed):"
    git diff --name-only HEAD~3 -- src/ include/ | head -10
}

analyze_perf_regression() {
    echo "PERFORMANCE REGRESSION ANALYSIS"

    # Son 5 commit
    echo "RECENT COMMITS:"
    git log --oneline -5

    # git bisect ile regresyon commit'ini bul
    echo "BISECTING..."
    echo "Run manually:"
    echo "  git bisect start"
    echo "  git bisect bad HEAD"
    echo "  git bisect good HEAD~5"
    echo "  git bisect run ./scripts/perf_check.sh"
}
