# AXIOM Zenith - Agent-Executable Formal Verification Runner
param (
    [string]$Model = "MantisSecureVault.tla",
    [string]$LogFile = ".agents/logs/tla_last_run.log"
)

$Image = "axiom-ci:ubuntu-latest"
$TlaDir = "/workspace/formal/tla"
$JarUrl = "https://github.com/tlaplus/tlaplus/releases/download/v1.8.0/tla2tools.jar"

Write-Host "--------------------------------------------------------"
Write-Host "  [AXIOM] FORMAL VERIFICATION ENGINE (AGENT RUNNER)     "
Write-Host "--------------------------------------------------------"

# Ensure we are in the right directory
$ProjectRoot = Get-Location
$ModelPath = Join-Path $ProjectRoot "axiom-master/formal/tla/$Model"

if (!(Test-Path $ModelPath)) {
    Write-Error "Model file not found at $ModelPath"
    "ERROR: Model file $Model not found." | Out-File -FilePath $LogFile
    exit 1
}

Write-Host "[EXEC] Running TLC on $Model. Output redirected to $LogFile"

# Redirect all output to log file for agent analysis
& docker run --rm `
    -v "${ProjectRoot}/axiom-master:/workspace" `
    -w "$TlaDir" `
    $Image `
    bash -c "
        if [ ! -f tla2tools.jar ]; then 
            curl -L $JarUrl -o tla2tools.jar;
        fi
        java -Xmx4G -cp tla2tools.jar tlc2.TLC -workers auto -deadlock -gzip $Model
    " *>&1 | Out-File -FilePath $LogFile

Write-Host "[DONE] Check $LogFile for results."
