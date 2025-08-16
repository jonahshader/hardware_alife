#pragma once

#include <memory>

#include "fp/fp.h"
#include "snn.h"

namespace {
  
};

class Node {
public:
  virtual size_t inputs() = 0;
  virtual size_t outputs() = 0;
  virtual void update() = 0;
};

struct Limb {};

struct Creature {};
