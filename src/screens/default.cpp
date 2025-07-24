#include "default.h"

#include <chrono>

#include "glad/glad.h"

DefaultScreen::DefaultScreen(Game &game) : game(game) {}

void DefaultScreen::show() {
  last_time = std::chrono::high_resolution_clock::now();
}

void DefaultScreen::update() {}

void DefaultScreen::render() {
  render_start();
  render_end();
}

void DefaultScreen::resize(int width, int height) {
  vp.update(width, height);
  hud_vp.update(width, height);
}

void DefaultScreen::hide() {}

bool DefaultScreen::handle_input(SDL_Event event) {
  if (event.type == SDL_KEYDOWN) {
    keys_pressed.insert(event.key.keysym.sym);
    switch (event.key.keysym.sym) {
      case SDLK_ESCAPE:
        game.stop();
        return true;
      default:
        break;
    }
  } else if (event.type == SDL_KEYUP) {
    keys_pressed.erase(event.key.keysym.sym);
  } else if (event.type == SDL_MOUSEWHEEL) {
    vp.handle_scroll((float)event.wheel.y);
    return true;
  } else if (event.type == SDL_MOUSEMOTION) {
    if (event.motion.state & SDL_BUTTON_MMASK) {
      vp.handle_pan((float)event.motion.xrel, (float)event.motion.yrel);
      return true;
    }
  }

  return false;
}

void DefaultScreen::render_start() {
  // clear screen
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Enable blending for transparency
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // pan with keyboard
  // TODO: use .contains()
  constexpr float PAN_SPEED = 10.0f;
  constexpr float ZOOM_SPEED = 1.0f / 20.0f;
  if (keys_pressed.contains(SDLK_w)) {
    vp.handle_pan(0.0f, PAN_SPEED);
  }
  if (keys_pressed.contains(SDLK_s)) {
    vp.handle_pan(0.0f, -PAN_SPEED);
  }
  if (keys_pressed.contains(SDLK_a)) {
    vp.handle_pan(PAN_SPEED, 0.0f);
  }
  if (keys_pressed.contains(SDLK_d)) {
    vp.handle_pan(-PAN_SPEED, 0.0f);
  }
  if (keys_pressed.contains(SDLK_q)) {
    vp.handle_scroll(-ZOOM_SPEED);
  }
  if (keys_pressed.contains(SDLK_e)) {
    vp.handle_scroll(ZOOM_SPEED);
  }

  // setup shaders for rendering
  auto &res = game.get_resources();
  res.set_transform(vp.get_transform());
  res.main_font.set_transform(hud_vp.get_transform());

  // Set screen scale for anti-aliasing
  res.set_screen_scale_worldspace(vp.get_screen_scale_worldspace());
  res.set_screen_scale_screenspace(vp.get_screen_scale_screenspace());

  res.begin();
}

void DefaultScreen::render_end() {
  auto &res = game.get_resources();
  auto &regular = res.main_font;

  auto now = std::chrono::high_resolution_clock::now();
  float dt = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_time).count();
  auto left = hud_vp.get_left();
  auto bottom = hud_vp.get_bottom();
  regular.add_text(left, bottom + 30.0f, 100, "dt: " + std::to_string(dt) + "s",
                   glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), FontRenderer::HAlign::RIGHT);
  regular.add_text(left, bottom + 00.0f, 150, "fps: " + std::to_string(1.0f / dt),
                   glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), FontRenderer::HAlign::RIGHT);
  last_time = now;

  // finish rendering
  res.end();
  res.render();
}
