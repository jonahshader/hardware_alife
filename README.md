# Hardware ALife

Artificial life/evolution simulator designed for both software and hardware implementations.

## Project Structure

- `software/pure/` - Pure C++ software implementation using SDL3
- `software/verilated/` - Verilator-based hardware simulation (WIP)
- `hardware/vhdl/` - VHDL hardware implementation (WIP)

## Prerequisites

- **CMake** (3.14 or later)
- **C++ Compiler** with C++20 support
  - Windows: Visual Studio 2019+ or clang
  - Linux/macOS: GCC 10+ or clang 12+
- **OpenMP** (for parallel processing)
- **Git** (for downloading dependencies)

All other dependencies (SDL3, GLAD, GLM) are automatically downloaded via CMake.

## Quick Build

### Windows
```batch
# Run build_pure.bat for pure software version
build_pure.bat

# Then build
cmake --build build_pure --config Release
```

### Linux/macOS
```bash
# Run build script for pure software version
./build_pure.sh

# Then build
cmake --build build_pure --config Release
```

## Manual Build

### Pure Software Version
```bash
# Configure
cmake -B build_pure -DBUILD_SOFTWARE_PURE=ON -DBUILD_SOFTWARE_VERILATED=OFF

# Build
cmake --build build_pure --config Release

# Run
./build_pure/software/pure/Release/software_pure.exe  # Windows
./build_pure/software/pure/software_pure             # Linux/macOS
```

### Both Versions (Pure + Verilated)
```bash
# Configure both
cmake -B build_both -DBUILD_SOFTWARE_PURE=ON -DBUILD_SOFTWARE_VERILATED=ON

# Build
cmake --build build_both --config Release
```

## Build Options

- `BUILD_SOFTWARE_PURE=ON` - Build pure C++ software version (default: ON)
- `BUILD_SOFTWARE_VERILATED=ON` - Build Verilator hardware simulation (default: OFF)

## Running the Software

The pure software version includes a test screen that demonstrates:
- **Framebuffer rendering** with various visual patterns
- **Interactive audio** with mouse-controlled panning and frequency
- **Game system architecture** with screen management

Move your mouse around the window to interact with both the visuals and audio.

## Development

The software uses a modular architecture designed for easy porting to hardware:
- `Game` class manages the main loop and screen stack
- `Framebuffer` provides pixel-perfect rendering
- `AudioManager` handles multi-source audio mixing
- `Screen` base class for different application states

This design allows the core simulation logic to be easily adapted for FPGA/VHDL implementation.
