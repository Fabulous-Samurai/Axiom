@echo off
set "EXE_PATH=%~dp0out\default-ninja\bin\axiom_studio.exe"

if exist "%EXE_PATH%" (
    echo [CLI] Starting AXIOM Studio...
    start "" "%EXE_PATH%" %*
) else (
    echo [ERROR] AXIOM Studio executable not found at %EXE_PATH%
    echo [ERROR] Please run build first.
    exit /b 1
)
