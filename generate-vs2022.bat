@echo off
rem Generates a 32-bit Visual Studio 2022 solution for the client, launcher and server.
rem CEF (in-game browser UI) is left off; add -DM2O_CEF=1 to enable it.
if not exist build mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A Win32 -DM2O_SERVER=1 -DM2O_CLIENT=1
if errorlevel 1 (
    echo.
    echo CMake failed. Check that CMake 3.15+ and the "Desktop development with C++" workload are installed.
    pause
    exit /b 1
)
echo.
echo Done. Open build\m2o-reborn.sln, pick Release, and build.
pause
