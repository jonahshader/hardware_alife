#include "rect.cuh"
#include <glad/glad.h>
#include <iostream>
#include <cuda_gl_interop.h>

RectRenderer::RectRenderer() : BaseRenderer("shaders/rect.vert", "shaders/rect.frag") {
  initialize();
}

void RectRenderer::setup_base_mesh() {
  float base_mesh[] = {
      // t1
      -0.5f, -0.5f, // bottom left
      0.5f, -0.5f,  // bottom right
      0.5f, 0.5f,   // top right
      // t2
      0.5f, 0.5f,
      -0.5f, 0.5f,
      -0.5f, -0.5f,
  };

  glBindBuffer(GL_ARRAY_BUFFER, vbo_base_mesh);
  glBufferData(GL_ARRAY_BUFFER, sizeof(base_mesh), base_mesh, GL_STATIC_DRAW);
}

void RectRenderer::setup_vertex_attributes() {
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
  
  // radius
  glVertexAttribPointer(3, 1, GL_FLOAT, false, BYTES_PER_RECT, (void *)(4 * sizeof(float)));
  glVertexAttribDivisor(3, 1);
  glEnableVertexAttribArray(3);
  
  // color (r, g, b, a)
  glVertexAttribPointer(4, 4, GL_FLOAT, false, BYTES_PER_RECT, (void *)(5 * sizeof(float)));
  glVertexAttribDivisor(4, 1);
  glEnableVertexAttribArray(4);
}

void RectRenderer::render_impl(size_t count) {
  glDrawArraysInstanced(GL_TRIANGLES, 0, 6, count);
}


void RectRenderer::add_rect(float x, float y, float width, float height, float radius,
                            glm::vec4 color) {
  data.emplace_back(x);
  data.emplace_back(y);
  data.emplace_back(width);
  data.emplace_back(height);
  data.emplace_back(radius);
  data.emplace_back(color.r);
  data.emplace_back(color.g);
  data.emplace_back(color.b);
  data.emplace_back(color.a);
}


