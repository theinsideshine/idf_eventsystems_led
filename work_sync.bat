@echo off
setlocal

echo ==============================
echo Sync time + BUILD / FLASH / MONITOR
echo ==============================

echo.
echo [1/4] Intentando sincronizar hora...
w32tm /resync >nul 2>&1
if errorlevel 1 (
    echo    No se pudo sincronizar la hora. Sigo igual...
) else (
    echo    Hora sincronizada.
)

echo.
echo [2/4] Exportando ESP-IDF...
call C:\Espressif\frameworks\esp-idf-v5.3.4\export.bat
if errorlevel 1 goto end

echo.
echo [3/4] Reconfigurando...
idf.py reconfigure
if errorlevel 1 goto end

echo.
echo [4/4] Compilando...
copy /b build\build.ninja +,, >nul 2>&1
idf.py build
if errorlevel 1 goto end

echo.
echo Flasheando...
idf.py flash
if errorlevel 1 goto end

echo.
echo Monitor...
idf.py monitor

:end
echo.
pause
endlocal