#include "gpu_test.h"

#include "graphics/gpu.h"
#include <Eigen/Dense>

namespace {
using Eigen::Vector2;
using std::int16_t;
using std::uint16_t;
}; // namespace

GPUTestScreen::GPUTestScreen(ScreenContext &ctx) : Screen(ctx) {}

void GPUTestScreen::render(Framebuffer &fb) {
  // grab mouse state
  const auto &mouse = get_mouse_state();

  // draw circle on the mouse
  gpu::Instruction instr = gpu::Circle(Vector2<int16_t>(64, 64), 8, Pixel(0, 255, 0, 255));
  gpu::render(instr, fb);
}
