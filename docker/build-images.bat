@echo off
echo ========================================================
echo   AXIOM — Building CI Docker Images (Windows)
echo ========================================================

echo.
echo [1/2] Building Ubuntu 22.04 image...
docker build -f "%~dp0Dockerfile.ubuntu" -t axiom-ci:ubuntu "%~dp0."

echo.
echo [2/2] Building Fedora 39 image...
docker build -f "%~dp0Dockerfile.fedora" -t axiom-ci:fedora "%~dp0."

echo.
echo Check ready images:
docker images | findstr axiom-ci
echo.
pause
