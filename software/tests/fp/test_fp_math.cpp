#include <gtest/gtest.h>
#include "systems/fp/fp_math.h"
#include <cmath>

using namespace fp;

class FpMathTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
  
  // Helper to compare with standard library functions
  template<typename FP>
  void ExpectNear(const FP& actual, double expected, double tolerance = 0.1) {
    EXPECT_NEAR(actual.to_double(), expected, tolerance);
  }
};

// Test sqrt function
TEST_F(FpMathTest, SqrtFunction) {
  FixedPoint<8, 8> fp1(4.0);
  auto result1 = sqrt(fp1);
  ExpectNear(result1, 2.0, 0.01);
  
  FixedPoint<8, 8> fp2(9.0);
  auto result2 = sqrt(fp2);
  ExpectNear(result2, 3.0, 0.01);
  
  FixedPoint<8, 8> fp3(2.0);
  auto result3 = sqrt(fp3);
  ExpectNear(result3, std::sqrt(2.0), 0.01);
  
  // Test sqrt(0)
  FixedPoint<8, 8> zero(0.0);
  auto result_zero = sqrt(zero);
  ExpectNear(result_zero, 0.0, 0.01);
}

TEST_F(FpMathTest, SqrtNegativeThrows) {
  using FP = FixedPoint<8, 8, true, OverflowPolicy::Throw>;
  FP negative(-1.0);
  EXPECT_THROW(sqrt(negative), std::domain_error);
}

TEST_F(FpMathTest, SqrtNegativeWraps) {
  using FP = FixedPoint<8, 8, true, OverflowPolicy::Wrap>;
  FP negative(-1.0);
  auto result = sqrt(negative);
  EXPECT_EQ(result.raw_value(), 0);
}

// Test trigonometric functions
TEST_F(FpMathTest, SinFunction) {
  FixedPoint<8, 8> fp1(0.0);
  auto result1 = sin(fp1);
  ExpectNear(result1, 0.0, 0.01);
  
  FixedPoint<8, 8> fp2(1.5708);  // π/2
  auto result2 = sin(fp2);
  ExpectNear(result2, 1.0, 0.1);
  
  FixedPoint<8, 8> fp3(3.14159);  // π
  auto result3 = sin(fp3);
  ExpectNear(result3, 0.0, 0.1);
}

TEST_F(FpMathTest, CosFunction) {
  FixedPoint<8, 8> fp1(0.0);
  auto result1 = cos(fp1);
  ExpectNear(result1, 1.0, 0.1);
  
  FixedPoint<8, 8> fp2(1.5708);  // π/2
  auto result2 = cos(fp2);
  ExpectNear(result2, 0.0, 0.2);
  
  FixedPoint<8, 8> fp3(3.14159);  // π
  auto result3 = cos(fp3);
  ExpectNear(result3, -1.0, 0.2);
}

TEST_F(FpMathTest, TanFunction) {
  FixedPoint<8, 8> fp1(0.0);
  auto result1 = tan(fp1);
  ExpectNear(result1, 0.0, 0.01);
  
  FixedPoint<8, 8> fp2(0.7854);  // π/4
  auto result2 = tan(fp2);
  ExpectNear(result2, 1.0, 0.2);
  
  // Test behavior near π/2 (where tan is undefined)
  using FP = FixedPoint<8, 8, true, OverflowPolicy::Throw>;
  FP fp3(1.5708);  // π/2
  EXPECT_THROW(tan(fp3), std::domain_error);
}

// Test exponential function
TEST_F(FpMathTest, ExpFunction) {
  FixedPoint<8, 8> fp1(0.0);
  auto result1 = exp(fp1);
  ExpectNear(result1, 1.0, 0.01);
  
  FixedPoint<8, 8> fp2(1.0);
  auto result2 = exp(fp2);
  ExpectNear(result2, std::exp(1.0), 0.1);
  
  FixedPoint<8, 8> fp3(2.0);
  auto result3 = exp(fp3);
  ExpectNear(result3, std::exp(2.0), 0.5);
}

// Test natural logarithm
TEST_F(FpMathTest, LogFunction) {
  FixedPoint<8, 8> fp1(1.0);
  auto result1 = log(fp1);
  ExpectNear(result1, 0.0, 0.1);
  
  FixedPoint<8, 8> fp2(2.71828);  // e
  auto result2 = log(fp2);
  ExpectNear(result2, 1.0, 0.2);
  
  FixedPoint<8, 8> fp3(10.0);
  auto result3 = log(fp3);
  ExpectNear(result3, std::log(10.0), 0.2);
}

TEST_F(FpMathTest, LogNegativeThrows) {
  using FP = FixedPoint<8, 8, true, OverflowPolicy::Throw>;
  FP negative(-1.0);
  EXPECT_THROW(log(negative), std::domain_error);
  
  FP zero(0.0);
  EXPECT_THROW(log(zero), std::domain_error);
}

// Test power function
TEST_F(FpMathTest, PowFunction) {
  FixedPoint<8, 8> base1(2.0);
  FixedPoint<8, 8> exp1(3.0);
  auto result1 = pow(base1, exp1);
  ExpectNear(result1, 8.0, 0.5);
  
  FixedPoint<8, 8> base2(3.0);
  FixedPoint<8, 8> exp2(2.0);
  auto result2 = pow(base2, exp2);
  ExpectNear(result2, 9.0, 0.5);
  
  FixedPoint<8, 8> base3(4.0);
  FixedPoint<8, 8> exp3(0.5);
  auto result3 = pow(base3, exp3);
  ExpectNear(result3, 2.0, 0.2);
}

TEST_F(FpMathTest, PowNegativeBaseThrows) {
  using FP = FixedPoint<8, 8, true, OverflowPolicy::Throw>;
  FP negative(-2.0);
  FP exponent(2.0);
  EXPECT_THROW(pow(negative, exponent), std::domain_error);
}

// Test mathematical identities
TEST_F(FpMathTest, MathematicalIdentities) {
  FixedPoint<8, 8> angle(0.5);
  
  // sin²(x) + cos²(x) = 1
  auto sin_val = sin(angle);
  auto cos_val = cos(angle);
  auto identity = sin_val * sin_val + cos_val * cos_val;
  ExpectNear(identity, 1.0, 0.2);
  
  // tan(x) = sin(x) / cos(x)
  auto tan_val = tan(angle);
  auto tan_calculated = sin_val / cos_val;
  EXPECT_NEAR(tan_val.to_double(), tan_calculated.to_double(), 0.2);
}

// Test exp and log are inverses
TEST_F(FpMathTest, ExpLogInverse) {
  FixedPoint<8, 8> value(2.0);
  
  auto exp_log = exp(log(value));
  ExpectNear(exp_log, 2.0, 0.2);
  
  auto log_exp = log(exp(value));
  ExpectNear(log_exp, 2.0, 0.2);
}

// Test edge cases with small values
TEST_F(FpMathTest, SmallValues) {
  FixedPoint<8, 8> small(0.01);
  
  // sin(x) ≈ x for small x
  auto sin_small = sin(small);
  ExpectNear(sin_small, 0.01, 0.001);
  
  // exp(x) ≈ 1 + x for small x
  auto exp_small = exp(small);
  ExpectNear(exp_small, 1.01, 0.01);
}

// Test precision with different fixed-point formats
TEST_F(FpMathTest, DifferentPrecision) {
  // Higher precision format
  FixedPoint<16, 16> high_prec(3.14159265);
  auto sin_high = sin(high_prec);
  
  // Lower precision format
  FixedPoint<8, 8> low_prec(3.14159265);
  auto sin_low = sin(low_prec);
  
  // High precision should be more accurate
  double expected = std::sin(3.14159265);
  double high_error = std::abs(sin_high.to_double() - expected);
  double low_error = std::abs(sin_low.to_double() - expected);
  
  EXPECT_LT(high_error, low_error);
}