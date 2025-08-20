#pragma once

#include <memory>
#include <utility>

class Screen;

class ScreenContext {
public:
  virtual ~ScreenContext() = default;

  virtual std::pair<int, int> get_framebuffer_size() const = 0;
  virtual void set_vsync(bool enabled) = 0;
  virtual void set_framerate_limit(float fps) = 0;
  virtual void set_fullscreen(bool enabled) = 0;
  virtual void stop() = 0;

  virtual void push_screen(std::unique_ptr<Screen> screen) = 0;
  virtual void pop_screen() = 0;
  virtual void change_screen(std::unique_ptr<Screen> screen) = 0;
};
