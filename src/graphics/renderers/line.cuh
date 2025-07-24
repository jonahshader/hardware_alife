#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "graphics/shader.h"
#include "base_renderer.h"

class LineRenderer : public BaseRenderer<unsigned int> {
public:
  LineRenderer();
  void add_line(float x1, float y1, float x2, float y2, float radius, const glm::vec4 &color);
  void add_line(float x1, float y1, float x2, float y2, float r1, float r2, const glm::vec4 &color1,
                const glm::vec4 &color2);
  void add_line(const glm::vec2 &v1, const glm::vec2 &v2, float r1, float r2,
                const glm::vec4 &color1, const glm::vec4 &color2);

  static const size_t VERTEX_ELEMS = 7;
  static const size_t VERTEX_BYTES = VERTEX_ELEMS * sizeof(float);
  static const size_t VERTICES_PER_LINE = 6;

protected:
  void setup_vertex_attributes() override;
  void render_impl(size_t count) override;
  size_t get_element_count() const override { return VERTEX_ELEMS; }
  size_t get_bytes_per_element() const override { return VERTEX_BYTES * VERTICES_PER_LINE; }
  bool uses_base_mesh() const override { return false; }
  const char* get_renderer_name() const override { return "LineRenderer"; }

private:
  void add_vertex(float x, float y, float tx, float ty, float length, float radius,
                  const glm::vec4 &color);
  void add_vertex(float x, float y, float tx, float ty, float length, float radius,
                  unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
};
