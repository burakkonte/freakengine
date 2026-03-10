@echo off
REM ─────────────────────────────────────────────
REM Freak Engine - Shader Compiler
REM Compiles bgfx .sc shader sources to platform binaries.
REM
REM Usage: compile_shaders.bat [dx11|vulkan|opengl]
REM Default: dx11 (primary Windows target)
REM
REM Requires: shaderc built from bgfx.cmake
REM   Located at: build\debug\bin\shaderc.exe (or similar)
REM ─────────────────────────────────────────────

setlocal enabledelayedexpansion

cd /d "%~dp0\.."

REM ── Locate shaderc ──
set SHADERC=
for %%P in (
    "build\debug\bin\shaderc.exe"
    "build\develop\bin\shaderc.exe"
    "build\release\bin\shaderc.exe"
    "third_party\bgfx.cmake\bgfx\.build\win64_vs2022\bin\shadercRelease.exe"
) do (
    if exist %%P (
        set SHADERC=%%P
        goto :found_shaderc
    )
)

echo [ERROR] shaderc.exe not found. Build the project first (cmake --build --preset debug)
echo         or check that bgfx.cmake BGFX_BUILD_TOOLS is ON.
exit /b 1

:found_shaderc
echo [Freak Engine] Using shaderc: %SHADERC%

REM ── Platform selection ──
set PLATFORM=%1
if "%PLATFORM%"=="" set PLATFORM=dx11

if "%PLATFORM%"=="dx11" (
    set PROFILE_VS=vs_5_0
    set PROFILE_FS=ps_5_0
    set BGFX_PLATFORM=windows
) else if "%PLATFORM%"=="vulkan" (
    set PROFILE_VS=spirv
    set PROFILE_FS=spirv
    set BGFX_PLATFORM=linux
) else if "%PLATFORM%"=="opengl" (
    set PROFILE_VS=120
    set PROFILE_FS=120
    set BGFX_PLATFORM=linux
) else (
    echo [ERROR] Unknown platform: %PLATFORM% (use dx11, vulkan, or opengl)
    exit /b 1
)

echo [Freak Engine] Compiling shaders for: %PLATFORM% (%PROFILE_VS% / %PROFILE_FS%)

REM ── Paths ──
set SHADER_SRC=engine\render\shaders\src
set VARYING=engine\render\shaders\varying\varying.def.sc
set OUT_DIR=shaders\compiled
set BGFX_INCLUDE=third_party\bgfx.cmake\bgfx\src

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

REM ── Compile vertex shaders ──
set ERROR_COUNT=0

for %%F in (%SHADER_SRC%\vs_*.sc) do (
    set NAME=%%~nF
    echo   [VS] !NAME!
    %SHADERC% ^
        -f "%%F" ^
        -o "%OUT_DIR%\!NAME!.bin" ^
        --type vertex ^
        --platform %BGFX_PLATFORM% ^
        --profile %PROFILE_VS% ^
        --varyingdef "%VARYING%" ^
        -i "%BGFX_INCLUDE%"
    if errorlevel 1 (
        echo   [FAIL] !NAME!
        set /a ERROR_COUNT+=1
    )
)

REM ── Compile fragment shaders ──
for %%F in (%SHADER_SRC%\fs_*.sc) do (
    set NAME=%%~nF
    echo   [FS] !NAME!
    %SHADERC% ^
        -f "%%F" ^
        -o "%OUT_DIR%\!NAME!.bin" ^
        --type fragment ^
        --platform %BGFX_PLATFORM% ^
        --profile %PROFILE_FS% ^
        --varyingdef "%VARYING%" ^
        -i "%BGFX_INCLUDE%"
    if errorlevel 1 (
        echo   [FAIL] !NAME!
        set /a ERROR_COUNT+=1
    )
)

echo.
if %ERROR_COUNT% GTR 0 (
    echo [Freak Engine] Shader compilation FAILED: %ERROR_COUNT% error(s)
    exit /b 1
) else (
    echo [Freak Engine] All shaders compiled successfully to %OUT_DIR%\
)

dir /b "%OUT_DIR%\*.bin" 2>nul
echo.
