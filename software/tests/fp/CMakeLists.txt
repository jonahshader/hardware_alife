# Fixed Point Tests

# Create test executable
add_executable(fp_tests
  test_fp_core.cpp
  test_fp_math.cpp
  test_fp_io.cpp
  test_fp_literals.cpp
  test_fp_types.cpp
)

# Link with GTest
target_link_libraries(fp_tests
  gtest_main
  gtest
)

# Include directories - use absolute path from source root
target_include_directories(fp_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/software/pure/src  # Access to fp headers
)

# Set C++ standard
target_compile_features(fp_tests PRIVATE cxx_std_20)

# Discover and register tests
gtest_discover_tests(fp_tests)