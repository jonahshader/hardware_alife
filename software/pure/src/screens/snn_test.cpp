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
  event_audio_source = std::make_shared<EventAudioSource>();
}

SNNTestScreen::~SNNTestScreen() = default;

void SNNTestScreen::show() {
  Screen::show();
  network.clear();

  // Add audio source when screen becomes active
  if (event_audio_source) {
    AudioManager::instance().add_source(event_audio_source.get());
  }
}

void SNNTestScreen::hide() {
  Screen::hide();

  // Remove audio source when screen becomes inactive
  if (event_audio_source) {
    AudioManager::instance().remove_source(event_audio_source.get());
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
  std::array<uint8_t, INPUTS> input;
  generate_fourier_input(mouse.x, mouse.y, input);

  // Update the neural network
  network.update(input);

  // Trigger audio for first neuron only
  if (event_audio_source && network.act_hidden[0]) {
    event_audio_source->trigger_click(0.3f, 5.0f);
  }
}

void SNNTestScreen::generate_fourier_input(float mouse_x, float mouse_y,
                                           std::array<uint8_t, INPUTS> &input) {
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
    feature = (feature * 0.5f + 0.5f);
    feature *= input_multiplier;
    feature = std::clamp(feature, 0.0f, 1.0f);
    input[i] = static_cast<uint8_t>(feature * 255.0f);
  }
}

void SNNTestScreen::render(Framebuffer &fb) {
  // Clear background
  for (int y = 0; y < fb.height(); ++y) {
    for (int x = 0; x < fb.width(); ++x) {
      fb.at(x, y) = Pixel(32, 32, 32, 255);
    }
  }

  // Simple 1x1 pixel visualization, left-aligned
  const auto &mouse = get_mouse_state();
  std::array<uint8_t, INPUTS> current_input;
  generate_fourier_input(mouse.x, mouse.y, current_input);

  // Row 1: Input (orange)
  for (int i = 0; i < INPUTS && i < fb.width(); ++i) {
    uint8_t val = current_input[i];
    fb.at(i, 0) = Pixel(val, val / 2, 0, 255);
  }

  // Row 2: Hidden state (green) 
  for (int i = 0; i < HIDDEN && i < fb.width(); ++i) {
    uint8_t state_val = network.s_hidden[i];
    uint8_t green_intensity = std::max(state_val, (uint8_t)64);
    fb.at(i, 2) = Pixel(0, green_intensity, green_intensity/2, 255);
  }

  // Row 3: Hidden activation (red)
  for (int i = 0; i < HIDDEN && i < fb.width(); ++i) {
    bool is_active = network.act_hidden[i];
    uint8_t intensity = is_active ? 255 : 80;
    fb.at(i, 4) = Pixel(intensity, 0, 0, 255);
  }

  // Row 4: Output (cyan)
  std::vector<int16_t> output;
  network.get_output(output);
  for (int i = 0; i < OUTPUTS && i < fb.width(); ++i) {
    int16_t val = output[i];
    uint8_t normalized = static_cast<uint8_t>(std::clamp(val + 128, 0, 255));
    fb.at(i, 6) = Pixel(0, normalized, normalized / 2, 255);
  }

  // Draw mouse crosshair
  const auto &mouse_state = get_mouse_state();
  int mouse_pixel_x = (int)(mouse_state.x * fb.width());
  int mouse_pixel_y = (int)(mouse_state.y * fb.height());

  // Horizontal line
  for (int x = 0; x < fb.width(); ++x) {
    if (mouse_pixel_y >= 0 && mouse_pixel_y < fb.height()) {
      Pixel &p = fb.at(x, mouse_pixel_y);
      p.r = 255 - p.r;
      p.g = 255 - p.g;
      p.b = 255 - p.b;
    }
  }

  // Vertical line
  for (int y = 0; y < fb.height(); ++y) {
    if (mouse_pixel_x >= 0 && mouse_pixel_x < fb.width()) {
      Pixel &p = fb.at(mouse_pixel_x, y);
      p.r = 255 - p.r;
      p.g = 255 - p.g;
      p.b = 255 - p.b;
    }
  }
}
