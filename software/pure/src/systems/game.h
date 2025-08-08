#pragma once

#include <SDL3/SDL.h>

#include "screens/screen.h"

#include <memory>
#include <string>
#include <vector>

class Game {
public:
  explicit Game(int internal_width = 128, int internal_height = 128, const std::string &title);
  ~Game();
  void run();
  void push(const std::shared_ptr<Screen> &screen);
  void pop();
  void change(const std::shared_ptr<Screen> &screen);
  [[nodiscard]] bool is_running() const;
  void stop();
  void handle_input(const SDL_Event &event);
  void set_vsync(bool enabled);
  void set_fullscreen(bool enabled);

private:
  SDL_Window *window{nullptr};
  SDL_Renderer *renderer{nullptr};
  std::vector<std::shared_ptr<Screen>> screens{};

  int internal_width;
  int internal_height;
  SDL_Texture *frame{nullptr};
  SDL_AudioStream *audio_stream{nullptr};

  bool running{true};
  bool vsync{true};
  bool fullscreen{false};

  void resize();
};
