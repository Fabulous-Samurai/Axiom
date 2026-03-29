# AXIOM Zenith - Agent-Executable Docker Test Runner (Line-Ending Fixed)
param (
    [string]$LogFile = ".agents/logs/tests_docker_last_run.log"
)

$Image = "axiom-ci:ubuntu-latest"

Write-Host "--------------------------------------------------------"
Write-Host "  [AXIOM] DOCKER TEST RUNNER (FIXED COMMAND)            "
Write-Host "--------------------------------------------------------"

$ProjectRoot = Get-Location

# Single-line command to avoid bash CRLF issues
$DockerCmd = "mkdir -p build && cd build && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release .. && ninja axiom run_tests giga_test_suite && ./run_tests && ./giga_test_suite --benchmark_format=console"

& docker run --rm -v "${ProjectRoot}/axiom-master:/workspace" -w "/workspace" $Image bash -c "$DockerCmd" *>&1 | Out-File -FilePath $LogFile

Write-Host "[DONE] Docker tests complete. Log saved to $LogFile"
