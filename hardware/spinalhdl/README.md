# SpinalHDL GPU Implementation

This directory contains a SpinalHDL implementation of the GPU types with proper union support.

## Setup

1. Install sbt (Scala Build Tool):
   ```bash
   # Ubuntu/Debian
   sudo apt install sbt
   
   # Or download from https://www.scala-sbt.org/
   ```

2. Install dependencies:
   ```bash
   cd hardware/spinalhdl
   sbt compile
   ```

## Usage

Generate VHDL:
```bash
sbt "runMain GenerateGpu"
```

This will create VHDL files in `generated/vhdl/` that you can use alongside your existing VHDL code.

## Features

- Clean union types with `GpuInstr` discriminated by renderer type
- Type-safe access to instruction variants (`.asRect`, `.asCircle`, etc.)
- Generates synthesizable VHDL compatible with your existing toolchain
- Maintains the same bit widths as your original VHDL types

## Integration

The generated VHDL can be used in your existing GHDL/Vivado flow. SpinalHDL generates standard IEEE VHDL that works with all tools.