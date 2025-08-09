#include "test_screen.h"
#include "audio/audio_manager.h"
#include <cmath>
#include <numbers>

// Simple sine wave audio source for testing
class SineWaveSource : public AudioSource {
public:
  SineWaveSource(float freq = 440.0f) : frequency(freq) {}

  void generate_samples(float* left_buffer, float* right_buffer, int sample_count) override {
    const float sample_rate = 44100.0f;

    // Load atomic values once per buffer to avoid repeated atomic operations
    const float freq = frequency.load(std::memory_order_relaxed);
    const float p = pan.load(std::memory_order_relaxed);
    const float vol = volume.load(std::memory_order_relaxed);

    for (int i = 0; i < sample_count; ++i) {
      float sample = std::sin(phase * 2.0f * std::numbers::pi_v<float>) * vol;

      // Apply panning
      left_buffer[i] = sample * (1.0f - p);
      right_buffer[i] = sample * p;

      phase += freq / sample_rate;
      if (phase >= 1.0f) {
        phase -= 1.0f;
      }
    }
  }

  void set_frequency(float freq) { frequency.store(freq, std::memory_order_relaxed); }
  void set_pan(float p) { pan.store(p, std::memory_order_relaxed); }
  void set_volume(float v) override { volume.store(v, std::memory_order_relaxed); }
  float get_volume() const override { return volume.load(std::memory_order_relaxed); }

private:
  float phase = 0.0f;  // Only accessed from audio thread, no atomics needed
  std::atomic<float> frequency{440.0f};
  std::atomic<float> pan{0.5f};      // 0.0 = left, 1.0 = right, 0.5 = center
  std::atomic<float> volume{0.1f};   // Keep volume low
};

TestScreen::TestScreen(ScreenContext& ctx) : Screen(ctx) {
  sine_wave_source = std::make_unique<SineWaveSource>(440.0f);
  event_audio_source = std::make_shared<EventAudioSource>();
}

TestScreen::~TestScreen() = default;

void TestScreen::show() {
  Screen::show();
  time = 0.0f;

  // Add audio sources when screen becomes active
  if (sine_wave_source) {
    AudioManager::instance().add_source(sine_wave_source.get());
  }
  if (event_audio_source) {
    AudioManager::instance().add_source(event_audio_source.get());
  }
}

void TestScreen::hide() {
  Screen::hide();
  // Remove audio sources when screen becomes inactive
  if (sine_wave_source) {
    AudioManager::instance().remove_source(sine_wave_source.get());
  }
  if (event_audio_source) {
    AudioManager::instance().remove_source(event_audio_source.get());
  }
}

void TestScreen::update() {
  // Simple animation
  time += 0.016f; // ~60fps
  
  const auto& mouse = get_mouse_state();

  // Update sine wave audio based on mouse position
  if (sine_wave_source) {
    auto* sine_source = static_cast<SineWaveSource*>(sine_wave_source.get());
    sine_source->set_pan(mouse.x);  // Pan based on X position
    sine_source->set_frequency(220.0f + (1.0f - mouse.y) * 660.0f);  // Pitch based on Y (inverted)
  }
  
  // Handle mouse clicks for sound effects using polling
  if (event_audio_source) {
    static bool prev_left = false;
    static bool prev_middle = false;
    static bool prev_right = false;
    
    // Check for mouse button press transitions (was up, now down)
    if (mouse.left_pressed && !prev_left) {
      event_audio_source->trigger_click(0.5f, 2.0f);  // Left click: click sound
    }
    if (mouse.middle_pressed && !prev_middle) {
      event_audio_source->trigger_beep(0.4f, 2.0f);   // Middle click: beep sound
    }
    if (mouse.right_pressed && !prev_right) {
      event_audio_source->trigger_explosion(0.8f, 2.0f);  // Right click: explosion sound
    }
    
    // Store current state for next frame
    prev_left = mouse.left_pressed;
    prev_middle = mouse.middle_pressed;
    prev_right = mouse.right_pressed;
  }
  
  // Test keyboard input
  if (is_key_just_pressed(SDL_SCANCODE_SPACE) && event_audio_source) {
    event_audio_source->trigger_beep(0.6f, 1.0f);  // Space key: beep
  }
  
  // Trigger random events to demonstrate EventAudioSource
  static float last_click_time = 0.0f;
  if (time - last_click_time > 1.0f && event_audio_source) {  // Every ~1 second
    // Simulate neural spikes with random timing and jitter
    event_audio_source->trigger_click(0.3f, 5.0f);  // 5ms jitter
    last_click_time = time;
  }
}

void TestScreen::render(Framebuffer &fb) {
  // Test framebuffer rendering with various patterns
  for (int y = 0; y < fb.height(); ++y) {
    for (int x = 0; x < fb.width(); ++x) {
      float fx = (float)x / fb.width();
      float fy = (float)y / fb.height();
      
      const auto& mouse = get_mouse_state();

      // Create animated pattern influenced by mouse position
      float wave = std::sin((fx + mouse.x) * 8.0f + time) * 0.5f + 0.5f;
      float wave2 = std::cos((fy + mouse.y) * 6.0f + time * 1.2f) * 0.5f + 0.5f;

      // Test different rendering patterns in different quadrants
      uint8_t r, g, b;

      if (fx < 0.5f && fy < 0.5f) {
        // Top-left: Gradient
        r = (uint8_t)(fx * 255);
        g = (uint8_t)(fy * 255);
        b = (uint8_t)((fx + fy) * 127);
      } else if (fx >= 0.5f && fy < 0.5f) {
        // Top-right: Animated waves
        r = (uint8_t)(wave * 255);
        g = (uint8_t)(wave2 * 255);
        b = (uint8_t)(128);
      } else if (fx < 0.5f && fy >= 0.5f) {
        // Bottom-left: Mouse-responsive color
        r = (uint8_t)(mouse.x * 255);
        g = (uint8_t)(mouse.y * 255);
        b = (uint8_t)((std::sin(time * 2.0f) * 0.5f + 0.5f) * 255);
      } else {
        // Bottom-right: Checkerboard pattern
        int checker = ((x / 8) + (y / 8)) % 2;
        uint8_t intensity = checker ? 255 : 64;
        r = intensity;
        g = intensity;
        b = intensity;
      }

      fb.at(x, y) = Pixel(r, g, b, 255);
    }
  }

  // Draw a crosshair at mouse position for visual feedback
  const auto& mouse = get_mouse_state();
  int mouse_pixel_x = (int)(mouse.x * fb.width());
  int mouse_pixel_y = (int)(mouse.y * fb.height());

  // Horizontal line
  for (int x = 0; x < fb.width(); ++x) {
    if (mouse_pixel_y >= 0 && mouse_pixel_y < fb.height()) {
      fb.at(x, mouse_pixel_y) = Pixel(255, 255, 255, 255);
    }
  }

  // Vertical line
  for (int y = 0; y < fb.height(); ++y) {
    if (mouse_pixel_x >= 0 && mouse_pixel_x < fb.width()) {
      fb.at(mouse_pixel_x, y) = Pixel(255, 255, 255, 255);
    }
  }
}

