#pragma once

#include <array>
#include <bit>
#include <cmath>
#include <compare>
#include <concepts>
#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace fp {

// Overflow handling policies
enum class OverflowPolicy {
  Wrap,     // Wrap around (default behavior)
  Saturate, // Clamp to min/max values
  Throw     // Throw exception on overflow
};

// Concepts
template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept floating_point = std::is_floating_point_v<T>;

// Helper to get the appropriate storage type based on total bits
template <size_t Bits> struct storage_type {
  using type = std::conditional_t<
      Bits <= 8, int8_t,
      std::conditional_t<
          Bits <= 16, int16_t,
          std::conditional_t<Bits <= 32, int32_t, std::conditional_t<Bits <= 64, int64_t, void>>>>;
};

template <size_t Bits> struct unsigned_storage_type {
  using type = std::conditional_t<
      Bits <= 8, uint8_t,
      std::conditional_t<Bits <= 16, uint16_t,
                         std::conditional_t<Bits <= 32, uint32_t,
                                            std::conditional_t<Bits <= 64, uint64_t, void>>>>;
};

// Promotion type for multiplication
template <typename T1, typename T2>
using promote_t = std::conditional_t<
    sizeof(T1) + sizeof(T2) <= 4, int32_t,
    std::conditional_t<sizeof(T1) + sizeof(T2) <= 8, int64_t, int64_t>>;

// Forward declaration
template <size_t UpperBits, size_t LowerBits, bool Signed = true,
          OverflowPolicy Policy = OverflowPolicy::Wrap>
class FixedPoint;

// Type traits for detecting fixed point types
template <typename T> struct is_fixed_point : std::false_type {};

template <size_t U, size_t L, bool S, OverflowPolicy P>
struct is_fixed_point<FixedPoint<U, L, S, P>> : std::true_type {};

template <typename T> inline constexpr bool is_fixed_point_v = is_fixed_point<T>::value;

template <typename T>
concept fixed_point_type = is_fixed_point_v<T>;

// Main FixedPoint class
template <size_t UpperBits, size_t LowerBits, bool Signed, OverflowPolicy Policy> class FixedPoint {
public:
  static constexpr size_t upper_bits = UpperBits;
  static constexpr size_t lower_bits = LowerBits;
  static constexpr size_t total_bits = UpperBits + LowerBits;
  static constexpr bool is_signed = Signed;
  static constexpr OverflowPolicy overflow_policy = Policy;

  static_assert(total_bits <= 64, "Total bits cannot exceed 64");
  static_assert(total_bits > 0, "Total bits must be at least 1");

  using storage_t = std::conditional_t<Signed, typename storage_type<total_bits>::type,
                                       typename unsigned_storage_type<total_bits>::type>;

  using signed_storage_t = typename storage_type<total_bits>::type;
  using unsigned_storage_t = typename unsigned_storage_type<total_bits>::type;

  static constexpr storage_t scale_factor = storage_t(1) << lower_bits;

private:
  storage_t value;

  // Overflow handling
  template <typename T> static constexpr storage_t handle_overflow(T val) {
    if constexpr (Policy == OverflowPolicy::Wrap) {
      return static_cast<storage_t>(val);
    } else if constexpr (Policy == OverflowPolicy::Saturate) {
      constexpr auto max_val = std::numeric_limits<storage_t>::max();
      constexpr auto min_val = std::numeric_limits<storage_t>::min();
      if (val > max_val)
        return max_val;
      if (val < min_val)
        return min_val;
      return static_cast<storage_t>(val);
    } else { // Throw
      constexpr auto max_val = std::numeric_limits<storage_t>::max();
      constexpr auto min_val = std::numeric_limits<storage_t>::min();
      if (val > max_val || val < min_val) {
        throw std::overflow_error("Fixed point overflow");
      }
      return static_cast<storage_t>(val);
    }
  }

public:
  // Constructors
  constexpr FixedPoint() : value(0) {}

  // Constructor from raw value (private, use from_raw)
  struct raw_tag {};
  constexpr FixedPoint(storage_t raw, raw_tag) : value(raw) {}

  // Named constructor for raw values
  static consteval FixedPoint from_raw(storage_t raw) {
    return FixedPoint(raw, raw_tag{});
  }

  // Constructor from integer types
  template <integral T> constexpr FixedPoint(T val) {
    using promoted_t = std::conditional_t<sizeof(T) < sizeof(storage_t),
                                          std::make_signed_t<int64_t>, std::make_signed_t<int64_t>>;
    promoted_t temp = static_cast<promoted_t>(val) << lower_bits;
    value = handle_overflow(temp);
  }

  // Constructor from floating point types
  template <floating_point T> constexpr FixedPoint(T val) {
    auto temp = static_cast<int64_t>(val * scale_factor);
    value = handle_overflow(temp);
  }

  // Constructor from other FixedPoint types
  template <size_t U2, size_t L2, bool S2, OverflowPolicy P2>
  constexpr FixedPoint(const FixedPoint<U2, L2, S2, P2> &other) {
    if constexpr (L2 == lower_bits) {
      // Same fractional bits, direct copy with overflow check
      value = handle_overflow(static_cast<int64_t>(other.raw_value()));
    } else if constexpr (L2 > lower_bits) {
      // More fractional bits in source, shift right
      constexpr size_t shift = L2 - lower_bits;
      auto temp = static_cast<int64_t>(other.raw_value()) >> shift;
      value = handle_overflow(temp);
    } else {
      // Fewer fractional bits in source, shift left
      constexpr size_t shift = lower_bits - L2;
      auto temp = static_cast<int64_t>(other.raw_value()) << shift;
      value = handle_overflow(temp);
    }
  }

  // Conversion operators
  template <floating_point T> explicit operator T() const {
    return static_cast<T>(value) / scale_factor;
  }

  template <integral T> explicit operator T() const {
    return static_cast<T>(value >> lower_bits);
  }

  // Three-way comparison
  template <fixed_point_type FP> constexpr auto operator<=>(const FP &rhs) const {
    constexpr size_t max_lower = std::max(lower_bits, FP::lower_bits);
    using result_t = FixedPoint<std::max(upper_bits, FP::upper_bits), max_lower, true, Policy>;
    result_t lhs_conv(*this);
    result_t rhs_conv(rhs);
    return lhs_conv.raw_value() <=> rhs_conv.raw_value();
  }

  template <fixed_point_type FP> constexpr bool operator==(const FP &rhs) const {
    return (*this <=> rhs) == 0;
  }

  // Comparison with integers
  template <integral T> constexpr auto operator<=>(T rhs) const {
    return *this <=> FixedPoint<sizeof(T) * 8, 0, std::is_signed_v<T>, Policy>(rhs);
  }

  template <integral T> constexpr bool operator==(T rhs) const {
    return (*this <=> rhs) == 0;
  }

  // Arithmetic operators with optimized promotion
  template <fixed_point_type FP> constexpr auto operator+(const FP &rhs) const {
    constexpr size_t result_upper = std::max(upper_bits, FP::upper_bits) + 1;
    constexpr size_t result_lower = std::max(lower_bits, FP::lower_bits);
    constexpr bool result_signed = is_signed || FP::is_signed;
    using result_t = FixedPoint<result_upper, result_lower, result_signed, Policy>;

    result_t lhs_conv(*this);
    result_t rhs_conv(rhs);

    using op_t = promote_t<storage_t, typename FP::storage_t>;
    auto result_val =
        static_cast<op_t>(lhs_conv.raw_value()) + static_cast<op_t>(rhs_conv.raw_value());
    return result_t::from_raw(result_t::handle_overflow(result_val));
  }

  template <fixed_point_type FP> constexpr auto operator-(const FP &rhs) const {
    constexpr size_t result_upper = std::max(upper_bits, FP::upper_bits) + 1;
    constexpr size_t result_lower = std::max(lower_bits, FP::lower_bits);
    constexpr bool result_signed = true;
    using result_t = FixedPoint<result_upper, result_lower, result_signed, Policy>;

    result_t lhs_conv(*this);
    result_t rhs_conv(rhs);

    using op_t = promote_t<storage_t, typename FP::storage_t>;
    auto result_val =
        static_cast<op_t>(lhs_conv.raw_value()) - static_cast<op_t>(rhs_conv.raw_value());
    return result_t::from_raw(result_t::handle_overflow(result_val));
  }

  template <fixed_point_type FP> constexpr auto operator*(const FP &rhs) const {
    constexpr size_t result_upper = upper_bits + FP::upper_bits;
    constexpr size_t result_lower = lower_bits + FP::lower_bits;
    constexpr bool result_signed = is_signed || FP::is_signed;

    using op_t = promote_t<storage_t, typename FP::storage_t>;
    auto result_val = static_cast<op_t>(value) * static_cast<op_t>(rhs.raw_value());

    using temp_result_t = FixedPoint<result_upper, result_lower, result_signed, Policy>;
    return temp_result_t::from_raw(temp_result_t::handle_overflow(result_val));
  }

  template <fixed_point_type FP> constexpr auto operator/(const FP &rhs) const {
    if (rhs.raw_value() == 0) {
      if constexpr (Policy == OverflowPolicy::Throw) {
        throw std::domain_error("Division by zero");
      }
      return *this;
    }

    constexpr size_t result_upper = upper_bits + FP::lower_bits;
    constexpr size_t result_lower = lower_bits + FP::upper_bits;
    constexpr bool result_signed = is_signed || FP::is_signed;
    using result_t = FixedPoint<result_upper, result_lower, result_signed, Policy>;

    using op_t = promote_t<storage_t, typename FP::storage_t>;
    auto scaled_num = static_cast<op_t>(value) << (FP::lower_bits + FP::upper_bits);
    auto result_val = scaled_num / rhs.raw_value();

    return result_t::from_raw(result_t::handle_overflow(result_val));
  }

  // Explicit result type multiplication
  template <typename ResultType, fixed_point_type FP>
  constexpr ResultType multiply_as(const FP &rhs) const {
    auto temp = *this * rhs;
    return ResultType(temp);
  }

  // Fused multiply-add
  template <fixed_point_type FP1, fixed_point_type FP2>
  constexpr auto fma(const FP1 &mul, const FP2 &add) const {
    // Perform multiplication without intermediate rounding
    using op_t = promote_t<storage_t, typename FP1::storage_t>;
    auto mul_result = static_cast<op_t>(value) * static_cast<op_t>(mul.raw_value());

    // Determine result type
    constexpr size_t temp_upper = upper_bits + FP1::upper_bits;
    constexpr size_t temp_lower = lower_bits + FP1::lower_bits;
    constexpr size_t result_upper = std::max(temp_upper, FP2::upper_bits) + 1;
    constexpr size_t result_lower = std::max(temp_lower, FP2::lower_bits);
    constexpr bool result_signed = is_signed || FP1::is_signed || FP2::is_signed;
    using result_t = FixedPoint<result_upper, result_lower, result_signed, Policy>;

    // Convert add operand and perform addition
    if constexpr (temp_lower > FP2::lower_bits) {
      auto shifted_add = static_cast<op_t>(add.raw_value()) << (temp_lower - FP2::lower_bits);
      return result_t::from_raw(result_t::handle_overflow(mul_result + shifted_add));
    } else if constexpr (temp_lower < FP2::lower_bits) {
      mul_result <<= (FP2::lower_bits - temp_lower);
      return result_t::from_raw(result_t::handle_overflow(mul_result + add.raw_value()));
    } else {
      return result_t::from_raw(result_t::handle_overflow(mul_result + add.raw_value()));
    }
  }

  // Integer arithmetic
  template <integral T> constexpr auto operator+(T rhs) const {
    return *this + FixedPoint<sizeof(T) * 8, 0, std::is_signed_v<T>, Policy>(rhs);
  }

  template <integral T> constexpr auto operator-(T rhs) const {
    return *this - FixedPoint<sizeof(T) * 8, 0, std::is_signed_v<T>, Policy>(rhs);
  }

  template <integral T> constexpr auto operator*(T rhs) const {
    return *this * FixedPoint<sizeof(T) * 8, 0, std::is_signed_v<T>, Policy>(rhs);
  }

  template <integral T> constexpr auto operator/(T rhs) const {
    return *this / FixedPoint<sizeof(T) * 8, 0, std::is_signed_v<T>, Policy>(rhs);
  }

  // Compound assignment operators
  template <typename T> constexpr FixedPoint &operator+=(const T &rhs) {
    *this = *this + rhs;
    return *this;
  }

  template <typename T> constexpr FixedPoint &operator-=(const T &rhs) {
    *this = *this - rhs;
    return *this;
  }

  template <typename T> constexpr FixedPoint &operator*=(const T &rhs) {
    *this = *this * rhs;
    return *this;
  }

  template <typename T> constexpr FixedPoint &operator/=(const T &rhs) {
    *this = *this / rhs;
    return *this;
  }

  // Unary operators
  constexpr FixedPoint operator-() const {
    if constexpr (!Signed && Policy == OverflowPolicy::Throw) {
      if (value != 0) {
        throw std::overflow_error("Negation of unsigned non-zero value");
      }
    }
    return from_raw(handle_overflow(-static_cast<int64_t>(value)));
  }

  constexpr FixedPoint operator+() const {
    return *this;
  }

  // Utility functions
  constexpr storage_t raw_value() const {
    return value;
  }

  constexpr auto abs() const {
    if constexpr (Signed) {
      return from_raw(value >= 0 ? value : handle_overflow(-static_cast<int64_t>(value)));
    } else {
      return *this;
    }
  }

  constexpr auto floor() const {
    return FixedPoint<upper_bits, 0, Signed, Policy>(static_cast<storage_t>(value >> lower_bits));
  }

  constexpr auto ceil() const {
    auto mask = (storage_t(1) << lower_bits) - 1;
    if ((value & mask) == 0) {
      return floor();
    }
    return FixedPoint<upper_bits, 0, Signed, Policy>(
        static_cast<storage_t>((value >> lower_bits) + 1));
  }

  constexpr auto round() const {
    auto half = storage_t(1) << (lower_bits - 1);
    return FixedPoint<upper_bits, 0, Signed, Policy>(
        static_cast<storage_t>((value + half) >> lower_bits));
  }

  // Conversion helpers
  template <floating_point T> static constexpr FixedPoint from_float(T val) {
    return FixedPoint(val);
  }

  template <floating_point T> constexpr T to_float() const {
    return static_cast<T>(*this);
  }

  constexpr float to_float() const {
    return static_cast<float>(*this);
  }

  constexpr double to_double() const {
    return static_cast<double>(*this);
  }

  template <integral T> constexpr T to_int() const {
    return static_cast<T>(*this);
  }

  // String conversion
  std::string to_string(int precision = -1) const {
    std::ostringstream oss;
    if (precision >= 0) {
      oss.precision(precision);
      oss << std::fixed;
    }
    oss << to_double();
    return oss.str();
  }

  static FixedPoint from_string(const std::string &str) {
    std::istringstream iss(str);
    double val;
    if (!(iss >> val)) {
      throw std::invalid_argument("Invalid fixed-point string");
    }
    return FixedPoint(val);
  }
};

// Non-member operators for integer on left side
template <integral T, fixed_point_type FP> constexpr auto operator+(T lhs, const FP &rhs) {
  return FixedPoint<sizeof(T) * 8, 0, std::is_signed_v<T>, FP::overflow_policy>(lhs) + rhs;
}

template <integral T, fixed_point_type FP> constexpr auto operator-(T lhs, const FP &rhs) {
  return FixedPoint<sizeof(T) * 8, 0, std::is_signed_v<T>, FP::overflow_policy>(lhs) - rhs;
}

template <integral T, fixed_point_type FP> constexpr auto operator*(T lhs, const FP &rhs) {
  return FixedPoint<sizeof(T) * 8, 0, std::is_signed_v<T>, FP::overflow_policy>(lhs) * rhs;
}

template <integral T, fixed_point_type FP> constexpr auto operator/(T lhs, const FP &rhs) {
  return FixedPoint<sizeof(T) * 8, 0, std::is_signed_v<T>, FP::overflow_policy>(lhs) / rhs;
}

// Comparison operators with integer on left
template <integral T, fixed_point_type FP> constexpr auto operator<=>(T lhs, const FP &rhs) {
  return FixedPoint<sizeof(T) * 8, 0, std::is_signed_v<T>, FP::overflow_policy>(lhs) <=> rhs;
}

template <integral T, fixed_point_type FP> constexpr bool operator==(T lhs, const FP &rhs) {
  return (lhs <=> rhs) == 0;
}

// Stream operators
template <fixed_point_type FP> std::ostream &operator<<(std::ostream &os, const FP &fp) {
  return os << fp.to_double();
}

template <fixed_point_type FP> std::istream &operator>>(std::istream &is, FP &fp) {
  double val;
  if (is >> val) {
    fp = FP(val);
  }
  return is;
}

// Common type aliases
template <size_t IntBits, size_t FracBits> using SFixed = FixedPoint<IntBits, FracBits, true>;

template <size_t IntBits, size_t FracBits> using UFixed = FixedPoint<IntBits, FracBits, false>;

// Common fixed point types
using Q8_8 = SFixed<8, 8>;     // 8.8 signed fixed point
using Q16_16 = SFixed<16, 16>; // 16.16 signed fixed point
using Q24_8 = SFixed<24, 8>;   // 24.8 signed fixed point
using Q1_15 = SFixed<1, 15>;   // 1.15 signed fixed point (common in DSP)

using UQ8_8 = UFixed<8, 8>;     // 8.8 unsigned fixed point
using UQ16_16 = UFixed<16, 16>; // 16.16 unsigned fixed point

// Saturating versions
template <size_t IntBits, size_t FracBits>
using SFixedSat = FixedPoint<IntBits, FracBits, true, OverflowPolicy::Saturate>;

template <size_t IntBits, size_t FracBits>
using UFixedSat = FixedPoint<IntBits, FracBits, false, OverflowPolicy::Saturate>;

// Math functions
template <fixed_point_type FP> constexpr auto sqrt(const FP &x) {
  if (x.raw_value() < 0) {
    if constexpr (FP::overflow_policy == OverflowPolicy::Throw) {
      throw std::domain_error("sqrt of negative number");
    }
    return FP::from_raw(0);
  }

  // Newton-Raphson method
  if (x.raw_value() == 0)
    return x;

  auto guess = x;
  auto prev = FP::from_raw(0);

  constexpr int max_iterations = 20;
  for (int i = 0; i < max_iterations && guess != prev; ++i) {
    prev = guess;
    guess = (guess + x / guess) * FP::from_float(0.5);
  }

  return guess;
}

// Trigonometric functions using CORDIC
template <fixed_point_type FP> constexpr auto sin(const FP &x) {
  // Normalize to [-pi, pi]
  constexpr auto pi = FP::from_float(3.14159265358979323846);
  constexpr auto two_pi = FP::from_float(6.28318530717958647692);

  auto normalized = x;
  while (normalized > pi)
    normalized -= two_pi;
  while (normalized < -pi)
    normalized += two_pi;

  // Taylor series approximation for small angles
  auto x2 = normalized * normalized;
  auto result = normalized;
  auto term = normalized;

  // sin(x) = x - x³/3! + x⁵/5! - x⁷/7! + ...
  for (int i = 1; i <= 5; ++i) {
    term = term * x2 * FP::from_float(-1.0 / ((2 * i) * (2 * i + 1)));
    result += term;
  }

  return result;
}

template <fixed_point_type FP> constexpr auto cos(const FP &x) {
  constexpr auto pi_2 = FP::from_float(1.57079632679489661923);
  return sin(x + pi_2);
}

template <fixed_point_type FP> constexpr auto tan(const FP &x) {
  auto c = cos(x);
  if (c.raw_value() == 0) {
    if constexpr (FP::overflow_policy == OverflowPolicy::Throw) {
      throw std::domain_error("tan undefined");
    }
    return FP::from_raw(std::numeric_limits<typename FP::storage_t>::max());
  }
  return sin(x) / c;
}

// Exponential functions
template <fixed_point_type FP> constexpr auto exp(const FP &x) {
  // exp(x) using Taylor series
  auto result = FP::from_float(1.0);
  auto term = FP::from_float(1.0);

  for (int i = 1; i <= 10; ++i) {
    term = term * x / FP::from_float(i);
    result += term;
    if (term.abs().raw_value() < 1)
      break; // Early termination
  }

  return result;
}

template <fixed_point_type FP> constexpr auto log(const FP &x) {
  if (x.raw_value() <= 0) {
    if constexpr (FP::overflow_policy == OverflowPolicy::Throw) {
      throw std::domain_error("log of non-positive number");
    }
    return FP::from_raw(std::numeric_limits<typename FP::storage_t>::min());
  }

  // Natural log using Newton's method
  // Find y such that e^y = x, so y = log(x)
  auto y = FP::from_float(1.0);

  for (int i = 0; i < 20; ++i) {
    auto ey = exp(y);
    auto delta = (ey - x) / ey;
    y -= delta;
    if (delta.abs().raw_value() < 1)
      break;
  }

  return y;
}

template <fixed_point_type FP1, fixed_point_type FP2>
constexpr auto pow(const FP1 &base, const FP2 &exponent) {
  // a^b = exp(b * log(a))
  if (base.raw_value() <= 0) {
    if constexpr (FP1::overflow_policy == OverflowPolicy::Throw) {
      throw std::domain_error("pow with non-positive base");
    }
    return FP1::from_raw(0);
  }
  return exp(exponent * log(base));
}

// Min/max functions
template <fixed_point_type FP1, fixed_point_type FP2>
constexpr auto min(const FP1 &a, const FP2 &b) {
  return a < b ? a : b;
}

template <fixed_point_type FP1, fixed_point_type FP2>
constexpr auto max(const FP1 &a, const FP2 &b) {
  return a > b ? a : b;
}

// User-defined literals
namespace literals {

constexpr Q8_8 operator""_q8(long double val) {
  return Q8_8(static_cast<double>(val));
}

constexpr Q16_16 operator""_q16(long double val) {
  return Q16_16(static_cast<double>(val));
}

constexpr Q24_8 operator""_q24(long double val) {
  return Q24_8(static_cast<double>(val));
}

// Parse fixed-point literal at compile time
template <char... Digits> constexpr Q16_16 operator""_fp() {
  constexpr std::array<char, sizeof...(Digits)> arr{Digits...};
  double val = 0.0;
  double factor = 1.0;
  bool decimal = false;
  bool negative = false;
  size_t i = 0;

  if (arr[0] == '-') {
    negative = true;
    i = 1;
  }

  for (; i < arr.size(); ++i) {
    if (arr[i] == '.') {
      decimal = true;
    } else {
      int digit = arr[i] - '0';
      if (decimal) {
        factor *= 0.1;
        val += digit * factor;
      } else {
        val = val * 10 + digit;
      }
    }
  }

  return Q16_16(negative ? -val : val);
}

} // namespace literals

} // namespace fp
