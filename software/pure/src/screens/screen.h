#pragma once

#include "graphics/framebuffer.h"
#include "screen_context.h"
#include <SDL3/SDL.h>
#include <unordered_set>

class Screen {
public:
  Screen(ScreenContext& ctx) : context(ctx) {}

  virtual void show();
  virtual void hide();
  virtual void update(){};
  virtual void render(Framebuffer &fb) = 0;
  bool handle_input(SDL_Event event);

  virtual ~Screen() = default;

protected:
  struct MouseState {
    float x = 0.5f;
    float y = 0.5f;
    float raw_x = 0.0f;
    float raw_y = 0.0f;
    bool left_pressed = false;
    bool middle_pressed = false;
    bool right_pressed = false;
  };

  bool is_key_pressed(SDL_Scancode key) const;
  bool is_key_just_pressed(SDL_Scancode key) const;
  bool is_key_just_released(SDL_Scancode key) const;

  const MouseState& get_mouse_state() const { return mouse_state; }
  ScreenContext& get_context() { return context; }


private:
  void update_rendered_mouse_position(float raw_x, float raw_y, int window_width, int window_height);

  ScreenContext& context;
  std::unordered_set<SDL_Scancode> pressed_keys;
  std::unordered_set<SDL_Scancode> just_pressed_keys;
  std::unordered_set<SDL_Scancode> just_released_keys;
  MouseState mouse_state;
};
