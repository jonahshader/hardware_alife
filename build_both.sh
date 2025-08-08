#!/bin/bash
echo "Configuring both pure and verilated builds..."
cmake -B build_both -DBUILD_SOFTWARE_PURE=ON -DBUILD_SOFTWARE_VERILATED=ON

if [ $? -ne 0 ]; then
    echo "Configuration failed!"
    exit 1
fi

echo "Configuration successful!"
echo "To build, run: cmake --build build_both --config Release"
echo "Or run: make -C build_both -j$(nproc)"
echo ""
echo "This will build both targets:"
echo "  - software_pure"
echo "  - software_verilated"