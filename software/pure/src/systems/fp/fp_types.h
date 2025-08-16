#pragma once

#include "fp_core.h"

namespace fp {

// Common type aliases
template <size_t IntBits, size_t FracBits>
using SFixed = FixedPoint<IntBits, FracBits, true>;

template <size_t IntBits, size_t FracBits>
using UFixed = FixedPoint<IntBits, FracBits, false>;

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

} // namespace fp