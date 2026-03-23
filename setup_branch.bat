@echo off
REM ----------------------------------------------------
REM Setup inicial para branch de ESP-IDF
REM ----------------------------------------------------

REM 1️⃣ Exportar el IDF
call C:\Espressif\frameworks\esp-idf-v5.3.4\export.bat

REM 2️⃣ Limpiar build anterior
rmdir /s /q build

REM 3️⃣ Configurar target (esp32s3 por ejemplo)
idf.py set-target esp32s3

REM 4️⃣ Compilar todo desde cero
idf.py build



REM %️⃣ Flasehar
idf.py flash 


REM 6️⃣ Compilar todo desde cero
idf.py monitor

echo.
echo -------------------------------
echo Preparacion completa
echo Ahora podes abrir VSCode o hacer flash/monitor
echo -------------------------------
pause