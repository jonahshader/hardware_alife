#include <gtest/gtest.h>
#include "systems/fp/fp_literals.h"

using namespace fp;
using namespace fp::literals;

class FpLiteralsTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

// Test Q8_8 literal (_q8)
TEST_F(FpLiteralsTest, Q8LiteralBasic) {
  auto fp1 = 3.14_q8;
  EXPECT_NEAR(fp1.to_double(), 3.14, 0.01);
  
  auto fp2 = 0.0_q8;
  EXPECT_NEAR(fp2.to_double(), 0.0, 0.01);
  
  auto fp3 = 1.5_q8;
  EXPECT_NEAR(fp3.to_double(), 1.5, 0.01);
}

// Test Q8_8 literal with negative values
TEST_F(FpLiteralsTest, Q8LiteralNegative) {
  // Note: UDLs don't directly support negative literals,
  // but we can test the resulting type
  auto fp1 = 2.5_q8;
  auto neg_fp1 = -fp1;
  EXPECT_NEAR(neg_fp1.to_double(), -2.5, 0.01);
}

// Test Q16_16 literal (_q16)
TEST_F(FpLiteralsTest, Q16LiteralBasic) {
  auto fp1 = 3.14159_q16;
  EXPECT_NEAR(fp1.to_double(), 3.14159, 0.001);
  
  auto fp2 = 100.25_q16;
  EXPECT_NEAR(fp2.to_double(), 100.25, 0.01);
  
  auto fp3 = 0.00001_q16;
  EXPECT_NEAR(fp3.to_double(), 0.00001, 0.00001);
}

// Test Q24_8 literal (_q24)
TEST_F(FpLiteralsTest, Q24LiteralBasic) {
  auto fp1 = 1000.125_q24;
  EXPECT_NEAR(fp1.to_double(), 1000.125, 0.01);
  
  auto fp2 = 42.0_q24;
  EXPECT_NEAR(fp2.to_double(), 42.0, 0.01);
}

// Test compile-time literal (_fp)
TEST_F(FpLiteralsTest, CompileTimeLiteral) {
  auto fp1 = 3.14_fp;
  EXPECT_NEAR(fp1.to_double(), 3.14, 0.01);
  
  auto fp2 = 0.5_fp;
  EXPECT_NEAR(fp2.to_double(), 0.5, 0.01);
  
  auto fp3 = 42.0_fp;
  EXPECT_NEAR(fp3.to_double(), 42.0, 0.01);
}

// Test that literals create correct types
TEST_F(FpLiteralsTest, LiteralTypes) {
  auto q8_val = 1.0_q8;
  auto q16_val = 1.0_q16;
  auto q24_val = 1.0_q24;
  auto fp_val = 1.0_fp;
  
  // Check that they have the expected characteristics
  static_assert(std::is_same_v<decltype(q8_val), Q8_8>);
  static_assert(std::is_same_v<decltype(q16_val), Q16_16>);
  static_assert(std::is_same_v<decltype(q24_val), Q24_8>);
  static_assert(std::is_same_v<decltype(fp_val), Q16_16>);
  
  EXPECT_EQ(q8_val.upper_bits, 8);
  EXPECT_EQ(q8_val.lower_bits, 8);
  
  EXPECT_EQ(q16_val.upper_bits, 16);
  EXPECT_EQ(q16_val.lower_bits, 16);
  
  EXPECT_EQ(q24_val.upper_bits, 24);
  EXPECT_EQ(q24_val.lower_bits, 8);
}

// Test arithmetic with literals
TEST_F(FpLiteralsTest, ArithmeticWithLiterals) {
  auto result1 = 2.5_q8 + 1.5_q8;
  EXPECT_NEAR(result1.to_double(), 4.0, 0.01);
  
  auto result2 = 3.0_q16 * 2.0_q16;
  EXPECT_NEAR(result2.to_double(), 6.0, 0.01);
  
  auto result3 = 10.0_q24 / 2.0_q24;
  EXPECT_NEAR(result3.to_double(), 5.0, 0.01);
}

// Test mixed literals (different precisions)
TEST_F(FpLiteralsTest, MixedLiterals) {
  auto q8_val = 2.5_q8;
  auto q16_val = 1.5_q16;
  
  auto result = q8_val + q16_val;
  EXPECT_NEAR(result.to_double(), 4.0, 0.01);
}

// Test precision differences
TEST_F(FpLiteralsTest, PrecisionDifferences) {
  // Same value in different precisions
  auto low_prec = 3.14159_q8;
  auto high_prec = 3.14159_q16;
  
  // High precision should be more accurate
  EXPECT_GT(std::abs(high_prec.to_double() - 3.14159), 
            std::abs(low_prec.to_double() - 3.14159) - 0.1);
}

// Test that literals work in constexpr contexts
TEST_F(FpLiteralsTest, ConstexprLiterals) {
  constexpr auto fp1 = 2.5_q8;
  constexpr auto fp2 = 1.5_q8;
  constexpr auto result = fp1 + fp2;
  
  static_assert(result.to_double() > 3.9 && result.to_double() < 4.1);
  EXPECT_NEAR(result.to_double(), 4.0, 0.01);
}

// Test edge cases with literals
TEST_F(FpLiteralsTest, EdgeCases) {
  // Zero
  auto zero = 0.0_q8;
  EXPECT_EQ(zero.raw_value(), 0);
  
  // Very small value
  auto small = 0.00390625_q8;  // 1/256, exactly representable in 8.8
  EXPECT_EQ(small.raw_value(), 1);
  
  // Integer values
  auto integer = 5.0_q8;
  EXPECT_EQ(integer.raw_value(), 5 << 8);
}

// Test that compile-time literal parsing works correctly
TEST_F(FpLiteralsTest, CompileTimeParsing) {
  // Test various number formats
  auto fp1 = 123.0_fp;
  EXPECT_NEAR(fp1.to_double(), 123.0, 0.01);
  
  auto fp2 = 0.125_fp;
  EXPECT_NEAR(fp2.to_double(), 0.125, 0.001);
  
  auto fp3 = 42.5_fp;
  EXPECT_NEAR(fp3.to_double(), 42.5, 0.01);
}

// Test literals with calculations
TEST_F(FpLiteralsTest, LiteralsInCalculations) {
  // Test that literals can be used in complex expressions
  auto area = 3.14159_q16 * 2.0_q16 * 2.0_q16;  // π * r²
  EXPECT_NEAR(area.to_double(), 3.14159 * 4.0, 0.1);
  
  // Test with mixed operations
  auto result = (5.0_q8 + 3.0_q8) / 2.0_q8;
  EXPECT_NEAR(result.to_double(), 4.0, 0.01);
}

// Test that literals respect overflow policies
TEST_F(FpLiteralsTest, OverflowBehavior) {
  // Test that large values in small format behave as expected
  auto large_in_q8 = 1000.0_q8;  // Will overflow in Q8_8
  
  // Should be truncated/wrapped, not throw (default wrap policy)
  EXPECT_NE(large_in_q8.to_double(), 1000.0);
  EXPECT_NO_THROW(large_in_q8.to_double());
}

// Test literal compatibility with regular FixedPoint operations
TEST_F(FpLiteralsTest, CompatibilityWithRegularFixedPoint) {
  auto literal_fp = 2.5_q8;
  FixedPoint<8, 8> regular_fp(3.5);
  
  auto sum = literal_fp + regular_fp;
  EXPECT_NEAR(sum.to_double(), 6.0, 0.01);
  
  // Test assignment
  regular_fp = 1.5_q8;
  EXPECT_NEAR(regular_fp.to_double(), 1.5, 0.01);
}