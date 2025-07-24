#include "simple_rect.cuh"
#include <glad/glad.h>
#include <iostream>
#include <cuda_gl_interop.h>

SimpleRectRenderer::SimpleRectRenderer()
    : BaseRenderer("shaders/rect_simple.vert", "shaders/rect_simple.frag") {
  initialize();
}

void SimpleRectRenderer::setup_base_mesh() {
  float base_mesh[] = {
      // t1
      0.0f, 0.0f, // bottom left
      1.0f, 0.0f, // bottom right
      1.0f, 1.0f, // top right
      // t2
      1.0f, 1.0f,
      0.0f, 1.0f,
      0.0f, 0.0f,
  };

  glBindBuffer(GL_ARRAY_BUFFER, vbo_base_mesh);
  glBufferData(GL_ARRAY_BUFFER, sizeof(base_mesh), base_mesh, GL_STATIC_DRAW);
}

void SimpleRectRenderer::setup_vertex_attributes() {
  // Base mesh vertices
  glBindBuffer(GL_ARRAY_BUFFER, vbo_base_mesh);

  // x y position
  glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // Instance data
  glBindBuffer(GL_ARRAY_BUFFER, vbo_data);
  
  // offset (x, y)
  glVertexAttribPointer(1, 2, GL_FLOAT, false, BYTES_PER_RECT, (void *)0);
  glVertexAttribDivisor(1, 1);
  glEnableVertexAttribArray(1);
  
  // size (width, height)
  glVertexAttribPointer(2, 2, GL_FLOAT, false, BYTES_PER_RECT, (void *)(2 * sizeof(float)));
  glVertexAttribDivisor(2, 1);
  glEnableVertexAttribArray(2);
  
  // color (r, g, b, a)
  glVertexAttribPointer(3, 4, GL_FLOAT, false, BYTES_PER_RECT, (void *)(4 * sizeof(float)));
  glVertexAttribDivisor(3, 1);
  glEnableVertexAttribArray(3);
}

void SimpleRectRenderer::render_impl(size_t count) {
  glDrawArraysInstanced(GL_TRIANGLES, 0, 6, count);
}


void SimpleRectRenderer::add_rect(float x, float y, float width, float height, glm::vec4 color) {
  data.emplace_back(x);
  data.emplace_back(y);
  data.emplace_back(width);
  data.emplace_back(height);
  data.emplace_back(color.r);
  data.emplace_back(color.g);
  data.emplace_back(color.b);
  data.emplace_back(color.a);
}

void SimpleRectRenderer::add_rect(float2 pos, float2 size, glm::vec4 color) {
  add_rect(pos.x, pos.y, size.x, size.y, color);
}


