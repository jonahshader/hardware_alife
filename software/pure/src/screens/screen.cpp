#include "screen.h"
#include <algorithm>

void Screen::show() {
  pressed_keys.clear();
  just_pressed_keys.clear();
  just_released_keys.clear();
  mouse_state = MouseState{};
}

void Screen::hide() {
  pressed_keys.clear();
  just_pressed_keys.clear();
  just_released_keys.clear();
}

bool Screen::handle_input(SDL_Event event) {
  just_pressed_keys.clear();
  just_released_keys.clear();

  switch (event.type) {
    case SDL_EVENT_KEY_DOWN:
      if (!event.key.repeat) {
        just_pressed_keys.insert(event.key.scancode);
        pressed_keys.insert(event.key.scancode);
      }
      return true;

    case SDL_EVENT_KEY_UP:
      just_released_keys.insert(event.key.scancode);
      pressed_keys.erase(event.key.scancode);
      return true;

    case SDL_EVENT_MOUSE_MOTION: {
      int window_width, window_height;
      SDL_GetWindowSize(SDL_GetWindowFromID(event.motion.windowID), &window_width, &window_height);

      mouse_state.raw_x = (float)event.motion.x / window_width;
      mouse_state.raw_y = (float)event.motion.y / window_height;

      update_rendered_mouse_position(mouse_state.raw_x, mouse_state.raw_y, window_width, window_height);
      return true;
    }

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
      switch (event.button.button) {
        case SDL_BUTTON_LEFT: mouse_state.left_pressed = true; break;
        case SDL_BUTTON_MIDDLE: mouse_state.middle_pressed = true; break;
        case SDL_BUTTON_RIGHT: mouse_state.right_pressed = true; break;
      }
      return true;

    case SDL_EVENT_MOUSE_BUTTON_UP:
      switch (event.button.button) {
        case SDL_BUTTON_LEFT: mouse_state.left_pressed = false; break;
        case SDL_BUTTON_MIDDLE: mouse_state.middle_pressed = false; break;
        case SDL_BUTTON_RIGHT: mouse_state.right_pressed = false; break;
      }
      return true;
  }

  return false;
}

bool Screen::is_key_pressed(SDL_Scancode key) const {
  return pressed_keys.find(key) != pressed_keys.end();
}

bool Screen::is_key_just_pressed(SDL_Scancode key) const {
  return just_pressed_keys.find(key) != just_pressed_keys.end();
}

bool Screen::is_key_just_released(SDL_Scancode key) const {
  return just_released_keys.find(key) != just_released_keys.end();
}

void Screen::update_rendered_mouse_position(float raw_x, float raw_y, int window_width, int window_height) {
  auto [fb_width, fb_height] = context.get_framebuffer_size();

  float window_aspect = (float)window_width / window_height;
  float fb_aspect = (float)fb_width / fb_height;

  if (window_aspect > fb_aspect) {
    float scale = (float)window_height / fb_height;
    float rendered_width = fb_width * scale;
    float x_offset = (window_width - rendered_width) * 0.5f;

    if (raw_x * window_width >= x_offset && raw_x * window_width <= x_offset + rendered_width) {
      mouse_state.x = (raw_x * window_width - x_offset) / rendered_width;
      mouse_state.y = raw_y;
    }
  } else {
    float scale = (float)window_width / fb_width;
    float rendered_height = fb_height * scale;
    float y_offset = (window_height - rendered_height) * 0.5f;

    if (raw_y * window_height >= y_offset && raw_y * window_height <= y_offset + rendered_height) {
      mouse_state.x = raw_x;
      mouse_state.y = (raw_y * window_height - y_offset) / rendered_height;
    }
  }

  mouse_state.x = std::clamp(mouse_state.x, 0.0f, 1.0f);
  mouse_state.y = std::clamp(mouse_state.y, 0.0f, 1.0f);
}
