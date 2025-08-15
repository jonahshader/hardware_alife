#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

struct Pixel {
  std::uint8_t a;
  std::uint8_t b;
  std::uint8_t g;
  std::uint8_t r;

  constexpr Pixel() : r(0), g(0), b(0), a(255) {} // default to opaque
  constexpr Pixel(std::uint8_t R, std::uint8_t G, std::uint8_t B, std::uint8_t A = 255)
      : r(R), g(G), b(B), a(A) {}
};

class Framebuffer {
public:
  Framebuffer(int width, int height) : m_width(width), m_height(height), fb(width * height) {}

  // non-const [] returns a Pixel reference
  Pixel &operator[](std::size_t index) {
    return fb.at(index); // bounds checking
  }

  // const [] for read-only access
  const Pixel &operator[](std::size_t index) const {
    return fb.at(index);
  }

  // 2D access
  Pixel &at(int x, int y) {
    return fb.at(y * m_width + x);
  }

  const Pixel &at(int x, int y) const {
    return fb.at(y * m_width + x);
  }

  Pixel *data() {
    return fb.data();
  }
  const Pixel *data() const {
    return fb.data();
  }

  int width() const {
    return m_width;
  }
  int height() const {
    return m_height;
  }

private:
  int m_width, m_height;
  std::vector<Pixel> fb;
};
