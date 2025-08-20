#include "game.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <vector>

Game::Game(int internal_width, int internal_height, const std::string &title)
    : fb(internal_width, internal_height) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  SDL_CreateWindowAndRenderer("Hardware ALife", internal_width * 4, internal_height * 4, 0, &window,
                              &renderer);

  SDL_SetRenderVSync(renderer, vsync);
  SDL_SetWindowFullscreen(window, fullscreen);
  SDL_SetWindowResizable(window, true);

  // create virtual framebuffer
  frame = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
                            internal_width, internal_height);
  // set nearest-neighbor scaling
  SDL_SetTextureScaleMode(frame, SDL_SCALEMODE_NEAREST);

  // initialize audio manager
  AudioManager::instance().initialize();
}

Game::~Game() {
  AudioManager::instance().shutdown();

  if (frame) {
    SDL_DestroyTexture(frame);
  }
  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }
  if (window) {
    SDL_DestroyWindow(window);
  }
  SDL_Quit();
}

void Game::run() {
  SDL_Event e;
  Uint64 last_frame_time = SDL_GetTicks();
  fps_timer = SDL_GetTicks();
  frame_count = 0;
  while (running) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT) {
        running = false;
      } else {
        // iterate through screen in reverse order, feeding them the event until one of them handles
        // it
        for (auto it = screens.rbegin(); it != screens.rend(); ++it) {
          if ((*it)->handle_input(e)) {
            break; // event handled, stop processing
          }
        }
      }
    }

    // update and render current screen
    if (!screens.empty()) {
      screens.back()->update();
      screens.back()->render(fb);
    }

    // update texture with framebuffer data and render to screen
    SDL_UpdateTexture(frame, nullptr, fb.data(), fb.width() * sizeof(Pixel));
    SDL_RenderClear(renderer);

    // scale framebuffer to fit window while maintaining aspect ratio
    int window_w, window_h;
    SDL_GetWindowSize(window, &window_w, &window_h);
    float scale = std::min((float)window_w / fb.width(), (float)window_h / fb.height());

    SDL_FRect dst = {
      .x = (window_w - fb.width() * scale) / 2.0f,
      .y = (window_h - fb.height() * scale) / 2.0f,
      .w = fb.width() * scale,
      .h = fb.height() * scale
    };

    SDL_RenderTexture(renderer, frame, nullptr, &dst);
    SDL_RenderPresent(renderer);

    // FPS tracking
    frame_count++;
    Uint64 current_time = SDL_GetTicks();
    if (current_time - fps_timer >= 1000) { // 1 second elapsed
      float avg_fps = frame_count * 1000.0f / (current_time - fps_timer);
      printf("Average FPS: %.1f\n", avg_fps);
      frame_count = 0;
      fps_timer = current_time;
    }

    // Apply framerate limiting when vsync is disabled
    if (!vsync && framerate_limit > 0.0f) {
      float target_frame_time = 1000.0f / framerate_limit; // milliseconds per frame
      float elapsed = (float)(current_time - last_frame_time);

      if (elapsed < target_frame_time) {
        SDL_Delay((Uint32)(target_frame_time - elapsed));
      }
    }
    last_frame_time = SDL_GetTicks();
  }
}

std::pair<int, int> Game::get_framebuffer_size() const {
  return {fb.width(), fb.height()};
}

void Game::push_screen(std::unique_ptr<Screen> screen) {
  if (!screens.empty()) {
    screens.back()->hide();
  }
  screens.push_back(std::move(screen));
  screens.back()->show();
}

void Game::pop_screen() {
  if (!screens.empty()) {
    screens.back()->hide();
    screens.pop_back();
    if (!screens.empty()) {
      screens.back()->show();
    }
  }
}

void Game::change_screen(std::unique_ptr<Screen> screen) {
  if (!screens.empty()) {
    screens.back()->hide();
    screens.pop_back();
  }
  screens.push_back(std::move(screen));
  screens.back()->show();
}

bool Game::is_running() const {
  return running;
}

void Game::stop() {
  running = false;
}

void Game::handle_input(const SDL_Event &event) {
  if (!screens.empty() && running) {
    screens.back()->handle_input(event);
  }
}

void Game::set_vsync(bool enabled) {
  vsync = enabled;
  SDL_SetRenderVSync(renderer, vsync ? 1 : 0);
}

void Game::set_framerate_limit(float fps) {
  framerate_limit = fps;
}

void Game::set_fullscreen(bool enabled) {
  fullscreen = enabled;
  SDL_SetWindowFullscreen(window, fullscreen);
}
