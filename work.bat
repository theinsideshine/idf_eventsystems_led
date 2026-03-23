@echo off
call C:\Espressif\frameworks\esp-idf-v5.3.4\export.bat

echo ==============================
echo BUILD / FLASH / MONITOR
echo ==============================

idf.py build
if errorlevel 1 goto end

idf.py flash
if errorlevel 1 goto end

idf.py monitor

:end
pause