#!/usr/bin/env bash
set -euo pipefail

OUT_JSON="reports/zero_alloc_scan_summary.json"
OUT_TEXT="output/parsed_issues.txt"
mkdir -p reports output

PATTERNS=("std::vector" "std::string" "\bnew\b" "malloc(" "free(" "throw" "std::mutex" "std::unique_ptr" "std::shared_ptr")

echo "{" > "$OUT_JSON"
first=true
> "$OUT_TEXT"

for f in $(git ls-files | grep -E '\.(cpp|c|h|hpp|py|cs)$' || true); do
  count=0
  lines=""
  while IFS= read -r line; do
    for pat in "${PATTERNS[@]}"; do
      if echo "$line" | grep -E -q "$pat"; then
        count=$((count+1))
        lines+="$line\n"
      fi
    done
  done < "$f"
  if [ $count -gt 0 ]; then
    if [ "$first" = true ]; then first=false; else echo "," >> "$OUT_JSON"; fi
    echo "  \"$f\": { \"total\": $count }" >> "$OUT_JSON"
    printf "--- File: %s ---\nTotal issues: %d\n%s\n" "$f" "$count" "$lines" >> "$OUT_TEXT"
  fi
done

echo "}" >> "$OUT_JSON"
echo "Scan complete. Wrote $OUT_JSON and $OUT_TEXT"
