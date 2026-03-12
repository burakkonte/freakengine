@echo off
chcp 65001 >nul 2>&1
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cmake --build "C:\Users\su-ko\OneDrive\MASAST~1\freakland-m1\.claude\worktrees\keen-panini\build\develop" 1>"C:\Users\su-ko\freak_build.txt" 2>&1
echo EXIT_CODE=%ERRORLEVEL% >> "C:\Users\su-ko\freak_build.txt"
