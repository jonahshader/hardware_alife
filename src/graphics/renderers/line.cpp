#include "line.h"
#include <glad/glad.h>
#include <iostream>

LineRenderer::LineRenderer() : BaseRenderer("shaders/line.vert", "shaders/line.frag") {
  initialize();
}

void LineRenderer::setup_vertex_attributes() {
  glBindBuffer(GL_ARRAY_BUFFER, vbo_data);

  // x y position
  glVertexAttribPointer(0, 2, GL_FLOAT, false, VERTEX_BYTES, (void *)0);
  glEnableVertexAttribArray(0);
  
  // tex coordinates (x, y)
  glVertexAttribPointer(1, 2, GL_FLOAT, false, VERTEX_BYTES, (void *)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);
  
  // line end
  glVertexAttribPointer(2, 1, GL_FLOAT, false, VERTEX_BYTES, (void *)(4 * sizeof(float)));
  glEnableVertexAttribArray(2);
  
  // radius
  glVertexAttribPointer(3, 1, GL_FLOAT, false, VERTEX_BYTES, (void *)(5 * sizeof(float)));
  glEnableVertexAttribArray(3);
  
  // color
  glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, true, VERTEX_BYTES, (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(4);
}

void LineRenderer::render_impl(size_t count) {
  glDrawArrays(GL_TRIANGLES, 0, count * VERTICES_PER_LINE);
}


void LineRenderer::add_line(float x1, float y1, float x2, float y2, float radius,
                            const glm::vec4 &color) {
  // need to calculate the perpendicular vector to the line_dir
  // then we can use that to calculate the four points of the rectangle

  glm::vec2 line = glm::vec2(x2 - x1, y2 - y1);
  float line_len = glm::length(line);
  glm::vec2 line_dir = glm::normalize(line) * radius;
  glm::vec2 perp_dir = glm::vec2(-line_dir.y, line_dir.x); // counter-clockwise

  // bottom left
  glm::vec2 bl = glm::vec2(x1, y1) + perp_dir - line_dir;
  // bottom right
  glm::vec2 br = glm::vec2(x1, y1) - perp_dir - line_dir;
  // top left
  glm::vec2 tl = glm::vec2(x2, y2) + perp_dir + line_dir;
  // top right
  glm::vec2 tr = glm::vec2(x2, y2) - perp_dir + line_dir;

  // tri 1
  add_vertex(bl.x, bl.y, -radius, -radius, line_len, radius, color);
  add_vertex(br.x, br.y, radius, -radius, line_len, radius, color);
  add_vertex(tr.x, tr.y, radius, radius + line_len, line_len, radius, color);
  // tri 2
  add_vertex(tr.x, tr.y, radius, radius + line_len, line_len, radius, color);
  add_vertex(tl.x, tl.y, -radius, radius + line_len, line_len, radius, color);
  add_vertex(bl.x, bl.y, -radius, -radius, line_len, radius, color);
}


void LineRenderer::add_line(float x1, float y1, float x2, float y2, float r1, float r2,
                            const glm::vec4 &color1, const glm::vec4 &color2) {
  // need to calculate the perpendicular vector to the line_dir
  // then we can use that to calculate the four points of the rectangle

  glm::vec2 line = glm::vec2(x2 - x1, y2 - y1);
  float line_len = glm::length(line);
  float radius = std::max(r1, r2);
  glm::vec2 line_dir = glm::normalize(line) * radius;
  glm::vec2 perp_dir = glm::vec2(-line_dir.y, line_dir.x); // counter-clockwise

  // bottom left
  glm::vec2 bl = glm::vec2(x1, y1) + perp_dir - line_dir;
  // bottom right
  glm::vec2 br = glm::vec2(x1, y1) - perp_dir - line_dir;
  // top left
  glm::vec2 tl = glm::vec2(x2, y2) + perp_dir + line_dir;
  // top right
  glm::vec2 tr = glm::vec2(x2, y2) - perp_dir + line_dir;

  // tri 1
  add_vertex(bl.x, bl.y, -radius, -radius, line_len, r1, color1);
  add_vertex(br.x, br.y, radius, -radius, line_len, r1, color1);
  add_vertex(tr.x, tr.y, radius, radius + line_len, line_len, r2, color2);
  // tri 2
  add_vertex(tr.x, tr.y, radius, radius + line_len, line_len, r2, color2);
  add_vertex(tl.x, tl.y, -radius, radius + line_len, line_len, r2, color2);
  add_vertex(bl.x, bl.y, -radius, -radius, line_len, r1, color1);
}

void LineRenderer::add_line(const glm::vec2 &v1, const glm::vec2 &v2, float r1, float r2,
                            const glm::vec4 &color1, const glm::vec4 &color2) {
  add_line(v1.x, v1.y, v2.x, v2.y, r1, r2, color1, color2);
}


void LineRenderer::add_vertex(float x, float y, float tx, float ty, float length, float radius,
                              const glm::vec4 &color) {
  add_vertex(x, y, tx, ty, length, radius, color.r * 255, color.g * 255, color.b * 255,
             color.a * 255);
}

void LineRenderer::add_vertex(float x, float y, float tx, float ty, float length, float radius,
                              unsigned char red, unsigned char green, unsigned char blue,
                              unsigned char alpha) {
  data.emplace_back(reinterpret_cast<unsigned int &>(x));
  data.emplace_back(reinterpret_cast<unsigned int &>(y));
  data.emplace_back(reinterpret_cast<unsigned int &>(tx));
  data.emplace_back(reinterpret_cast<unsigned int &>(ty));
  data.emplace_back(reinterpret_cast<unsigned int &>(length));
  data.emplace_back(reinterpret_cast<unsigned int &>(radius));
  unsigned int color = 0;
  color |= red;
  color |= green << 8;
  color |= blue << 16;
  color |= alpha << 24;
  data.emplace_back(color);
}

