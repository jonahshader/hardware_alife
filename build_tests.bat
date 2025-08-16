@echo off
echo Configuring tests build...
cmake -B build_tests -DBUILD_SOFTWARE_PURE=ON -DBUILD_SOFTWARE_VERILATED=OFF -DBUILD_TESTS=ON

if %ERRORLEVEL% NEQ 0 (
    echo Configuration failed!
    pause
    exit /b %ERRORLEVEL%
)

echo Configuration successful!
echo To build and run tests:
echo   cmake --build build_tests --config Debug
echo   cd build_tests
echo   ctest --output-on-failure
echo Or open build_tests/hardware_alife.sln in Visual Studio
pause