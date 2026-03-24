@echo off
set PYTHONPATH=%~dp0scripts;%PYTHONPATH%
python "%~dp0scripts\ci\ci.py" %*
