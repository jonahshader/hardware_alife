#pragma once

#include "fp_core.h"
#include <limits>

namespace fp {

// Math functions
template <fixed_point_type FP>
constexpr auto sqrt(const FP &x) {
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
template <fixed_point_type FP>
constexpr auto sin(const FP &x) {
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

template <fixed_point_type FP>
constexpr auto cos(const FP &x) {
  constexpr auto pi_2 = FP::from_float(1.57079632679489661923);
  return sin(x + pi_2);
}

template <fixed_point_type FP>
constexpr auto tan(const FP &x) {
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
template <fixed_point_type FP>
constexpr auto exp(const FP &x) {
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

template <fixed_point_type FP>
constexpr auto log(const FP &x) {
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

} // namespace fp