#pragma once

#include "fp_core.h"
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

namespace fp {

// String conversion functions (free functions since the methods are already declared in fp_core.h)
template <fixed_point_type FP>
std::string to_string(const FP &fp, int precision = -1) {
  std::ostringstream oss;
  if (precision >= 0) {
    oss.precision(precision);
    oss << std::fixed;
  }
  oss << fp.to_double();
  return oss.str();
}

template <typename FP>
FP from_string(const std::string &str) {
  std::istringstream iss(str);
  double val;
  if (!(iss >> val)) {
    throw std::invalid_argument("Invalid fixed-point string");
  }
  return FP(val);
}

// Stream operators
template <fixed_point_type FP>
std::ostream &operator<<(std::ostream &os, const FP &fp) {
  return os << fp.to_double();
}

template <fixed_point_type FP>
std::istream &operator>>(std::istream &is, FP &fp) {
  double val;
  if (is >> val) {
    fp = FP(val);
  }
  return is;
}

} // namespace fp