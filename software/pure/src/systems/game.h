#pragma once

#include <SDL3/SDL.h>

#include "audio/audio_manager.h"
#include "screens/screen.h"
#include "screens/screen_context.h"
#include "graphics/framebuffer.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

class Game : public ScreenContext {
public:
  explicit Game(int internal_width = 128, int internal_height = 128, const std::string &title = "Hardware ALife");
  ~Game();
  void run();
  [[nodiscard]] bool is_running() const;
  void handle_input(const SDL_Event &event);

  // ScreenContext interface
  std::pair<int, int> get_framebuffer_size() const override;
  void set_vsync(bool enabled) override;
  void set_fullscreen(bool enabled) override;
  void stop() override;
  void push_screen(std::unique_ptr<Screen> screen) override;
  void pop_screen() override;
  void change_screen(std::unique_ptr<Screen> screen) override;

private:
  SDL_Window *window{nullptr};
  SDL_Renderer *renderer{nullptr};
  std::vector<std::unique_ptr<Screen>> screens{};

  Framebuffer fb;
  SDL_Texture *frame{nullptr};

  bool running{true};
  bool vsync{true};
  bool fullscreen{false};

  void resize();
};
