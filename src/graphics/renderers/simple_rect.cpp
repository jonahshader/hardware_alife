#include "simple_rect.h"
#include <glad/glad.h>
#include <iostream>

SimpleRectRenderer::SimpleRectRenderer()
    : BaseRenderer("shaders/rect_simple.vert", "shaders/rect_simple.frag") {
  initialize();
}

void SimpleRectRenderer::setup_vertex_attributes() {
  glBindBuffer(GL_ARRAY_BUFFER, vbo_data);
  
  // Each vertex has: position(2) + offset(2) + size(2) + color(4)
  const size_t stride = 10 * sizeof(float);
  
  // Position (aPos)
  glVertexAttribPointer(0, 2, GL_FLOAT, false, stride, (void*)0);
  glEnableVertexAttribArray(0);
  
  // Offset (aOffset)
  glVertexAttribPointer(1, 2, GL_FLOAT, false, stride, (void*)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);
  
  // Size (aSize)
  glVertexAttribPointer(2, 2, GL_FLOAT, false, stride, (void*)(4 * sizeof(float)));
  glEnableVertexAttribArray(2);
  
  // Color (aColor)
  glVertexAttribPointer(3, 4, GL_FLOAT, false, stride, (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(3);
}

void SimpleRectRenderer::render_impl(size_t count) {
  glDrawArrays(GL_TRIANGLES, 0, count * 6); // 6 vertices per rectangle
}

void SimpleRectRenderer::add_rect(float x, float y, float width, float height, glm::vec4 color) {
  // Generate 6 vertices for 2 triangles (quad)
  float vertices[6][10] = {
    // Triangle 1
    {0.0f, 0.0f, x, y, width, height, color.r, color.g, color.b, color.a}, // bottom left
    {1.0f, 0.0f, x, y, width, height, color.r, color.g, color.b, color.a}, // bottom right
    {1.0f, 1.0f, x, y, width, height, color.r, color.g, color.b, color.a}, // top right
    // Triangle 2
    {1.0f, 1.0f, x, y, width, height, color.r, color.g, color.b, color.a}, // top right
    {0.0f, 1.0f, x, y, width, height, color.r, color.g, color.b, color.a}, // top left
    {0.0f, 0.0f, x, y, width, height, color.r, color.g, color.b, color.a}  // bottom left
  };
  
  // Add all vertices to data buffer
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 10; j++) {
      data.push_back(vertices[i][j]);
    }
  }
}

void SimpleRectRenderer::add_rect(glm::vec2 pos, glm::vec2 size, glm::vec4 color) {
  add_rect(pos.x, pos.y, size.x, size.y, color);
}