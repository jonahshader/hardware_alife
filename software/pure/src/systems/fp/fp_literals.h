#pragma once

#include "fp_types.h"
#include <array>

namespace fp {

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
template <char... Digits>
constexpr Q16_16 operator""_fp() {
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