#include "circle.cuh"
#include <glad/glad.h>
#include <cuda_gl_interop.h>
#include <iostream>

CircleRenderer::CircleRenderer() : BaseRenderer("shaders/circle.vert", "shaders/circle.frag") {
  // Allocate initial buffer with MIN_BUFFER_SIZE
  buffer_size = MIN_BUFFER_SIZE;
  glBindBuffer(GL_ARRAY_BUFFER, vbo_data);
  glBufferData(GL_ARRAY_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  initialize();
}

void CircleRenderer::setup_base_mesh() {
  float baseMesh[] = {
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
  glBufferData(GL_ARRAY_BUFFER, sizeof(baseMesh), baseMesh, GL_STATIC_DRAW);
}

void CircleRenderer::setup_vertex_attributes() {
  // Base mesh vertices
  glBindBuffer(GL_ARRAY_BUFFER, vbo_base_mesh);

  // x y position
  glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // Instance data
  glBindBuffer(GL_ARRAY_BUFFER, vbo_data);
  GLsizei s = 4 * sizeof(float);
  
  // offset (x, y)
  glVertexAttribPointer(1, 2, GL_FLOAT, false, s, (void *)0);
  glVertexAttribDivisor(1, 1);
  glEnableVertexAttribArray(1);
  
  // size
  glVertexAttribPointer(2, 1, GL_FLOAT, false, s, (void *)(2 * sizeof(float)));
  glVertexAttribDivisor(2, 1);
  glEnableVertexAttribArray(2);
  
  // color (rgba)
  glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, true, s, (void *)(3 * sizeof(float)));
  glVertexAttribDivisor(3, 1);
  glEnableVertexAttribArray(3);
}

void CircleRenderer::render_impl(size_t count) {
  glDrawArraysInstanced(GL_TRIANGLES, 0, 6, count);
}


void CircleRenderer::add_circle(float x, float y, float radius, glm::vec4 color) {
  add_circle(x, y, radius, color.r * 255, color.g * 255, color.b * 255, color.a * 255);
}

void CircleRenderer::add_circle(float x, float y, float radius, unsigned char r, unsigned char g,
                                unsigned char b, unsigned char a) {
  radius *= 2;
  data.emplace_back(reinterpret_cast<unsigned int &>(x));
  data.emplace_back(reinterpret_cast<unsigned int &>(y));
  data.emplace_back(reinterpret_cast<unsigned int &>(radius));
  // pack color into a single unsigned int
  unsigned int color = 0;
  color |= r;
  color |= g << 8;
  color |= b << 16;
  color |= a << 24;

  data.emplace_back(color);
}
