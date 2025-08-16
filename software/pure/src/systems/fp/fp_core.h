#pragma once

#include <array>
#include <bit>
#include <cmath>
#include <compare>
#include <concepts>
#include <cstdint>
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
template <size_t Bits>
struct storage_type {
  using type = std::conditional_t<
      Bits <= 8, int8_t,
      std::conditional_t<
          Bits <= 16, int16_t,
          std::conditional_t<Bits <= 32, int32_t, std::conditional_t<Bits <= 64, int64_t, void>>>>;
};

template <size_t Bits>
struct unsigned_storage_type {
  using type = std::conditional_t<
      Bits <= 8, uint8_t,
      std::conditional_t<Bits <= 16, uint16_t,
                         std::conditional_t<Bits <= 32, uint32_t,
                                            std::conditional_t<Bits <= 64, uint64_t, void>>>>;
};

// Promotion type for multiplication
template <typename T1, typename T2>
using promote_t =
    std::conditional_t<sizeof(T1) + sizeof(T2) <= 4, int32_t,
                       std::conditional_t<sizeof(T1) + sizeof(T2) <= 8, int64_t, int64_t>>;

// Forward declaration
template <size_t UpperBits, size_t LowerBits, bool Signed = true,
          OverflowPolicy Policy = OverflowPolicy::Wrap>
class FixedPoint;

// Type traits for detecting fixed point types
template <typename T>
struct is_fixed_point : std::false_type {};

template <size_t U, size_t L, bool S, OverflowPolicy P>
struct is_fixed_point<FixedPoint<U, L, S, P>> : std::true_type {};

template <typename T>
inline constexpr bool is_fixed_point_v = is_fixed_point<T>::value;

template <typename T>
concept fixed_point_type = is_fixed_point_v<T>;

// Main FixedPoint class
template <size_t UpperBits, size_t LowerBits, bool Signed, OverflowPolicy Policy>
class FixedPoint {
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

public:
  // Overflow handling (public for template instantiation access)
  template <typename T>
  static constexpr storage_t handle_overflow(T val) {
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
  static constexpr FixedPoint from_raw(storage_t raw) {
    return FixedPoint(raw, raw_tag{});
  }

  // Constructor from integer types
  template <integral T>
  constexpr FixedPoint(T val) {
    using promoted_t = std::conditional_t<sizeof(T) < sizeof(storage_t),
                                          std::make_signed_t<int64_t>, std::make_signed_t<int64_t>>;
    promoted_t temp = static_cast<promoted_t>(val) << lower_bits;
    value = handle_overflow(temp);
  }

  // Constructor from floating point types
  template <floating_point T>
  constexpr FixedPoint(T val) {
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
  template <floating_point T>
  explicit constexpr operator T() const {
    return static_cast<T>(value) / scale_factor;
  }

  template <integral T>
  explicit constexpr operator T() const {
    return static_cast<T>(value >> lower_bits);
  }

  // Three-way comparison
  template <fixed_point_type FP>
  constexpr auto operator<=>(const FP &rhs) const {
    constexpr size_t max_lower = std::max(lower_bits, FP::lower_bits);
    using result_t = FixedPoint<std::max(upper_bits, FP::upper_bits), max_lower, true, Policy>;
    result_t lhs_conv(*this);
    result_t rhs_conv(rhs);
    return lhs_conv.raw_value() <=> rhs_conv.raw_value();
  }

  template <fixed_point_type FP>
  constexpr bool operator==(const FP &rhs) const {
    return (*this <=> rhs) == 0;
  }

  // Comparison with integers
  template <integral T>
  constexpr auto operator<=>(T rhs) const {
    return *this <=> FixedPoint<sizeof(T) * 8, 0, std::is_signed_v<T>, Policy>(rhs);
  }

  template <integral T>
  constexpr bool operator==(T rhs) const {
    return (*this <=> rhs) == 0;
  }

  // Arithmetic operators with optimized promotion
  template <fixed_point_type FP>
  constexpr auto operator+(const FP &rhs) const {
    constexpr size_t result_upper = std::min(std::max(upper_bits, FP::upper_bits) + 1, size_t(32));
    constexpr size_t result_lower = std::min(std::max(lower_bits, FP::lower_bits), 64 - result_upper);
    constexpr bool result_signed = is_signed || FP::is_signed;
    using result_t = FixedPoint<result_upper, result_lower, result_signed, Policy>;

    result_t lhs_conv(*this);
    result_t rhs_conv(rhs);

    using op_t = promote_t<storage_t, typename FP::storage_t>;
    auto result_val =
        static_cast<op_t>(lhs_conv.raw_value()) + static_cast<op_t>(rhs_conv.raw_value());
    return result_t::from_raw(static_cast<typename result_t::storage_t>(result_val));
  }

  template <fixed_point_type FP>
  constexpr auto operator-(const FP &rhs) const {
    constexpr size_t result_upper = std::min(std::max(upper_bits, FP::upper_bits) + 1, size_t(32));
    constexpr size_t result_lower = std::min(std::max(lower_bits, FP::lower_bits), 64 - result_upper);
    constexpr bool result_signed = true;
    using result_t = FixedPoint<result_upper, result_lower, result_signed, Policy>;

    result_t lhs_conv(*this);
    result_t rhs_conv(rhs);

    using op_t = promote_t<storage_t, typename FP::storage_t>;
    auto result_val =
        static_cast<op_t>(lhs_conv.raw_value()) - static_cast<op_t>(rhs_conv.raw_value());
    return result_t::from_raw(static_cast<typename result_t::storage_t>(result_val));
  }

  template <fixed_point_type FP>
  constexpr auto operator*(const FP &rhs) const {
    constexpr size_t temp_upper = upper_bits + FP::upper_bits;
    constexpr size_t temp_lower = lower_bits + FP::lower_bits;
    constexpr size_t result_upper = std::min(temp_upper, size_t(32));
    constexpr size_t result_lower = std::min(temp_lower, 64 - result_upper);
    constexpr bool result_signed = is_signed || FP::is_signed;

    using op_t = promote_t<storage_t, typename FP::storage_t>;
    auto result_val = static_cast<op_t>(value) * static_cast<op_t>(rhs.raw_value());

    using result_t = FixedPoint<result_upper, result_lower, result_signed, Policy>;
    return result_t::from_raw(static_cast<typename result_t::storage_t>(result_val));
  }

  template <fixed_point_type FP>
  constexpr auto operator/(const FP &rhs) const {
    constexpr size_t temp_upper = upper_bits + FP::lower_bits;
    constexpr size_t temp_lower = lower_bits + FP::upper_bits;
    constexpr size_t result_upper = std::min(temp_upper, size_t(32));
    constexpr size_t result_lower = std::min(temp_lower, 64 - result_upper);
    constexpr bool result_signed = is_signed || FP::is_signed;
    using result_t = FixedPoint<result_upper, result_lower, result_signed, Policy>;

    if (rhs.raw_value() == 0) {
      if constexpr (Policy == OverflowPolicy::Throw) {
        throw std::domain_error("Division by zero");
      }
      return result_t(*this);  // Convert to result type
    }

    using op_t = promote_t<storage_t, typename FP::storage_t>;
    auto scaled_num = static_cast<op_t>(value) << (FP::lower_bits + FP::upper_bits);
    auto result_val = scaled_num / rhs.raw_value();

    return result_t::from_raw(static_cast<typename result_t::storage_t>(result_val));
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
    constexpr size_t result_upper = std::min(std::max(temp_upper, FP2::upper_bits) + 1, size_t(32));
    constexpr size_t result_lower = std::min(std::max(temp_lower, FP2::lower_bits), 64 - result_upper);
    constexpr bool result_signed = is_signed || FP1::is_signed || FP2::is_signed;
    using result_t = FixedPoint<result_upper, result_lower, result_signed, Policy>;

    // Convert add operand and perform addition
    if constexpr (temp_lower > FP2::lower_bits) {
      auto shifted_add = static_cast<op_t>(add.raw_value()) << (temp_lower - FP2::lower_bits);
      return result_t::from_raw(static_cast<typename result_t::storage_t>(mul_result + shifted_add));
    } else if constexpr (temp_lower < FP2::lower_bits) {
      mul_result <<= (FP2::lower_bits - temp_lower);
      return result_t::from_raw(static_cast<typename result_t::storage_t>(mul_result + add.raw_value()));
    } else {
      return result_t::from_raw(static_cast<typename result_t::storage_t>(mul_result + add.raw_value()));
    }
  }

  // Integer arithmetic
  template <integral T>
  constexpr auto operator+(T rhs) const {
    return *this + FixedPoint<sizeof(T) * 8, 0, std::is_signed_v<T>, Policy>(rhs);
  }

  template <integral T>
  constexpr auto operator-(T rhs) const {
    return *this - FixedPoint<sizeof(T) * 8, 0, std::is_signed_v<T>, Policy>(rhs);
  }

  template <integral T>
  constexpr auto operator*(T rhs) const {
    return *this * FixedPoint<sizeof(T) * 8, 0, std::is_signed_v<T>, Policy>(rhs);
  }

  template <integral T>
  constexpr auto operator/(T rhs) const {
    return *this / FixedPoint<sizeof(T) * 8, 0, std::is_signed_v<T>, Policy>(rhs);
  }

  // Floating point arithmetic
  template <floating_point T>
  constexpr auto operator+(T rhs) const {
    return *this + FixedPoint(rhs);
  }

  template <floating_point T>
  constexpr auto operator-(T rhs) const {
    return *this - FixedPoint(rhs);
  }

  template <floating_point T>
  constexpr auto operator*(T rhs) const {
    return *this * FixedPoint(rhs);
  }

  template <floating_point T>
  constexpr auto operator/(T rhs) const {
    return *this / FixedPoint(rhs);
  }

  // Compound assignment operators
  template <typename T>
  constexpr FixedPoint &operator+=(const T &rhs) {
    *this = *this + rhs;
    return *this;
  }

  template <typename T>
  constexpr FixedPoint &operator-=(const T &rhs) {
    *this = *this - rhs;
    return *this;
  }

  template <typename T>
  constexpr FixedPoint &operator*=(const T &rhs) {
    *this = *this * rhs;
    return *this;
  }

  template <typename T>
  constexpr FixedPoint &operator/=(const T &rhs) {
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
    return from_raw(static_cast<storage_t>(-static_cast<int64_t>(value)));
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
      return from_raw(value >= 0 ? value : static_cast<storage_t>(-static_cast<int64_t>(value)));
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
  template <floating_point T>
  static constexpr FixedPoint from_float(T val) {
    return FixedPoint(val);
  }

  template <floating_point T>
  constexpr T to_float() const {
    return static_cast<T>(*this);
  }

  constexpr float to_float() const {
    return static_cast<float>(value) / scale_factor;
  }

  constexpr double to_double() const {
    return static_cast<double>(value) / scale_factor;
  }

  template <integral T>
  constexpr T to_int() const {
    return static_cast<T>(*this);
  }

  // String conversion (inline definitions)
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
template <integral T, fixed_point_type FP>
constexpr auto operator+(T lhs, const FP &rhs) {
  return FixedPoint<sizeof(T) * 8, 0, std::is_signed_v<T>, FP::overflow_policy>(lhs) + rhs;
}

template <integral T, fixed_point_type FP>
constexpr auto operator-(T lhs, const FP &rhs) {
  return FixedPoint<sizeof(T) * 8, 0, std::is_signed_v<T>, FP::overflow_policy>(lhs) - rhs;
}

template <integral T, fixed_point_type FP>
constexpr auto operator*(T lhs, const FP &rhs) {
  return FixedPoint<sizeof(T) * 8, 0, std::is_signed_v<T>, FP::overflow_policy>(lhs) * rhs;
}

template <integral T, fixed_point_type FP>
constexpr auto operator/(T lhs, const FP &rhs) {
  return FixedPoint<sizeof(T) * 8, 0, std::is_signed_v<T>, FP::overflow_policy>(lhs) / rhs;
}

// Comparison operators with integer on left
template <integral T, fixed_point_type FP>
constexpr auto operator<=>(T lhs, const FP &rhs) {
  return FixedPoint<sizeof(T) * 8, 0, std::is_signed_v<T>, FP::overflow_policy>(lhs) <=> rhs;
}

template <integral T, fixed_point_type FP>
constexpr bool operator==(T lhs, const FP &rhs) {
  return (lhs <=> rhs) == 0;
}

// Non-member operators for floating point on left side
template <floating_point T, fixed_point_type FP>
constexpr auto operator+(T lhs, const FP &rhs) {
  return FP(lhs) + rhs;
}

template <floating_point T, fixed_point_type FP>
constexpr auto operator-(T lhs, const FP &rhs) {
  return FP(lhs) - rhs;
}

template <floating_point T, fixed_point_type FP>
constexpr auto operator*(T lhs, const FP &rhs) {
  return FP(lhs) * rhs;
}

template <floating_point T, fixed_point_type FP>
constexpr auto operator/(T lhs, const FP &rhs) {
  return FP(lhs) / rhs;
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

} // namespace fp