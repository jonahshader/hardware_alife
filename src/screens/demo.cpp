#include "demo.h"
#include <cmath>
#include <chrono>

DemoScreen::DemoScreen(Game &game) : DefaultScreen(game) {}

void DemoScreen::render() {
  render_start();

  auto &res = game.get_resources();
  
  // CPU Circle Animation Demo
  if (show_cpu_demo) {
    const int grid_size = 8;
    const int total_circles = grid_size * grid_size;
    
    // Generate animated circles on CPU
    for (int idx = 0; idx < total_circles; idx++) {
      int row = idx / grid_size;
      int col = idx % grid_size;
      
      // Calculate position
      float spacing = 80.0f;
      float x = 200.0f + col * spacing;  // base_x
      float y = -200.0f + row * spacing; // base_y
      
      // Animate with sine waves
      float wave_x = sinf(animation_time * 2.0f + col * 0.5f) * 15.0f;
      float wave_y = cosf(animation_time * 1.5f + row * 0.3f) * 10.0f;
      x += wave_x;
      y += wave_y;
      
      // Animate radius with sine wave
      float base_radius = 15.0f;
      float radius_mod = sinf(animation_time * 3.0f + (row + col) * 0.4f) * 8.0f;
      float radius = base_radius + radius_mod;
      
      // Animate color based on position and time
      float hue = fmodf(animation_time * 0.5f + (row * 0.1f + col * 0.15f), 1.0f);
      
      // Convert HSV to RGB (simplified)
      float r, g, b;
      if (hue < 0.33f) {
        r = 1.0f - hue * 3.0f;
        g = hue * 3.0f;
        b = 0.0f;
      } else if (hue < 0.66f) {
        r = 0.0f;
        g = 1.0f - (hue - 0.33f) * 3.0f;
        b = (hue - 0.33f) * 3.0f;
      } else {
        r = (hue - 0.66f) * 3.0f;
        g = 0.0f;
        b = 1.0f - (hue - 0.66f) * 3.0f;
      }
      
      // Add circle to renderer
      res.circle.add_circle(x, y, radius, glm::vec4(r, g, b, 1.0f));
    }
  }
  
  // Test CircleRenderer (manual circles)
  if (show_circles) {
    res.circle.add_circle(-200.0f, 200.0f, 50.0f, glm::vec4(1.0f, 0.2f, 0.2f, 1.0f)); // Red circle
    res.circle.add_circle(-100.0f, 200.0f, 30.0f, glm::vec4(0.2f, 1.0f, 0.2f, 1.0f)); // Green circle
    res.circle.add_circle(0.0f, 200.0f, 40.0f, glm::vec4(0.2f, 0.2f, 1.0f, 0.8f));    // Blue circle (semi-transparent)
  }
  
  // Test RectRenderer (rounded rectangles)
  if (show_rects) {
    res.rect.add_rect(-200.0f, 50.0f, 80.0f, 60.0f, 10.0f, glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)); // Orange rounded rect
    res.rect.add_rect(-100.0f, 50.0f, 60.0f, 40.0f, 5.0f, glm::vec4(0.5f, 0.0f, 1.0f, 1.0f));  // Purple rounded rect
    res.rect.add_rect(0.0f, 50.0f, 90.0f, 50.0f, 15.0f, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));    // Cyan rounded rect
  }
  
  // Test SimpleRectRenderer (basic rectangles)
  if (show_simple_rects) {
    res.simple_rect.add_rect(-200.0f, -100.0f, 70.0f, 50.0f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow rect
    res.simple_rect.add_rect(-100.0f, -100.0f, 50.0f, 70.0f, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)); // Magenta rect
    res.simple_rect.add_rect(0.0f, -100.0f, 80.0f, 40.0f, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));    // Gray rect
  }
  
  // Test LineRenderer
  if (show_lines) {
    res.line.add_line(-200.0f, -250.0f, -100.0f, -200.0f, 8.0f, glm::vec4(1.0f, 0.3f, 0.3f, 1.0f)); // Red line
    res.line.add_line(-100.0f, -250.0f, 0.0f, -200.0f, 5.0f, glm::vec4(0.3f, 1.0f, 0.3f, 1.0f));    // Green line
    res.line.add_line(0.0f, -250.0f, 100.0f, -200.0f, 12.0f, glm::vec4(0.3f, 0.3f, 1.0f, 1.0f));    // Blue line
    
    // Add gradient line (different colors at endpoints)
    res.line.add_line(150.0f, -250.0f, 250.0f, -150.0f, 6.0f, 10.0f, 
                      glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)); // Red to blue gradient
  }
  
  // Test FontRenderer with labels (render in world space so they pan with camera)
  if (show_labels) {
    res.main_font_world.add_text(-200.0f, 280.0f, 30, "Circles", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    res.main_font_world.add_text(-200.0f, 130.0f, 30, "Rounded Rects", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    res.main_font_world.add_text(-200.0f, -20.0f, 30, "Simple Rects", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    res.main_font_world.add_text(-200.0f, -170.0f, 30, "Lines", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
  }
  
  // Add control instructions in HUD
  auto left = hud_vp.get_left();
  auto top = hud_vp.get_top();
  res.main_font.add_text(left + 20.0f, top - 30.0f, 20, "Demo Controls:", glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
  res.main_font.add_text(left + 20.0f, top - 55.0f, 16, "1: Toggle Circles (" + std::string(show_circles ? "ON" : "OFF") + ")", 
                         glm::vec4(show_circles ? 0.5f : 0.3f, show_circles ? 1.0f : 0.3f, show_circles ? 0.5f : 0.3f, 1.0f));
  res.main_font.add_text(left + 20.0f, top - 75.0f, 16, "2: Toggle Rounded Rects (" + std::string(show_rects ? "ON" : "OFF") + ")", 
                         glm::vec4(show_rects ? 1.0f : 0.3f, show_rects ? 0.7f : 0.3f, show_rects ? 0.3f : 0.3f, 1.0f));
  res.main_font.add_text(left + 20.0f, top - 95.0f, 16, "3: Toggle Simple Rects (" + std::string(show_simple_rects ? "ON" : "OFF") + ")", 
                         glm::vec4(show_simple_rects ? 1.0f : 0.3f, show_simple_rects ? 1.0f : 0.3f, show_simple_rects ? 0.3f : 0.3f, 1.0f));
  res.main_font.add_text(left + 20.0f, top - 115.0f, 16, "4: Toggle Lines (" + std::string(show_lines ? "ON" : "OFF") + ")", 
                         glm::vec4(show_lines ? 0.3f : 0.3f, show_lines ? 0.5f : 0.3f, show_lines ? 1.0f : 0.3f, 1.0f));
  res.main_font.add_text(left + 20.0f, top - 135.0f, 16, "5: Toggle Labels (" + std::string(show_labels ? "ON" : "OFF") + ")", 
                         glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
  res.main_font.add_text(left + 20.0f, top - 155.0f, 16, "6: Toggle Animation Demo (" + std::string(show_cpu_demo ? "ON" : "OFF") + ")", 
                         glm::vec4(show_cpu_demo ? 1.0f : 0.3f, show_cpu_demo ? 0.8f : 0.3f, show_cpu_demo ? 0.3f : 0.3f, 1.0f));

  render_end();
}

void DemoScreen::update() {
  // Update animation time
  static auto start_time = std::chrono::high_resolution_clock::now();
  auto current_time = std::chrono::high_resolution_clock::now();
  animation_time = std::chrono::duration<float>(current_time - start_time).count();
}

bool DemoScreen::handle_input(SDL_Event event) {
  // multiplex input
  if (DefaultScreen::handle_input(event)) {
    return true;
  } else if (event.type == SDL_KEYDOWN) {
    switch (event.key.keysym.sym) {
      case SDLK_1:
        show_circles = !show_circles;
        return true;
      case SDLK_2:
        show_rects = !show_rects;
        return true;
      case SDLK_3:
        show_simple_rects = !show_simple_rects;
        return true;
      case SDLK_4:
        show_lines = !show_lines;
        return true;
      case SDLK_5:
        show_labels = !show_labels;
        return true;
      case SDLK_6:
        show_cpu_demo = !show_cpu_demo;
        return true;
      default:
        break;
    }
  }
  return false;
}