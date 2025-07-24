#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "graphics/shader.h"
#include "base_renderer.h"

class SimpleRectRenderer : public BaseRenderer<float> {
public:
  SimpleRectRenderer();
  void add_rect(float x, float y, float width, float height, glm::vec4 color);
  void add_rect(float2 pos, float2 size, glm::vec4 color);

  static constexpr auto FLOATS_PER_RECT = 8; // xoffset yoffset width height r g b a
  static constexpr auto BYTES_PER_RECT = FLOATS_PER_RECT * sizeof(float);

protected:
  void setup_vertex_attributes() override;
  void setup_base_mesh() override;
  void render_impl(size_t count) override;
  size_t get_element_count() const override { return FLOATS_PER_RECT; }
  size_t get_bytes_per_element() const override { return BYTES_PER_RECT; }
  const char* get_renderer_name() const override { return "SimpleRectRenderer"; }
};

// Include CUDA vector type for add_rect overload
#include <vector_types.h>
