#pragma once

#include "screen.h"
#include "../systems/snn.h"
#include "audio/cached_audio_source.h"
#include <array>
#include <memory>
#include <random>

class SNNTestScreen : public Screen {
public:
  SNNTestScreen(ScreenContext& ctx);
  ~SNNTestScreen() override;

  void show() override;
  void hide() override;
  void update() override;
  void render(Framebuffer &fb) override;

private:
  static constexpr int INPUTS = 16;
  static constexpr int HIDDEN = 32;
  static constexpr int OUTPUTS = 8;

  SNN<INPUTS, HIDDEN, OUTPUTS> network;
  std::mt19937 rng;

  // Fourier feature parameters for mouse input
  std::array<float, INPUTS> fourier_phases;
  std::array<float, INPUTS> fourier_freqs;
  float input_multiplier = 1.0f;

  // Audio for spike sounds
  std::shared_ptr<CachedAudioSource> cached_audio_source;

  void generate_fourier_input(float mouse_x, float mouse_y, std::array<uint8_t, INPUTS>& input);
};
