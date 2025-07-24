#pragma once

#include "default.cuh"
#include "systems/game.cuh"

class DemoScreen : public DefaultScreen {
public:
  explicit DemoScreen(Game &game);

  void render() override;
  void update() override;
  bool handle_input(SDL_Event event) override;

private:
  bool show_circles = true;
  bool show_rects = true;
  bool show_simple_rects = true;
  bool show_lines = true;
  bool show_labels = true;
  bool show_cuda_demo = true;
  
  float animation_time = 0.0f;
};
