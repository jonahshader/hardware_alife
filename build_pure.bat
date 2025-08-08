@echo off
echo Configuring pure software build...
cmake -B build_pure -DBUILD_SOFTWARE_PURE=ON -DBUILD_SOFTWARE_VERILATED=OFF

if %ERRORLEVEL% NEQ 0 (
    echo Configuration failed!
    pause
    exit /b %ERRORLEVEL%
)

echo Configuration successful!
echo To build, run: cmake --build build_pure --config Release
echo Or open build_pure/hardware_alife.sln in Visual Studio
pause