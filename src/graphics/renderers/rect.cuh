#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "graphics/shader.h"
#include "base_renderer.h"

class RectRenderer : public BaseRenderer<float> {
public:
  RectRenderer();
  void add_rect(float x, float y, float width, float height, float radius, glm::vec4 color);

  static constexpr auto FLOATS_PER_RECT = 9;
  static constexpr auto BYTES_PER_RECT = FLOATS_PER_RECT * sizeof(float);

protected:
  void setup_vertex_attributes() override;
  void setup_base_mesh() override;
  void render_impl(size_t count) override;
  size_t get_element_count() const override { return FLOATS_PER_RECT; }
  size_t get_bytes_per_element() const override { return BYTES_PER_RECT; }
  const char* get_renderer_name() const override { return "RectRenderer"; }
};
