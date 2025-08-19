#pragma once

#include <memory>

#include "snn.h"

namespace {};

class Creature;

class Node {
public:
  virtual size_t inputs() = 0;
  virtual size_t outputs() = 0;
  virtual void update(Creature &creature, float x, float y, float cos_ang, float sin_ang) = 0;
};



class Creature {

};
