@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cd /d "%~dp0.."
echo === CMAKE CONFIGURE ===
cmake --preset develop 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo === CONFIGURE FAILED ===
    exit /b %ERRORLEVEL%
)
echo === CMAKE BUILD ===
cmake --build build/develop 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo === BUILD FAILED ===
    exit /b %ERRORLEVEL%
)
echo === BUILD SUCCEEDED ===
