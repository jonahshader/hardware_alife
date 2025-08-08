#pragma once

#include <SDL3/SDL.h>

class Screen {
public:
  virtual void show(){}; // called when screen becomes the current screen
  virtual void hide(){}; // called when screen is no longer the current screen
  virtual void update(){};
  virtual void render(){};
  virtual bool handle_input(SDL_Event event){};

  virtual ~Screen() = default;
};
