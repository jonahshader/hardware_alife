#pragma once

#include "graphics/renderers/font.h"
#include "graphics/renderers/circle.cuh"
#include "graphics/renderers/rect.cuh"
#include "graphics/renderers/line.cuh"
#include "graphics/renderers/simple_rect.cuh"

class Resources {
public:
  RectRenderer rect{};
  SimpleRectRenderer simple_rect{};
  CircleRenderer circle{};
  LineRenderer line{};
  FontRenderer main_font{"fonts/OpenSans-Regular.arfont"};
  FontRenderer main_font_world{"fonts/OpenSans-Regular.arfont"};
  FontRenderer extra_bold_font{"fonts/OpenSans-ExtraBold.arfont"};
  FontRenderer fira_regular_font{"fonts/FiraSans-Regular.arfont"};

  void begin() {
    rect.begin();
    simple_rect.begin();
    circle.begin();
    line.begin();
    main_font.begin();
    main_font_world.begin();
    extra_bold_font.begin();
    fira_regular_font.begin();
  }

  void end() {
    rect.end();
    simple_rect.end();
    circle.end();
    line.end();
    main_font.end();
    main_font_world.end();
    extra_bold_font.end();
    fira_regular_font.end();
  }

  void set_transform(const glm::mat4 &transform) {
    rect.set_transform(transform);
    simple_rect.set_transform(transform);
    circle.set_transform(transform);
    line.set_transform(transform);
    main_font.set_transform(transform);
    main_font_world.set_transform(transform);
    extra_bold_font.set_transform(transform);
    fira_regular_font.set_transform(transform);
  }

  void render() {
    rect.render();
    simple_rect.render();
    circle.render();
    line.render();
    main_font.render();
    main_font_world.render();
    extra_bold_font.render();
    fira_regular_font.render();
  }
};
