#pragma once

#include "SDL.h"

class Screen {
public:
  virtual void show() = 0; // called when screen becomes the current screen
  virtual void update() = 0;
  virtual void render() = 0;
  virtual void resize(int width, int height) = 0;
  virtual void hide() = 0; // called when screen is no longer the current screen
  virtual bool handle_input(SDL_Event event) = 0;

  virtual ~Screen() = default;
};

