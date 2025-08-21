#pragma once

#include <Eigen/Dense>
#include <cnl/all.h>
#include <cstdint>
#include <variant>

#include "framebuffer.h"

namespace {
using Eigen::Vector2;
using Eigen::Vector3;
using std::uint16_t;
}; // namespace

namespace gpu {

// enum Renderer { RECT, CIRCLE, LINE, SPRITE, TILE };

// struct Instruction {
//   Renderer renderer;
//   // all bits could be used if world is large and no occlusion culling is applied
//   Vector2<int16_t> pos;
//   Vector2<uint16_t> size;
//   Pixel color;
//   uint16_t enum_data; // this is interpreted differently depending on the renderer
// };

struct Rect {
  Vector2<int16_t> pos;
  Vector2<int16_t> size;
  Pixel color;
};

struct Circle {
  Vector2<int16_t> pos;
  int16_t radius;
  Pixel color;
};

struct Line {
  Vector2<int16_t> start;
  Vector2<int16_t> end;
  Pixel color;
};

using Instruction = std::variant<Rect, Circle, Line>;

int render(const Instruction &instr, Framebuffer &fb);

}; // namespace gpu
