@echo off
REM ─────────────────────────────────────────────
REM Freak Engine - Third-party submodule setup
REM Run this once after cloning the repo.
REM ─────────────────────────────────────────────

echo [Freak Engine] Setting up third-party submodules...
echo.

cd /d "%~dp0\.."

REM ── SDL3 ──
echo [1/8] SDL3...
git submodule add --force https://github.com/libsdl-org/SDL.git third_party/SDL 2>nul
cd third_party/SDL && git checkout release-3.2.8 && cd ../..

REM ── bgfx.cmake (CMake wrapper for bgfx) ──
echo [2/8] bgfx.cmake...
git submodule add --force https://github.com/bkarber/bgfx.cmake.git third_party/bgfx.cmake 2>nul
cd third_party/bgfx.cmake && git checkout v1.128.8862-507 && cd ../..

REM ── EnTT ──
echo [3/8] EnTT...
git submodule add --force https://github.com/skypjack/entt.git third_party/entt 2>nul
cd third_party/entt && git checkout v3.14.0 && cd ../..

REM ── Tracy ──
echo [4/8] Tracy...
git submodule add --force https://github.com/wolfpld/tracy.git third_party/tracy 2>nul
cd third_party/tracy && git checkout v0.11.1 && cd ../..

REM ── Dear ImGui ──
echo [5/8] Dear ImGui...
git submodule add --force https://github.com/ocornut/imgui.git third_party/imgui 2>nul
cd third_party/imgui && git checkout v1.91.8 && cd ../..

REM ── nlohmann/json ──
echo [6/8] nlohmann/json...
git submodule add --force https://github.com/nlohmann/json.git third_party/json 2>nul
cd third_party/json && git checkout v3.11.3 && cd ../..

REM ── toml++ ──
echo [7/8] toml++...
git submodule add --force https://github.com/marzer/tomlplusplus.git third_party/tomlplusplus 2>nul
cd third_party/tomlplusplus && git checkout v3.4.0 && cd ../..

REM ── GLM ──
echo [8/8] GLM...
git submodule add --force https://github.com/g-truc/glm.git third_party/glm 2>nul
cd third_party/glm && git checkout 1.0.1 && cd ../..

REM ── stb (no tags, just pin a commit) ──
echo [bonus] stb...
git submodule add --force https://github.com/nothings/stb.git third_party/stb 2>nul

echo.
echo [Freak Engine] Submodule setup complete.
echo.
echo Next steps:
echo   1. cmake --preset debug
echo   2. cmake --build --preset debug
echo   3. build\debug\bin\freakland.exe
echo.
pause
