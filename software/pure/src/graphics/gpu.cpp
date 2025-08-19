#include "gpu.h"
#include <algorithm>
#include <limits>

namespace GPU {

void render_rect(const Instruction &instr, Framebuffer &fb);
void render_circle(const Instruction &instr, Framebuffer &fb);
void render_line(const Instruction &instr, Framebuffer &fb);
void render_sprite(const Instruction &instr, Framebuffer &fb);
void render_tile(const Instruction &instr, Framebuffer &fb);

void render(const Instruction &instr, Framebuffer &fb) {
  switch (instr.renderer) {
    case RECT:
      render_rect(instr, fb);
      break;
    case CIRCLE:
      render_circle(instr, fb);
      break;
    case LINE:
      render_line(instr, fb);
      break;
    case SPRITE:
      render_sprite(instr, fb);
      break;
    case TILE:
      render_tile(instr, fb);
      break;
    default:
      throw std::runtime_error("Unknown renderer");
  }
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

void render_rect(const Instruction &instr, Framebuffer &fb) {
  // optimization: if source is fully transparent, no need to render
  if (instr.color.a == 0)
    return;

  // clamp rectangle bounds to framebuffer
  auto x_start = std::max(instr.pos[0], static_cast<std::int16_t>(0));
  auto x_end = std::min(instr.pos[0] + static_cast<std::int16_t>(instr.size[0]),
                        static_cast<std::int16_t>(fb.width()));
  auto y_start = std::max(instr.pos[1], static_cast<std::int16_t>(0));
  auto y_end = std::min(instr.pos[1] + static_cast<std::int16_t>(instr.size[1]),
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
}

void render_circle(const Instruction &instr, Framebuffer &fb) {
  // TODO: see https://github.com/jonahshader/vampire_survivors_vhdl/blob/main/src_hdl/gpu.vhd
  // need to rewrite gpu to make it more generic. i.e., only common parts are renderer and pos.
  // size isn't used in every renderer so it shouldn't be included in the base instruction.
  // same thing here in C++.
  // optimization: if source is fully transparent, no need to render
  if (instr.color.a == 0)
    return;

  // clamp rectangle bounds to framebuffer
  auto x_start = std::max(instr.pos[0], static_cast<std::int16_t>(0));
  auto x_end = std::min(instr.pos[0] + static_cast<std::int16_t>(instr.size[0]),
                        static_cast<std::int16_t>(fb.width()));
  auto y_start = std::max(instr.pos[1], static_cast<std::int16_t>(0));
  auto y_end = std::min(instr.pos[1] + static_cast<std::int16_t>(instr.size[1]),
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
}

void render_line(const Instruction &instr, Framebuffer &fb) {}
void render_sprite(const Instruction &instr, Framebuffer &fb) {}
void render_tile(const Instruction &instr, Framebuffer &fb) {}

}; // namespace GPU
