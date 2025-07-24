#include "rect.h"
#include <glad/glad.h>
#include <iostream>

RectRenderer::RectRenderer() : BaseRenderer("shaders/rect.vert", "shaders/rect.frag") {
  initialize();
}

void RectRenderer::setup_vertex_attributes() {
  glBindBuffer(GL_ARRAY_BUFFER, vbo_data);
  
  // Each vertex has: position(2) + offset(2) + size(2) + radius(1) + color(4)
  const size_t stride = 11 * sizeof(float);
  
  // Position (aPos)
  glVertexAttribPointer(0, 2, GL_FLOAT, false, stride, (void*)0);
  glEnableVertexAttribArray(0);
  
  // Offset (aOffset)
  glVertexAttribPointer(1, 2, GL_FLOAT, false, stride, (void*)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);
  
  // Size (aSize)
  glVertexAttribPointer(2, 2, GL_FLOAT, false, stride, (void*)(4 * sizeof(float)));
  glEnableVertexAttribArray(2);
  
  // Radius (aRadius)
  glVertexAttribPointer(3, 1, GL_FLOAT, false, stride, (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(3);
  
  // Color (aColor)
  glVertexAttribPointer(4, 4, GL_FLOAT, false, stride, (void*)(7 * sizeof(float)));
  glEnableVertexAttribArray(4);
}

void RectRenderer::render_impl(size_t count) {
  glDrawArrays(GL_TRIANGLES, 0, count * 6); // 6 vertices per rectangle
}

void RectRenderer::add_rect(float x, float y, float width, float height, float radius, glm::vec4 color) {
  // Generate 6 vertices for 2 triangles (quad)
  float vertices[6][11] = {
    // Triangle 1
    {-0.5f, -0.5f, x, y, width, height, radius, color.r, color.g, color.b, color.a}, // bottom left
    { 0.5f, -0.5f, x, y, width, height, radius, color.r, color.g, color.b, color.a}, // bottom right
    { 0.5f,  0.5f, x, y, width, height, radius, color.r, color.g, color.b, color.a}, // top right
    // Triangle 2
    { 0.5f,  0.5f, x, y, width, height, radius, color.r, color.g, color.b, color.a}, // top right
    {-0.5f,  0.5f, x, y, width, height, radius, color.r, color.g, color.b, color.a}, // top left
    {-0.5f, -0.5f, x, y, width, height, radius, color.r, color.g, color.b, color.a}  // bottom left
  };
  
  // Add all vertices to data buffer
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 11; j++) {
      data.push_back(vertices[i][j]);
    }
  }
}