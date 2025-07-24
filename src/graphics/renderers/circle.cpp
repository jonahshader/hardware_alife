#include "circle.h"
#include <glad/glad.h>
#include <iostream>

CircleRenderer::CircleRenderer() : BaseRenderer("shaders/circle.vert", "shaders/circle.frag") {
  initialize();
}

void CircleRenderer::setup_vertex_attributes() {
  glBindBuffer(GL_ARRAY_BUFFER, vbo_data);
  
  // Each vertex has: position(2) + offset(2) + size(1) + color(4)
  const size_t stride = FLOATS_PER_VERTEX * sizeof(float);
  
  // Position (aPos) - normalized position within circle
  glVertexAttribPointer(0, 2, GL_FLOAT, false, stride, (void*)0);
  glEnableVertexAttribArray(0);
  
  // Offset (aOffset) - world position
  glVertexAttribPointer(1, 2, GL_FLOAT, false, stride, (void*)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);
  
  // Size (aSize) - circle diameter
  glVertexAttribPointer(2, 1, GL_FLOAT, false, stride, (void*)(4 * sizeof(float)));
  glEnableVertexAttribArray(2);
  
  // Color (aColor) - 4 individual float values
  glVertexAttribPointer(3, 4, GL_FLOAT, false, stride, (void*)(5 * sizeof(float)));
  glEnableVertexAttribArray(3);
}

void CircleRenderer::render_impl(size_t count) {
  glDrawArrays(GL_TRIANGLES, 0, count * VERTICES_PER_CIRCLE);
}

void CircleRenderer::add_circle(float x, float y, float radius, glm::vec4 color) {
  add_circle(x, y, radius, color.r * 255, color.g * 255, color.b * 255, color.a * 255);
}

void CircleRenderer::add_circle(float x, float y, float radius, unsigned char r, unsigned char g,
                                unsigned char b, unsigned char a) {
  radius *= 2; // CircleRenderer doubles radius internally
  
  // Convert color to float values
  float color_r = r / 255.0f;
  float color_g = g / 255.0f;
  float color_b = b / 255.0f;
  float color_a = a / 255.0f;
  
  // Generate vertices for 2 triangles (quad)
  // Format: aPos(2), aOffset(2), aSize(1), aColor(4)
  float vertices[VERTICES_PER_CIRCLE][FLOATS_PER_VERTEX] = {
    // Triangle 1
    {-0.5f, -0.5f, x, y, radius, color_r, color_g, color_b, color_a}, // bottom left
    { 0.5f, -0.5f, x, y, radius, color_r, color_g, color_b, color_a}, // bottom right
    { 0.5f,  0.5f, x, y, radius, color_r, color_g, color_b, color_a}, // top right
    // Triangle 2
    { 0.5f,  0.5f, x, y, radius, color_r, color_g, color_b, color_a}, // top right
    {-0.5f,  0.5f, x, y, radius, color_r, color_g, color_b, color_a}, // top left
    {-0.5f, -0.5f, x, y, radius, color_r, color_g, color_b, color_a}  // bottom left
  };
  
  // Add all vertices to data buffer
  for (int i = 0; i < VERTICES_PER_CIRCLE; i++) {
    for (int j = 0; j < FLOATS_PER_VERTEX; j++) {
      data.push_back(vertices[i][j]);
    }
  }
}