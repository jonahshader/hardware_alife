#!/bin/bash
echo "Configuring pure software build..."
cmake -B build_pure -DBUILD_SOFTWARE_PURE=ON -DBUILD_SOFTWARE_VERILATED=OFF

if [ $? -ne 0 ]; then
    echo "Configuration failed!"
    exit 1
fi

echo "Configuration successful!"
echo "To build, run: cmake --build build_pure --config Release"
echo "Or run: make -C build_pure -j$(nproc)"