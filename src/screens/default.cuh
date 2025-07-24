#pragma once

#include <chrono>
#include <unordered_set>

#include "screen.h"
#include "systems/game.cuh"
#include "graphics/viewports/extend.h"

class DefaultScreen : public Screen {
public:
  explicit DefaultScreen(Game &game);
  ~DefaultScreen() override = default;

  virtual void show() override;
  virtual void update() override;
  virtual void render() override;
  virtual void resize(int width, int height) override;
  virtual void hide() override;
  virtual bool handle_input(SDL_Event event) override;

protected:
  Game &game;
  ExtendViewport vp{720, 720};
  ExtendViewport hud_vp{720, 720};
  std::unordered_set<int> keys_pressed{};

  std::chrono::high_resolution_clock::time_point last_time;

  void render_start();
  void render_end();
};
