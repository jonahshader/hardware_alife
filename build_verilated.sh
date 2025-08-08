#!/bin/bash
echo "Configuring verilated software build..."
cmake -B build_verilated -DBUILD_SOFTWARE_PURE=OFF -DBUILD_SOFTWARE_VERILATED=ON

if [ $? -ne 0 ]; then
    echo "Configuration failed!"
    exit 1
fi

echo "Configuration successful!"
echo "To build, run: cmake --build build_verilated --config Release"
echo "Or run: make -C build_verilated -j$(nproc)"