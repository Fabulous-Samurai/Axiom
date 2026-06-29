Param()

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$repoRoot = Resolve-Path (Join-Path $scriptDir "..\..")
Set-Location $repoRoot

New-Item -ItemType Directory -Force -Path reports | Out-Null
New-Item -ItemType Directory -Force -Path output | Out-Null

$pyChecker = Join-Path $scriptDir "zero_alloc_checker_win.py"
# Adjusted check for python availability to avoid Get-Command parameter issues
try {
    $pythonCmd = (Get-Command python -ErrorAction Stop).Source
} catch {
    $pythonCmd = $null
}
if ($pythonCmd -and (Test-Path $pyChecker)) {
    Write-Host "Running Python checker..."
    & $pythonCmd $pyChecker --out "reports/zero_alloc_scan_summary.json" --text "output/parsed_issues.txt"
    exit $LASTEXITCODE
}

Write-Host "Python checker not available or missing. Running PowerShell fallback scanner..."

$patterns = @("std::vector","std::string","new\s","malloc\(","free\(","throw\s","std::mutex","std::unique_ptr","std::shared_ptr","std::lock_guard","std::thread")
$results = @()
$skipDirs = @(".git", "build", "node_modules", "_deps", "extern", "mimalloc-src", "ninja-build")

Get-ChildItem -Recurse -File | Where-Object {
    $fullName = $_.FullName
    $isValid = $fullName -match "\.(cpp|h|hpp)$"
    $isSkipped = $false
    foreach ($skip in $skipDirs) {
        if ($fullName -match "\\$skip\\") { $isSkipped = $true; break }
    }
    return $isValid -and (-not $isSkipped)
} | ForEach-Object {
    $path = $_.FullName
    try {
        $lines = Get-Content -LiteralPath $path -ErrorAction Stop
    } catch { return }
    for ($i = 0; $i -lt $lines.Count; $i++) {
        $ln = $lines[$i]
        foreach ($pat in $patterns) {
            if ($ln -match $pat) {
                $results += [PSCustomObject]@{ file=$path; line=($i+1); pattern=$pat; text=$ln }
            }
        }
    }
}

$results | ConvertTo-Json -Depth 6 | Out-File "reports/zero_alloc_scan_summary.json" -Encoding UTF8

"" | Out-File "output/parsed_issues.txt" -Encoding UTF8
$grouped = $results | Group-Object -Property file
foreach ($g in $grouped) {
    Add-Content "output/parsed_issues.txt" ("--- File: " + $g.Name + " ---")
    Add-Content "output/parsed_issues.txt" ("Total issues: " + $g.Count)
    foreach ($r in $g.Group) {
        Add-Content "output/parsed_issues.txt" ("[WARN] Line " + $r.line + " (" + $r.pattern + "): " + $r.text)
    }
    Add-Content "output/parsed_issues.txt" ""
}

Write-Host "Scan complete. Wrote reports/zero_alloc_scan_summary.json and output/parsed_issues.txt"
Exit 0
