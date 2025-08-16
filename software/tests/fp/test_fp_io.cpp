#include <gtest/gtest.h>
#include "systems/fp/fp_io.h"
#include <sstream>
#include <string>

using namespace fp;

class FpIoTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

// Test string conversion with to_string method
TEST_F(FpIoTest, ToStringMethod) {
  FixedPoint<8, 8> fp1(3.14159);
  
  // Default precision
  std::string str1 = fp1.to_string();
  EXPECT_FALSE(str1.empty());
  EXPECT_NE(str1.find("3.14"), std::string::npos);
  
  // Specific precision
  std::string str2 = fp1.to_string(2);
  EXPECT_EQ(str2, "3.14");
  
  std::string str3 = fp1.to_string(4);
  EXPECT_EQ(str3, "3.1416");
  
  // Zero precision
  std::string str4 = fp1.to_string(0);
  EXPECT_EQ(str4, "3");
}

// Test from_string static method
TEST_F(FpIoTest, FromStringMethod) {
  auto fp1 = FixedPoint<8, 8>::from_string("3.14");
  EXPECT_NEAR(fp1.to_double(), 3.14, 0.01);
  
  auto fp2 = FixedPoint<8, 8>::from_string("0.5");
  EXPECT_NEAR(fp2.to_double(), 0.5, 0.01);
  
  auto fp3 = FixedPoint<8, 8>::from_string("-2.75");
  EXPECT_NEAR(fp3.to_double(), -2.75, 0.01);
  
  auto fp4 = FixedPoint<8, 8>::from_string("0");
  EXPECT_NEAR(fp4.to_double(), 0.0, 0.01);
}

// Test from_string with invalid input
TEST_F(FpIoTest, FromStringInvalid) {
  EXPECT_THROW(FixedPoint<8, 8>::from_string("invalid"), std::invalid_argument);
  EXPECT_THROW(FixedPoint<8, 8>::from_string(""), std::invalid_argument);
  EXPECT_THROW(FixedPoint<8, 8>::from_string("3.14.15"), std::invalid_argument);
}

// Test free function to_string
TEST_F(FpIoTest, FreeToStringFunction) {
  FixedPoint<8, 8> fp(2.718);
  
  std::string str1 = to_string(fp);
  EXPECT_FALSE(str1.empty());
  
  std::string str2 = to_string(fp, 3);
  EXPECT_EQ(str2, "2.718");
}

// Test free function from_string
TEST_F(FpIoTest, FreeFromStringFunction) {
  auto fp = from_string<FixedPoint<8, 8>>("1.414");
  EXPECT_NEAR(fp.to_double(), 1.414, 0.01);
}

// Test stream output operator
TEST_F(FpIoTest, StreamOutput) {
  FixedPoint<8, 8> fp(3.14159);
  
  std::ostringstream oss;
  oss << fp;
  
  std::string result = oss.str();
  EXPECT_FALSE(result.empty());
  EXPECT_NE(result.find("3.14"), std::string::npos);
}

// Test stream input operator
TEST_F(FpIoTest, StreamInput) {
  std::istringstream iss("2.71828");
  FixedPoint<8, 8> fp;
  
  iss >> fp;
  
  EXPECT_NEAR(fp.to_double(), 2.71828, 0.01);
  EXPECT_TRUE(iss.good() || iss.eof());
}

// Test stream input with invalid data
TEST_F(FpIoTest, StreamInputInvalid) {
  std::istringstream iss("invalid_number");
  FixedPoint<8, 8> fp(1.0);  // Initialize with known value
  
  iss >> fp;
  
  EXPECT_TRUE(iss.fail());
  // fp should remain unchanged when input fails
  EXPECT_NEAR(fp.to_double(), 1.0, 0.01);
}

// Test round-trip conversion (string -> fp -> string)
TEST_F(FpIoTest, RoundTripStringConversion) {
  std::vector<std::string> test_values = {
    "0.0", "1.0", "-1.0", "3.14", "-2.718", "100.5", "0.125"
  };
  
  for (const auto& original : test_values) {
    auto fp = FixedPoint<8, 8>::from_string(original);
    std::string converted = fp.to_string(3);
    
    // Parse both as doubles to compare
    double orig_val = std::stod(original);
    double conv_val = std::stod(converted);
    
    EXPECT_NEAR(orig_val, conv_val, 0.01) << "Failed for input: " << original;
  }
}

// Test round-trip conversion (stream -> fp -> stream)
TEST_F(FpIoTest, RoundTripStreamConversion) {
  std::vector<double> test_values = {
    0.0, 1.0, -1.0, 3.14159, -2.71828, 100.5, 0.125, 42.0
  };
  
  for (double original : test_values) {
    // Create fp from original value
    FixedPoint<8, 8> fp1(original);
    
    // Stream out and back in
    std::ostringstream oss;
    oss << fp1;
    
    std::istringstream iss(oss.str());
    FixedPoint<8, 8> fp2;
    iss >> fp2;
    
    EXPECT_NEAR(fp1.to_double(), fp2.to_double(), 0.01)
      << "Failed for input: " << original;
  }
}

// Test with different fixed-point formats
TEST_F(FpIoTest, DifferentFormats) {
  // Test with different precisions
  FixedPoint<16, 16> high_prec(3.141592653589793);
  FixedPoint<4, 4> low_prec(3.141592653589793);
  
  std::string high_str = high_prec.to_string(6);
  std::string low_str = low_prec.to_string(6);
  
  // High precision should be more accurate
  EXPECT_NE(high_str.find("3.14159"), std::string::npos);
  EXPECT_NE(low_str.find("3.1"), std::string::npos);
}

// Test negative numbers
TEST_F(FpIoTest, NegativeNumbers) {
  FixedPoint<8, 8> negative(-5.25);
  
  std::string str = negative.to_string(2);
  EXPECT_EQ(str, "-5.25");
  
  auto parsed = FixedPoint<8, 8>::from_string("-5.25");
  EXPECT_NEAR(parsed.to_double(), -5.25, 0.01);
}

// Test very small numbers
TEST_F(FpIoTest, SmallNumbers) {
  FixedPoint<8, 8> small(0.00390625);  // 1/256, exactly representable in 8.8
  
  std::string str = small.to_string(8);
  auto parsed = FixedPoint<8, 8>::from_string(str);
  
  EXPECT_EQ(small.raw_value(), parsed.raw_value());
}

// Test zero
TEST_F(FpIoTest, Zero) {
  FixedPoint<8, 8> zero(0.0);
  
  std::string str = zero.to_string();
  EXPECT_EQ(str, "0");
  
  auto parsed = FixedPoint<8, 8>::from_string("0");
  EXPECT_EQ(zero.raw_value(), parsed.raw_value());
  
  // Test stream operations with zero
  std::ostringstream oss;
  oss << zero;
  EXPECT_EQ(oss.str(), "0");
  
  std::istringstream iss("0");
  FixedPoint<8, 8> zero2;
  iss >> zero2;
  EXPECT_EQ(zero.raw_value(), zero2.raw_value());
}