#include "demo.cuh"
#include <cuda_runtime.h>
#include <cmath>
#include <chrono>

// CUDA kernel to generate animated circles in a grid
__global__ void generate_circle_grid(unsigned int* circle_data, int grid_size, float time, float base_x, float base_y) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (idx >= grid_size * grid_size) return;
    
    int row = idx / grid_size;
    int col = idx % grid_size;
    
    // Calculate position
    float spacing = 80.0f;
    float x = base_x + col * spacing;
    float y = base_y + row * spacing;
    
    // Animate with sine waves
    float wave_x = sinf(time * 2.0f + col * 0.5f) * 15.0f;
    float wave_y = cosf(time * 1.5f + row * 0.3f) * 10.0f;
    x += wave_x;
    y += wave_y;
    
    // Animate radius with sine wave
    float base_radius = 15.0f;
    float radius_mod = sinf(time * 3.0f + (row + col) * 0.4f) * 8.0f;
    float radius = (base_radius + radius_mod) * 2.0f; // CircleRenderer doubles radius internally
    
    // Animate color based on position and time
    float hue = fmodf(time * 0.5f + (row * 0.1f + col * 0.15f), 1.0f);
    
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
    
    // Pack data into CircleRenderer format: x, y, radius, color
    int base_idx = idx * 4;
    circle_data[base_idx + 0] = __float_as_uint(x);
    circle_data[base_idx + 1] = __float_as_uint(y);
    circle_data[base_idx + 2] = __float_as_uint(radius);
    
    // Pack RGBA color into single uint (r, g, b, a in bytes)
    unsigned char rc = (unsigned char)(r * 255.0f);
    unsigned char gc = (unsigned char)(g * 255.0f);
    unsigned char bc = (unsigned char)(b * 255.0f);
    unsigned char ac = 255; // Full alpha
    
    unsigned int color = rc | (gc << 8) | (bc << 16) | (ac << 24);
    circle_data[base_idx + 3] = color;
}

DemoScreen::DemoScreen(Game &game) : DefaultScreen(game) {}

void DemoScreen::render() {
  render_start();

  auto &res = game.get_resources();
  
  // CUDA Circle Animation Demo
  if (show_cuda_demo) {
    const int grid_size = 8;
    const int total_circles = grid_size * grid_size;
    
    // Ensure CircleRenderer has enough capacity
    res.circle.ensure_vbo_capacity(total_circles);
    
    // Map CUDA buffer
    void* cuda_ptr = res.circle.cuda_map_buffer();
    if (cuda_ptr) {
      // Launch CUDA kernel to generate animated circles
      dim3 blockSize(256);
      dim3 gridSizeKernel((total_circles + blockSize.x - 1) / blockSize.x);
      
      generate_circle_grid<<<gridSizeKernel, blockSize>>>(
        (unsigned int*)cuda_ptr, 
        grid_size, 
        animation_time,
        200.0f,  // base_x (offset to right side)
        -200.0f  // base_y (offset to top)
      );
      
      // Wait for kernel to complete
      cudaDeviceSynchronize();
      
      // Unmap buffer
      res.circle.cuda_unmap_buffer();
      
      // Render the CUDA-generated circles
      res.circle.render(total_circles);
    } else {
      std::cerr << "Failed to map CUDA buffer for circle demo" << std::endl;
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
  res.main_font.add_text(left + 20.0f, top - 155.0f, 16, "6: Toggle CUDA Demo (" + std::string(show_cuda_demo ? "ON" : "OFF") + ")", 
                         glm::vec4(show_cuda_demo ? 1.0f : 0.3f, show_cuda_demo ? 0.8f : 0.3f, show_cuda_demo ? 0.3f : 0.3f, 1.0f));

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
        show_cuda_demo = !show_cuda_demo;
        return true;
      default:
        break;
    }
  }
  return false;
}
