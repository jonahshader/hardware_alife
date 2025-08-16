#include <gtest/gtest.h>
#include "systems/fp/fp_types.h"

using namespace fp;

class FpTypesTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

// Test SFixed template alias
TEST_F(FpTypesTest, SFixedAlias) {
  SFixed<8, 8> fp1(3.14);
  EXPECT_TRUE(fp1.is_signed);
  EXPECT_EQ(fp1.upper_bits, 8);
  EXPECT_EQ(fp1.lower_bits, 8);
  EXPECT_NEAR(fp1.to_double(), 3.14, 0.01);
  
  SFixed<16, 16> fp2(-2.5);
  EXPECT_TRUE(fp2.is_signed);
  EXPECT_EQ(fp2.upper_bits, 16);
  EXPECT_EQ(fp2.lower_bits, 16);
  EXPECT_NEAR(fp2.to_double(), -2.5, 0.01);
}

// Test UFixed template alias
TEST_F(FpTypesTest, UFixedAlias) {
  UFixed<8, 8> fp1(3.14);
  EXPECT_FALSE(fp1.is_signed);
  EXPECT_EQ(fp1.upper_bits, 8);
  EXPECT_EQ(fp1.lower_bits, 8);
  EXPECT_NEAR(fp1.to_double(), 3.14, 0.01);
  
  UFixed<16, 16> fp2(100.5);
  EXPECT_FALSE(fp2.is_signed);
  EXPECT_EQ(fp2.upper_bits, 16);
  EXPECT_EQ(fp2.lower_bits, 16);
  EXPECT_NEAR(fp2.to_double(), 100.5, 0.01);
}

// Test Q8_8 type alias
TEST_F(FpTypesTest, Q8_8Type) {
  Q8_8 fp(5.25);
  
  EXPECT_TRUE(fp.is_signed);
  EXPECT_EQ(fp.upper_bits, 8);
  EXPECT_EQ(fp.lower_bits, 8);
  EXPECT_EQ(fp.total_bits, 16);
  EXPECT_NEAR(fp.to_double(), 5.25, 0.01);
  
  // Test range - Q8_8 should handle -128 to 127.99609375
  Q8_8 max_val(127.5);
  EXPECT_NEAR(max_val.to_double(), 127.5, 0.1);
  
  Q8_8 min_val(-128.0);
  EXPECT_NEAR(min_val.to_double(), -128.0, 0.1);
}

// Test Q16_16 type alias
TEST_F(FpTypesTest, Q16_16Type) {
  Q16_16 fp(1000.125);
  
  EXPECT_TRUE(fp.is_signed);
  EXPECT_EQ(fp.upper_bits, 16);
  EXPECT_EQ(fp.lower_bits, 16);
  EXPECT_EQ(fp.total_bits, 32);
  EXPECT_NEAR(fp.to_double(), 1000.125, 0.01);
  
  // Test higher precision
  Q16_16 precise(3.141592653589793);
  EXPECT_NEAR(precise.to_double(), 3.141592653589793, 0.0001);
}

// Test Q24_8 type alias
TEST_F(FpTypesTest, Q24_8Type) {
  Q24_8 fp(1000000.25);
  
  EXPECT_TRUE(fp.is_signed);
  EXPECT_EQ(fp.upper_bits, 24);
  EXPECT_EQ(fp.lower_bits, 8);
  EXPECT_EQ(fp.total_bits, 32);
  EXPECT_NEAR(fp.to_double(), 1000000.25, 0.1);
  
  // Test large range capability
  Q24_8 large(8000000.0);
  EXPECT_NEAR(large.to_double(), 8000000.0, 1.0);
}

// Test Q1_15 type alias (common in DSP)
TEST_F(FpTypesTest, Q1_15Type) {
  Q1_15 fp(0.5);
  
  EXPECT_TRUE(fp.is_signed);
  EXPECT_EQ(fp.upper_bits, 1);
  EXPECT_EQ(fp.lower_bits, 15);
  EXPECT_EQ(fp.total_bits, 16);
  EXPECT_NEAR(fp.to_double(), 0.5, 0.01);
  
  // Q1_15 range is -1 to ~0.999969
  Q1_15 max_val(0.99);
  EXPECT_NEAR(max_val.to_double(), 0.99, 0.01);
  
  Q1_15 min_val(-1.0);
  EXPECT_NEAR(min_val.to_double(), -1.0, 0.01);
}

// Test UQ8_8 type alias
TEST_F(FpTypesTest, UQ8_8Type) {
  UQ8_8 fp(200.75);
  
  EXPECT_FALSE(fp.is_signed);
  EXPECT_EQ(fp.upper_bits, 8);
  EXPECT_EQ(fp.lower_bits, 8);
  EXPECT_EQ(fp.total_bits, 16);
  EXPECT_NEAR(fp.to_double(), 200.75, 0.1);
  
  // Unsigned can represent larger positive values
  UQ8_8 large(255.0);
  EXPECT_NEAR(large.to_double(), 255.0, 0.1);
}

// Test UQ16_16 type alias
TEST_F(FpTypesTest, UQ16_16Type) {
  UQ16_16 fp(50000.5);
  
  EXPECT_FALSE(fp.is_signed);
  EXPECT_EQ(fp.upper_bits, 16);
  EXPECT_EQ(fp.lower_bits, 16);
  EXPECT_EQ(fp.total_bits, 32);
  EXPECT_NEAR(fp.to_double(), 50000.5, 0.1);
}

// Test saturating versions
TEST_F(FpTypesTest, SFixedSatType) {
  SFixedSat<4, 4> fp(100.0);  // Will saturate
  
  EXPECT_TRUE(fp.is_signed);
  EXPECT_EQ(fp.overflow_policy, OverflowPolicy::Saturate);
  
  // Should saturate to max value (7.9375 for 4.4 signed)
  EXPECT_NEAR(fp.to_double(), 7.9375, 0.1);
}

TEST_F(FpTypesTest, UFixedSatType) {
  UFixedSat<4, 4> fp(100.0);  // Will saturate
  
  EXPECT_FALSE(fp.is_signed);
  EXPECT_EQ(fp.overflow_policy, OverflowPolicy::Saturate);
  
  // Should saturate to max value (15.9375 for 4.4 unsigned)
  EXPECT_NEAR(fp.to_double(), 15.9375, 0.1);
}

// Test type compatibility and conversions
TEST_F(FpTypesTest, TypeConversions) {
  Q8_8 q8(3.25);
  Q16_16 q16(q8);  // Convert to higher precision
  
  EXPECT_NEAR(q8.to_double(), q16.to_double(), 0.01);
  
  Q24_8 q24(q16);  // Convert from 16.16 to 24.8
  EXPECT_NEAR(q16.to_double(), q24.to_double(), 0.01);
}

// Test arithmetic between different type aliases
TEST_F(FpTypesTest, ArithmeticBetweenTypes) {
  Q8_8 a(2.5);
  Q16_16 b(1.5);
  
  auto sum = a + b;
  EXPECT_NEAR(sum.to_double(), 4.0, 0.01);
  
  auto product = a * b;
  EXPECT_NEAR(product.to_double(), 3.75, 0.01);
}

// Test that unsigned types can't represent negative values properly
TEST_F(FpTypesTest, UnsignedLimitations) {
  UQ8_8 unsigned_fp(5.0);
  
  // Trying to create negative value should result in wrap-around or large positive
  UQ8_8 should_wrap(-1.0);
  EXPECT_GT(should_wrap.to_double(), 250.0);  // Wrapped to large positive
}

// Test precision differences between formats
TEST_F(FpTypesTest, PrecisionComparison) {
  double test_value = 3.141592653589793;
  
  Q8_8 low_prec(test_value);
  Q16_16 med_prec(test_value);
  Q1_15 dsp_prec(test_value);  // This will saturate to ~0.999
  
  // Higher precision should be more accurate (except Q1_15 which saturates)
  double low_error = std::abs(low_prec.to_double() - test_value);
  double med_error = std::abs(med_prec.to_double() - test_value);
  
  EXPECT_LT(med_error, low_error);
}

// Test that type aliases are indeed the expected underlying types
TEST_F(FpTypesTest, TypeAliasCorrectness) {
  static_assert(std::is_same_v<Q8_8, FixedPoint<8, 8, true, OverflowPolicy::Wrap>>);
  static_assert(std::is_same_v<Q16_16, FixedPoint<16, 16, true, OverflowPolicy::Wrap>>);
  static_assert(std::is_same_v<Q24_8, FixedPoint<24, 8, true, OverflowPolicy::Wrap>>);
  static_assert(std::is_same_v<Q1_15, FixedPoint<1, 15, true, OverflowPolicy::Wrap>>);
  
  static_assert(std::is_same_v<UQ8_8, FixedPoint<8, 8, false, OverflowPolicy::Wrap>>);
  static_assert(std::is_same_v<UQ16_16, FixedPoint<16, 16, false, OverflowPolicy::Wrap>>);
  
  static_assert(std::is_same_v<SFixedSat<8, 8>, FixedPoint<8, 8, true, OverflowPolicy::Saturate>>);
  static_assert(std::is_same_v<UFixedSat<8, 8>, FixedPoint<8, 8, false, OverflowPolicy::Saturate>>);
}

// Test common DSP operations with Q1_15
TEST_F(FpTypesTest, DSPOperations) {
  Q1_15 a(0.5);
  Q1_15 b(0.25);
  
  auto sum = a + b;
  EXPECT_NEAR(sum.to_double(), 0.75, 0.01);
  
  auto product = a * b;
  EXPECT_NEAR(product.to_double(), 0.125, 0.01);
  
  // Test that Q1_15 stays within [-1, 1) range
  EXPECT_LE(sum.to_double(), 1.0);
  EXPECT_GE(sum.to_double(), -1.0);
}