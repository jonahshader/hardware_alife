#pragma once

#include <Eigen/Dense>
#include <cnl/all.h>
#include <cstdint>

#include "framebuffer.h"

namespace {
using Eigen::Vector2;
using Eigen::Vector3;
std::uint16_t;
}; // namespace

namespace GPU {

enum Renderer { RECT, CIRCLE, LINE, SPRITE, TILE };

struct Instruction {
  Renderer renderer;
  // all bits could be used if world is large and no occlusion culling is applied
  Vector2<int16_t> pos;
  Vector2<uint16_t> size;
  Pixel color;
  uint16_t enum_data; // this is interpreted differently depending on the renderer
};

void render(const Instruction &instr, Framebuffer &fb);

}; // namespace GPU
