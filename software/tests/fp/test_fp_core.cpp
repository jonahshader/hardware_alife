#include <gtest/gtest.h>
#include "systems/fp/fp_core.h"

using namespace fp;

class FpCoreTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

// Test basic construction
TEST_F(FpCoreTest, BasicConstruction) {
  FixedPoint<8, 8> fp1;
  EXPECT_EQ(fp1.raw_value(), 0);
  
  FixedPoint<8, 8> fp2(5);
  EXPECT_EQ(fp2.raw_value(), 5 << 8);
  
  FixedPoint<8, 8> fp3(3.25);
  EXPECT_EQ(fp3.raw_value(), static_cast<int16_t>(3.25 * 256));
}

// Test from_raw constructor
TEST_F(FpCoreTest, FromRawConstruction) {
  auto fp = FixedPoint<8, 8>::from_raw(0x0340);  // 3.25 in 8.8 format
  EXPECT_EQ(fp.raw_value(), 0x0340);
  EXPECT_NEAR(fp.to_double(), 3.25, 0.01);
}

// Test arithmetic operations
TEST_F(FpCoreTest, BasicArithmetic) {
  FixedPoint<8, 8> a(2.5);
  FixedPoint<8, 8> b(1.25);
  
  auto sum = a + b;
  EXPECT_NEAR(sum.to_double(), 3.75, 0.01);
  
  auto diff = a - b;
  EXPECT_NEAR(diff.to_double(), 1.25, 0.01);
  
  auto prod = a * b;
  EXPECT_NEAR(prod.to_double(), 3.125, 0.01);
  
  auto quot = a / b;
  EXPECT_NEAR(quot.to_double(), 2.0, 0.01);
}

// Test comparison operations
TEST_F(FpCoreTest, Comparisons) {
  FixedPoint<8, 8> a(2.5);
  FixedPoint<8, 8> b(1.25);
  FixedPoint<8, 8> c(2.5);
  
  EXPECT_TRUE(a > b);
  EXPECT_TRUE(b < a);
  EXPECT_TRUE(a == c);
  EXPECT_TRUE(a >= c);
  EXPECT_TRUE(a <= c);
  EXPECT_FALSE(a < b);
}

// Test conversion operations
TEST_F(FpCoreTest, Conversions) {
  FixedPoint<8, 8> fp(5.75);
  
  EXPECT_EQ(static_cast<int>(fp), 5);
  EXPECT_NEAR(static_cast<double>(fp), 5.75, 0.01);
  EXPECT_NEAR(fp.to_float(), 5.75f, 0.01f);
  EXPECT_EQ(fp.to_int<int>(), 5);
}

// Test utility functions
TEST_F(FpCoreTest, UtilityFunctions) {
  FixedPoint<8, 8> fp(5.75);
  
  EXPECT_NEAR(fp.floor().to_double(), 5.0, 0.01);
  EXPECT_NEAR(fp.ceil().to_double(), 6.0, 0.01);
  EXPECT_NEAR(fp.round().to_double(), 6.0, 0.01);
  
  FixedPoint<8, 8> neg(-3.5);
  EXPECT_NEAR(neg.abs().to_double(), 3.5, 0.01);
}

// Test overflow policies
TEST_F(FpCoreTest, OverflowWrap) {
  using FP = FixedPoint<4, 4, true, OverflowPolicy::Wrap>;
  
  // Max value for 4.4 signed is 7.9375
  FP fp(8.0);  // Should wrap
  EXPECT_LT(fp.to_double(), 8.0);  // Wrapped value
}

TEST_F(FpCoreTest, OverflowSaturate) {
  using FP = FixedPoint<4, 4, true, OverflowPolicy::Saturate>;
  
  FP fp(100.0);  // Should saturate to max
  EXPECT_NEAR(fp.to_double(), 7.9375, 0.1);  // Max value for 4.4 signed
}

TEST_F(FpCoreTest, OverflowThrow) {
  using FP = FixedPoint<4, 4, true, OverflowPolicy::Throw>;
  
  EXPECT_THROW(FP fp(100.0), std::overflow_error);
}

// Test mixed-type operations
TEST_F(FpCoreTest, MixedTypes) {
  FixedPoint<8, 8> fp(2.5);
  
  auto result1 = fp + 3;
  EXPECT_NEAR(result1.to_double(), 5.5, 0.01);
  
  auto result2 = 3 + fp;
  EXPECT_NEAR(result2.to_double(), 5.5, 0.01);
  
  auto result3 = fp * 2;
  EXPECT_NEAR(result3.to_double(), 5.0, 0.01);
}

// Test cross-format conversions
TEST_F(FpCoreTest, CrossFormatConversion) {
  FixedPoint<8, 8> fp1(3.25);
  FixedPoint<16, 16> fp2(fp1);
  
  EXPECT_NEAR(fp1.to_double(), fp2.to_double(), 0.01);
  
  FixedPoint<4, 12> fp3(fp1);
  EXPECT_NEAR(fp1.to_double(), fp3.to_double(), 0.01);
}

// Test fused multiply-add
TEST_F(FpCoreTest, FusedMultiplyAdd) {
  FixedPoint<8, 8> a(2.0);
  FixedPoint<8, 8> b(3.0);
  FixedPoint<8, 8> c(1.5);
  
  auto result = a.fma(b, c);  // a * b + c = 2 * 3 + 1.5 = 7.5
  EXPECT_NEAR(result.to_double(), 7.5, 0.01);
}

// Test compound assignment operators
TEST_F(FpCoreTest, CompoundAssignment) {
  FixedPoint<8, 8> fp(2.0);
  
  fp += 1.5;
  EXPECT_NEAR(fp.to_double(), 3.5, 0.01);
  
  fp -= 0.5;
  EXPECT_NEAR(fp.to_double(), 3.0, 0.01);
  
  fp *= 2.0;
  EXPECT_NEAR(fp.to_double(), 6.0, 0.01);
  
  fp /= 3.0;
  EXPECT_NEAR(fp.to_double(), 2.0, 0.01);
}

// Test unary operators
TEST_F(FpCoreTest, UnaryOperators) {
  FixedPoint<8, 8> fp(2.5);
  
  auto neg = -fp;
  EXPECT_NEAR(neg.to_double(), -2.5, 0.01);
  
  auto pos = +fp;
  EXPECT_NEAR(pos.to_double(), 2.5, 0.01);
}

// Test string conversion
TEST_F(FpCoreTest, StringConversion) {
  FixedPoint<8, 8> fp(3.14159);
  
  std::string str = fp.to_string(2);
  EXPECT_EQ(str, "3.14");
  
  auto fp2 = FixedPoint<8, 8>::from_string("2.5");
  EXPECT_NEAR(fp2.to_double(), 2.5, 0.01);
}