#pragma once

#include "screen.h"
#include "audio/audio_source.h"
#include "audio/event_audio_source.h"
#include <memory>

class TestScreen : public Screen {
public:
  TestScreen(ScreenContext& ctx);
  ~TestScreen() override;

  void show() override;
  void hide() override;
  void update() override;
  void render(Framebuffer &fb) override;


private:
  float time = 0.0f;

  // Audio test sources
  std::unique_ptr<AudioSource> sine_wave_source;
  std::shared_ptr<EventAudioSource> event_audio_source;
};
