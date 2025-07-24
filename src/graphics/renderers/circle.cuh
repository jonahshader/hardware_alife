#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "graphics/shader.h"
#include "base_renderer.h"

class CircleRenderer : public BaseRenderer<unsigned int> {
public:
  CircleRenderer();
  void add_circle(float x, float y, float radius, glm::vec4 color);
  void add_circle(float x, float y, float radius, unsigned char r, unsigned char g, unsigned char b,
                  unsigned char a);
  
  size_t get_circle_count() const {
    return buffer_size / CIRCLE_SIZE;
  }

  static const size_t ELEMS_PER_CIRCLE = 4;
  static const size_t CIRCLE_SIZE = ELEMS_PER_CIRCLE * sizeof(unsigned int);

protected:
  void setup_vertex_attributes() override;
  void setup_base_mesh() override;
  void render_impl(size_t count) override;
  size_t get_element_count() const override { return ELEMS_PER_CIRCLE; }
  size_t get_bytes_per_element() const override { return CIRCLE_SIZE; }
  const char* get_renderer_name() const override { return "CircleRenderer"; }

private:
  const unsigned int MIN_BUFFER_SIZE = 1024;


};
