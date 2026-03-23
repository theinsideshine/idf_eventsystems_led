@echo off
call C:\Espressif\frameworks\esp-idf-v5.3.4\export.bat

echo ==============================
echo MONITOR
echo ==============================



idf.py monitor

:end
pause