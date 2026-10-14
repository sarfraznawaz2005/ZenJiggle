@echo off
setlocal
cd /d "%~dp0"

where gcc >nul 2>&1 || (echo ERROR: gcc not found. Install MinGW-w64 and add it to PATH. & exit /b 1)
where windres >nul 2>&1 || (echo ERROR: windres not found. It comes with MinGW-w64. & exit /b 1)

if not exist dist mkdir dist

echo Building icon resource...
windres zenjiggle.rc -O coff -o dist\zenjiggle_res.o || exit /b 1

echo Compiling...
gcc -Os -s -nostartfiles -ffunction-sections -fdata-sections -Wl,--gc-sections -mwindows ^
    -o dist\ZenJiggle.exe zenjiggle.c dist\zenjiggle_res.o ^
    -lkernel32 -luser32 -lshell32 -lgdi32 || exit /b 1

del dist\zenjiggle_res.o

echo.
echo Done: %~dp0dist\ZenJiggle.exe
for %%F in (dist\ZenJiggle.exe) do echo Size: %%~zF bytes
exit /b 0
