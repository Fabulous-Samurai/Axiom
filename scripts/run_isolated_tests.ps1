# Ensure the script is run with administrator privileges
if (-not ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")) {
    Write-Host "Please run this script as Administrator." -ForegroundColor Red
    exit
}

# Define the CPU core to isolate and pin tasks to
$IsolatedCore = 3

# Check if the taskset equivalent is available
if (-Not (Get-Command "wsl" -ErrorAction SilentlyContinue)) {
    Write-Host "WSL is required to run Linux commands like taskset. Please install WSL." -ForegroundColor Red
    exit
}

# Run the benchmark on the isolated core using WSL and taskset
Write-Host "Running benchmark on isolated core $IsolatedCore..."
wsl taskset -c $IsolatedCore ./build/axiom_benchmark.exe

# Run the tests on the isolated core using WSL and taskset
Write-Host "Running tests on isolated core $IsolatedCore..."
wsl taskset -c $IsolatedCore ./build/run_tests.exe

# Provide a warning about OS jitter
Write-Host "Note: Running benchmarks or tests on non-isolated cores may introduce OS jitter." -ForegroundColor Yellow
Write-Host "Ensure background services and Chrome are not running on the isolated core."
