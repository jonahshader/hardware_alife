#include "snn_test.h"
#include "audio/audio_manager.h"
#include <cmath>
#include <numbers>

SNNTestScreen::SNNTestScreen(ScreenContext &ctx) : Screen(ctx), rng(std::random_device{}()) {
  // Initialize Fourier feature parameters
  std::uniform_real_distribution<float> phase_dist(0.0f, 2.0f * std::numbers::pi_v<float>);
  std::uniform_real_distribution<float> freq_dist(1.0f, 8.0f);

  for (int i = 0; i < INPUTS; ++i) {
    fourier_phases[i] = phase_dist(rng);
    fourier_freqs[i] = freq_dist(rng);
  }

  network.init(rng);

  // Initialize audio source
  cached_audio_source = std::make_shared<CachedAudioSource>();
}

SNNTestScreen::~SNNTestScreen() = default;

void SNNTestScreen::show() {
  Screen::show();
  network.clear();

  // Add audio source when screen becomes active
  if (cached_audio_source) {
    AudioManager::instance().add_source(cached_audio_source.get());
  }
}

void SNNTestScreen::hide() {
  Screen::hide();

  // Remove audio source when screen becomes inactive
  if (cached_audio_source) {
    AudioManager::instance().remove_source(cached_audio_source.get());
  }
}

void SNNTestScreen::update() {
  const auto &mouse = get_mouse_state();

  // Handle keyboard to adjust input multiplier
  if (is_key_just_pressed(SDL_SCANCODE_EQUALS) || is_key_just_pressed(SDL_SCANCODE_KP_PLUS)) {
    input_multiplier *= 1.1f;
    input_multiplier = std::clamp(input_multiplier, 0.1f, 5.0f);
  }
  if (is_key_just_pressed(SDL_SCANCODE_MINUS) || is_key_just_pressed(SDL_SCANCODE_KP_MINUS)) {
    input_multiplier *= 0.9f;
    input_multiplier = std::clamp(input_multiplier, 0.1f, 5.0f);
  }

  // Generate input from mouse position using Fourier features
  std::array<int8_t, INPUTS> input;
  generate_fourier_input(mouse.x, mouse.y, input);

  // Update the neural network
  network.update(input);

  // Trigger audio for all neurons with panning
  if (cached_audio_source) {
    for (int i = 0; i < HIDDEN; ++i) {
      if (network.act_hidden[i]) {
        // Map neuron index to pan: -1.0 (left) to 1.0 (right)
        float pan = (static_cast<float>(i) / (HIDDEN - 1)) * 2.0f - 1.0f;
        cached_audio_source->trigger_click(0.2f, 0.0f, pan);
      }
    }
  }
}

void SNNTestScreen::generate_fourier_input(float mouse_x, float mouse_y,
                                           std::array<int8_t, INPUTS> &input) {
  for (int i = 0; i < INPUTS; ++i) {
    // Generate Fourier features: sin/cos of different frequencies applied to mouse position
    float feature;
    if (i % 2 == 0) {
      // Even indices: sin features
      feature = std::sin(fourier_freqs[i] * mouse_x + fourier_phases[i]);
    } else {
      // Odd indices: cos features
      feature = std::cos(fourier_freqs[i] * mouse_y + fourier_phases[i]);
    }

    // Apply some transforms
    feature *= input_multiplier;
    feature = std::clamp(feature, -1.0f, 1.0f);
    input[i] = static_cast<int8_t>(feature * 127.0f);
  }
}

void SNNTestScreen::render(Framebuffer &fb) {
  // Clear background
  // for (int y = 0; y < fb.height(); ++y) {
  //   for (int x = 0; x < fb.width(); ++x) {
  //     fb.at(x, y) = Pixel(32, 32, 32, 255);
  //   }
  // }

  // Simple 1x1 pixel visualization, left-aligned
  const auto &mouse = get_mouse_state();
  std::array<int8_t, INPUTS> current_input;
  generate_fourier_input(mouse.x, mouse.y, current_input);

  // Input (orange)
  int j = 0;
  static int row = 0;
  for (int i = 0; i < INPUTS && i < fb.width(); ++i) {
    auto val = current_input[i];
    if (val >= 0) {
      fb.at(j, row) = Pixel(val * 2, val, 0, 255);
    } else {
      fb.at(j, row) = Pixel(-val, 0, -val * 2, 255);
    }
    ++j;
  }

  // Hidden state (green)
  for (int i = 0; i < HIDDEN && i < fb.width(); ++i) {
    uint8_t state_val = network.s_hidden[i];
    uint8_t green_intensity = std::max(state_val, (uint8_t)64);
    fb.at(j, row) = Pixel(0, green_intensity, green_intensity / 2, 255);
    ++j;
  }

  // Hidden activation (red)
  for (int i = 0; i < HIDDEN && i < fb.width(); ++i) {
    bool is_active = network.act_hidden[i];
    uint8_t intensity = is_active ? 255 : 80;
    fb.at(j, row) = Pixel(intensity, 0, 0, 255);
    ++j;
  }

  // Output (cyan)
  std::vector<int16_t> output;
  network.get_output(output);
  for (int i = 0; i < OUTPUTS && i < fb.width(); ++i) {
    int16_t val = output[i];
    uint8_t normalized = static_cast<uint8_t>(std::clamp(val + 128, 0, 255));
    fb.at(j, row) = Pixel(0, normalized, normalized / 2, 255);
    ++j;
  }

  row = (row + 1) % fb.height();

  // // Draw mouse crosshair
  // const auto &mouse_state = get_mouse_state();
  // int mouse_pixel_x = (int)(mouse_state.x * fb.width());
  // int mouse_pixel_y = (int)(mouse_state.y * fb.height());

  // // Horizontal line
  // for (int x = 0; x < fb.width(); ++x) {
  //   if (mouse_pixel_y >= 0 && mouse_pixel_y < fb.height()) {
  //     Pixel &p = fb.at(x, mouse_pixel_y);
  //     p.r = 255 - p.r;
  //     p.g = 255 - p.g;
  //     p.b = 255 - p.b;
  //   }
  // }

  // // Vertical line
  // for (int y = 0; y < fb.height(); ++y) {
  //   if (mouse_pixel_x >= 0 && mouse_pixel_x < fb.width()) {
  //     Pixel &p = fb.at(mouse_pixel_x, y);
  //     p.r = 255 - p.r;
  //     p.g = 255 - p.g;
  //     p.b = 255 - p.b;
  //   }
  // }
}
