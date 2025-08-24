# Verilated software implementation

# Find Verilator
find_package(verilator HINTS $ENV{VERILATOR_ROOT} REQUIRED)

# Generate VHDL modules using our pipeline (just the Verilog generation)
add_vhdl_module(rw_fb_test_top 
    VHDL_FILES gpu/gpu_types.vhd gpu/rw_fb.vhd tops/rw_fb_test_top.vhd
    TOP_ENTITY rw_fb_test_top
)

# Test executable with GTest
add_executable(verilated_tests 
    src/tests/rw_fb_test.cpp
)

# Use Verilator's CMake integration to handle the generated Verilog
add_custom_target(generate_verilog DEPENDS ${CMAKE_BINARY_DIR}/hardware/generated/verilog/rw_fb_test_top.v)
verilate(verilated_tests SOURCES ${CMAKE_BINARY_DIR}/hardware/generated/verilog/rw_fb_test_top.v TOP_MODULE rw_fb_test_top)
add_dependencies(verilated_tests generate_verilog)

# Link libraries for tests
target_link_libraries(verilated_tests PRIVATE
    gtest
    gtest_main
)

# Main executable (placeholder for now)
add_executable(software_verilated src/main.cpp)

# Link libraries for main executable
target_link_libraries(software_verilated PRIVATE
    SDL3::SDL3
    glad
    glm::glm
    OpenMP::OpenMP_CXX
)

# Include directories
target_include_directories(software_verilated PRIVATE 
    ${glad_SOURCE_DIR}/include
)

# Platform-specific configurations
if(WIN32)
  target_link_libraries(software_verilated PRIVATE SDL3::SDL3-shared)
  target_link_libraries(verilated_tests PRIVATE SDL3::SDL3-shared)
  
  if(MSVC)
    set_target_properties(software_verilated PROPERTIES
      LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
  endif()
  
  # Copy SDL3 DLL to output directory on Windows
  add_custom_command(TARGET software_verilated POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3::SDL3-shared>
    $<TARGET_FILE_DIR:software_verilated>
  )
endif()