#pragma once

#include <memory>
#include <stack>
#include <string>

#include <SDL.h>

#include "resources.h"
#include "screens/screen.h"

class Game {
public:
  Game(int width, int height, bool fullscreen, const std::string &title);
  ~Game();
  void run();
  void push(std::shared_ptr<Screen> screen);
  void pop();
  void change(const std::shared_ptr<Screen> &screen);
  [[nodiscard]] bool is_running() const;
  void stop();
  void handle_input(SDL_Event event);
  Resources &get_resources();

  void toggle_vsync();

private:
  std::stack<std::shared_ptr<Screen>> screen_stack;
  std::unique_ptr<Resources> resources{};

  SDL_Window *window{nullptr};
  SDL_GLContext main_context{nullptr};

  bool running{true};
  bool vsync{true};

  void resize();
};
