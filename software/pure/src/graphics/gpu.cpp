#include "gpu.h"
#include <algorithm>
#include <limits>

constexpr int FRAME_READ_DELAY = 3; // used for alpha blending

namespace gpu {

int render(const Rect &instr, Framebuffer &fb);
int render(const Circle &instr, Framebuffer &fb);
int render(const Line &instr, Framebuffer &fb);

int render(const Instruction &instr, Framebuffer &fb) {
  return std::visit([&fb](const auto& instruction) -> int {
    return render(instruction, fb);
  }, instr);
}

// OpenGL-style alpha blending: GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
// formula: final = source.rgb * source.a + dest.rgb * (1 - source.a)
//          final.a = source.a + dest.a * (1 - source.a)
Pixel alpha_blend(const Pixel &source, const Pixel &dest) {
  constexpr auto max_alpha = std::numeric_limits<std::uint8_t>::max();
  const auto src_alpha = source.a;
  const auto inv_src_alpha = max_alpha - src_alpha;

  return Pixel(
      // red component
      (source.r * src_alpha + dest.r * inv_src_alpha) / max_alpha,
      // green component
      (source.g * src_alpha + dest.g * inv_src_alpha) / max_alpha,
      // blue component
      (source.b * src_alpha + dest.b * inv_src_alpha) / max_alpha,
      // alpha component (standard alpha compositing)
      src_alpha + (dest.a * inv_src_alpha) / max_alpha);
}

int render(const Rect &instr, Framebuffer &fb) {
  // optimization: if source is fully transparent, no need to render
  if (instr.color.a == 0)
    return 0;

  // clamp rectangle bounds to framebuffer
  auto x_start = std::max(instr.pos[0], std::int16_t{0});
  auto x_end = std::min(static_cast<std::int16_t>(instr.pos[0] + instr.size[0]),
                        static_cast<std::int16_t>(fb.width()));
  auto y_start = std::max(instr.pos[1], std::int16_t{0});
  auto y_end = std::min(static_cast<std::int16_t>(instr.pos[1] + instr.size[1]),
                        static_cast<std::int16_t>(fb.height()));

  // optimization: if source is fully opaque, we can skip blending
  if (instr.color.a == std::numeric_limits<std::uint8_t>::max()) {
    for (auto y = y_start; y < y_end; ++y) {
      for (auto x = x_start; x < x_end; ++x) {
        fb.at(x, y) = instr.color;
      }
    }
  }
  // standard alpha blending
  else {
    for (auto y = y_start; y < y_end; ++y) {
      for (auto x = x_start; x < x_end; ++x) {
        const Pixel existing = fb.at(x, y);
        fb.at(x, y) = alpha_blend(instr.color, existing);
      }
    }
  }

  return 0;
}

int render(const Circle &instr, Framebuffer &fb) {
  // TODO: see https://github.com/jonahshader/vampire_survivors_vhdl/blob/main/src_hdl/gpu.vhd
  // need to rewrite gpu to make it more generic. i.e., only common parts are renderer and pos.
  // size isn't used in every renderer so it shouldn't be included in the base instruction.
  // same thing here in C++.
  // optimization: if source is fully transparent, no need to render
  if (instr.color.a == 0)
    return 0; // TODO

  // clamp rectangle bounds to framebuffer
  auto x_start = std::max(static_cast<std::int16_t>(instr.pos[0] - instr.radius), std::int16_t{0});
  auto x_end = std::min(static_cast<std::int16_t>(instr.pos[0] + instr.radius + 1), static_cast<std::int16_t>(fb.width()));
  auto y_start = std::max(static_cast<std::int16_t>(instr.pos[1] - instr.radius), std::int16_t{0});
  auto y_end = std::min(static_cast<std::int16_t>(instr.pos[1] + instr.radius + 1), static_cast<std::int16_t>(fb.height()));
  std::uint32_t r2 = instr.radius * instr.radius;

  // optimization: if source is fully opaque, we can skip blending
  if (instr.color.a == std::numeric_limits<std::uint8_t>::max()) {
    for (auto y = y_start; y < y_end; ++y) {
      for (auto x = x_start; x < x_end; ++x) {
        auto dx = x - instr.pos[0];
        auto dy = y - instr.pos[1];
        if (dx * dx + dy * dy <= r2) {
          fb.at(x, y) = instr.color;
        }
      }
    }
  }
  // standard alpha blending
  else {
    for (auto y = y_start; y < y_end; ++y) {
      for (auto x = x_start; x < x_end; ++x) {
        auto dx = x - instr.pos[0];
        auto dy = y - instr.pos[1];
        if (dx * dx + dy * dy <= r2) {
          const Pixel existing = fb.at(x, y);
          fb.at(x, y) = alpha_blend(instr.color, existing);
        }
      }
    }
  }

  return 0; // TODO
}

int render(const Line &instr, Framebuffer &fb) {
  // optimization: if source is fully transparent, no need to render
  if (instr.color.a == 0)
    return 0;

  // extract coordinates
  int x0 = instr.start[0];
  int y0 = instr.start[1];
  int x1 = instr.end[0];
  int y1 = instr.end[1];

  // calculate deltas and determine if line is steep
  int dx = std::abs(x1 - x0);
  int dy = std::abs(y1 - y0);
  bool steep = dy > dx;

  // if steep, swap x and y coordinates
  if (steep) {
    std::swap(x0, y0);
    std::swap(x1, y1);
    std::swap(dx, dy);
  }

  // ensure we're drawing left to right
  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  // recalculate deltas after potential swaps
  dx = x1 - x0;
  dy = std::abs(y1 - y0);

  // determine y step direction
  int ystep = (y0 < y1) ? 1 : -1;

  // bresenham algorithm variables
  int error = 0;
  int y = y0;

  // optimization: if source is fully opaque, we can skip blending
  bool use_blending = (instr.color.a != std::numeric_limits<std::uint8_t>::max());

  // main line drawing loop
  for (int x = x0; x <= x1; ++x) {
    // determine actual pixel coordinates (handle steep case)
    int pixel_x = steep ? y : x;
    int pixel_y = steep ? x : y;

    // bounds checking and pixel setting
    if (pixel_x >= 0 && pixel_x < fb.width() && pixel_y >= 0 && pixel_y < fb.height()) {

      if (use_blending) {
        const Pixel existing = fb.at(pixel_x, pixel_y);
        fb.at(pixel_x, pixel_y) = alpha_blend(instr.color, existing);
      } else {
        fb.at(pixel_x, pixel_y) = instr.color;
      }
    }

    // update error and y coordinate
    error += dy;
    if ((error << 1) >= dx) { // equivalent to (error * 2) >= dx
      y += ystep;
      error -= dx;
    }
  }

  return 0; // success
}

}; // namespace gpu
