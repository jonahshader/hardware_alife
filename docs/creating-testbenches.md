# Creating VHDL Testbenches with Verilator

This guide explains how to create testbenches for VHDL modules using our VHDL→Verilog→Verilator→GTest pipeline.

## Overview

Our testbench workflow:
1. **VHDL Module** - Your design under test (DUT)
2. **Top-level Wrapper** - VHDL wrapper with fixed generics for testing
3. **GHDL Synthesis** - Convert VHDL to Verilog
4. **Verilator** - Convert Verilog to C++ model
5. **GTest** - C++ test harness

## Step-by-Step Process

### 1. Create the Top-Level Wrapper

Create a wrapper in `hardware/vhdl/tops/` that instantiates your DUT with specific test parameters.

**File:** `hardware/vhdl/tops/<module_name>_test_top.vhd`

```vhdl
-- Top-level wrapper for <module_name> for Verilator testing
-- Instantiates <module_name> with specific generics for testing

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- Include any work packages your module uses
use work.gpu_types.all;

entity <module_name>_test_top is
  port (
    -- Copy all ports from your DUT exactly
    -- Do NOT include generics here - they go in the instantiation
    clk    : in std_logic;
    reset  : in std_logic;
    -- ... other ports
  );
end entity <module_name>_test_top;

architecture wrapper of <module_name>_test_top is
begin
  -- Instantiate your DUT with fixed generics for testing
  u_dut : entity work.<module_name>
    generic map (
      WIDTH => 256,          -- Set specific values for testing
      HEIGHT => 256,
      DEPTH => 8
      -- ... other generics
    )
    port map (
      clk => clk,
      reset => reset
      -- ... connect all ports
    );
end architecture wrapper;
```

**Important Notes:**
- The wrapper entity should have the same ports as your DUT (without generics)
- Set specific generic values suitable for testing

### 2. Update CMakeLists.txt

Add your module to the VHDL pipeline in `software/verilated/CMakeLists.txt`:

```cmake
# Generate VHDL modules using our pipeline
add_vhdl_module(<module_name>_test_top
    VHDL_FILES gpu/gpu_types.vhd gpu/<module_name>.vhd tops/<module_name>_test_top.vhd
    TOP_ENTITY <module_name>_test_top
)

# Create test executable
add_executable(<module_name>_tests
    src/tests/<module_name>_test.cpp
)

# Use Verilator's CMake integration
add_custom_target(generate_<module_name>_verilog DEPENDS ${CMAKE_BINARY_DIR}/hardware/generated/verilog/<module_name>_test_top.v)
verilate(<module_name>_tests SOURCES ${CMAKE_BINARY_DIR}/hardware/generated/verilog/<module_name>_test_top.v TOP_MODULE <module_name>_test_top)
add_dependencies(<module_name>_tests generate_<module_name>_verilog)

# Link GTest
target_link_libraries(<module_name>_tests PRIVATE
    gtest
    gtest_main
)
```

**File Paths:**
- List all VHDL files your module depends on in `VHDL_FILES`
- Include package files (like `gpu_types.vhd`) first
- Include your main module file
- Include the wrapper file last

### 3. Create the C++ Test File

Create `software/verilated/src/tests/<module_name>_test.cpp`:

```cpp
#include <gtest/gtest.h>
#include <memory>
#include "V<module_name>_test_top.h"
#include "verilated.h"

class <ModuleName>Test : public ::testing::Test {
protected:
    std::unique_ptr<V<module_name>_test_top> dut;
    uint64_t time_counter;

    void SetUp() override {
        dut = std::make_unique<V<module_name>_test_top>();
        time_counter = 0;

        // Initialize inputs
        dut->clk = 0;
        dut->reset = 1;  // Start in reset
        // ... initialize other inputs

        // Initial evaluation
        dut->eval();
    }

    void TearDown() override {
        dut->final();
    }

    // Helper to advance simulation time
    void tick() {
        time_counter++;
        dut->clk = !dut->clk;
        dut->eval();
    }

    // Helper to perform full clock cycle
    void clock_cycle() {
        tick(); // Rising edge
        tick(); // Falling edge
    }

    // Helper to release reset
    void release_reset() {
        dut->reset = 1;
        clock_cycle();
        dut->reset = 0;
        clock_cycle();
    }
};

TEST_F(<ModuleName>Test, BasicInstantiation) {
    EXPECT_NE(dut, nullptr);
}

TEST_F(<ModuleName>Test, ResetBehavior) {
    release_reset();
    // Test that module comes out of reset properly
    // Check expected default output values
}

// Add more specific tests for your module's functionality
```

### 4. Handle VHDL Type Conversions

Verilator flattens VHDL records and arrays. For example:

**VHDL:**
```vhdl
signal pos : fb_vec_t;  -- record with x,y fields
signal rgb : rgb_t;     -- record with r,g,b fields
```

**Becomes in C++:**
```cpp
dut->pos_x = 10;
dut->pos_y = 20;
dut->rgb_r = 0xF;
dut->rgb_g = 0x8;
dut->rgb_b = 0x4;
```

### 5. Build and Test

Build the verilated implementation:
```bash
cmake -B build_verilated -DBUILD_SOFTWARE_VERILATED=ON
cmake --build build_verilated
```

Run your specific test:
```bash
./build_verilated/software/verilated/<module_name>_tests
```

Run all tests:
```bash
./build_verilated/software/verilated/verilated_tests
```

## Common Issues and Solutions

### GHDL Compilation Errors
- **Missing function bodies**: Ensure all functions in packages have implementations
- **Signal in default value**: Calculate addresses/values inside processes, not in variable declarations
- **Package dependencies**: List package files before modules that use them in `VHDL_FILES`

### Verilator Errors
- **Comment parsing**: Avoid underscores in directory names that appear in generated comments
- **Missing signals**: Check that wrapper port names exactly match your DUT
- **Type mismatches**: Use appropriate bit widths for std_logic_vector ports

### Test Failures
- **Timing**: Account for read latency and clock domain crossings
- **Reset**: Ensure proper reset sequences before testing functionality
- **Memory persistence**: Allow sufficient clock cycles for memory operations to complete

## Directory Structure

```
hardware/
├── vhdl/
│   ├── gpu/
│   │   ├── gpu_types.vhd          # Package files
│   │   └── <module_name>.vhd      # Your DUT
│   └── tops/
│       └── <module_name>_test_top.vhd  # Test wrapper
software/
└── verilated/
    ├── CMakeLists.txt             # Build configuration
    └── src/tests/
        └── <module_name>_test.cpp # GTest testbench
```

## Tips

1. **Start Simple**: Begin with basic instantiation and reset tests
2. **Use Helper Functions**: Create utilities for common operations (setting positions, RGB values, etc.)
3. **Account for Latency**: Many VHDL modules have pipeline delays - wait sufficient cycles
4. **Cross-Domain Signals**: Use multiple clock cycles for signals crossing clock domains
5. **Meaningful Generics**: Set test generics to values that make verification predictable
6. **Incremental Testing**: Add one test at a time and verify each passes before moving on

This workflow allows you to thoroughly test VHDL modules using familiar C++ testing frameworks while maintaining the hardware design in VHDL.
