#pragma once

#include "graphics/framebuffer.h"
#include "screen.h"

class GPUTestScreen : public Screen {
public:
  GPUTestScreen(ScreenContext &ctx);
  ~GPUTestScreen() override = default;

  void render(Framebuffer &fb) override;
};
