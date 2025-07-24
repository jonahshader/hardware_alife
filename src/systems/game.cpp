#include "game.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>

#include <SDL.h>
#include "glad/glad.h"

static void sdl_die(const char *message) {
  fprintf(stderr, "%s: %s\n", message, SDL_GetError());
  exit(2);
}

Game::Game(int width, int height, bool fullscreen, const std::string &title) {
#ifdef _WIN32
  // Set DPI awareness for windows
  std::cout << "Enabling Process DPI awareness for Windows" << std::endl;
  SetProcessDPIAware();
#endif
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    sdl_die("Couldn't initialize SDL");
  atexit(SDL_Quit);
  SDL_GL_LoadLibrary(nullptr); // Default OpenGL is fine.

  // Request an OpenGL ES 2.0 context
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

  // Also request a depth buffer
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;
  flags |= fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_RESIZABLE;
  int w = fullscreen ? 0 : width;
  int h = fullscreen ? 0 : height;
  window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h,
                            flags);

  if (window == nullptr)
    sdl_die("Couldn't set video mode");

  main_context = SDL_GL_CreateContext(window);
  if (main_context == nullptr)
    sdl_die("Failed to create OpenGL context");

  // Check OpenGL ES properties
  printf("OpenGL ES loaded\n");
  gladLoadGLES2Loader(SDL_GL_GetProcAddress);
  printf("Vendor:   %s\n", glGetString(GL_VENDOR));
  printf("Renderer: %s\n", glGetString(GL_RENDERER));
  printf("Version:  %s\n", glGetString(GL_VERSION));

  // Use v-sync
  SDL_GL_SetSwapInterval(vsync ? 1 : 0);

  // Disable depth test, face culling.
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  SDL_GL_GetDrawableSize(window, &w, &h);
  glViewport(0, 0, w, h);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  // After init, create resources
  resources = std::make_unique<Resources>();
}

Game::~Game() {
  SDL_GL_DeleteContext(main_context);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void Game::run() {
  SDL_Event event;
  while (running) {
    while (SDL_PollEvent(&event)) {
      // TODO: if I do imgui, I need to handle it here
      handle_input(event);

      // handle quit and resize events
      if (event.type == SDL_QUIT) {
        stop();
      } else if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
          // Instead of using event.window.data1/data2, get the drawable size:
          int drawableW, drawableH;
          SDL_GL_GetDrawableSize(window, &drawableW, &drawableH);
          std::cout << "Window resized to " << event.window.data1 << "x" << event.window.data2
                    << std::endl;
          std::cout << "Drawable size: " << drawableW << "x" << drawableH << std::endl;
          resize();
          glViewport(0, 0, drawableW, drawableH);
        }
      }
    }

    if (!screen_stack.empty()) {
      auto top = screen_stack.top();
      // TODO: it would be cool if I could continue to go down the stack if the top screen is
      // semi-transparent. this would make popups easier to implement.
      top->update();
      top->render();
    }

    SDL_GL_SwapWindow(window);
  }
}

void Game::resize() {
  if (!screen_stack.empty()) {
    int width, height;
    SDL_GL_GetDrawableSize(window, &width, &height);
    screen_stack.top()->resize(width, height);
  }
}

void Game::push(std::shared_ptr<Screen> screen) {
  screen_stack.push(screen);
  resize();
}

void Game::pop() {
  if (!screen_stack.empty()) {
    screen_stack.top()->hide();
    screen_stack.pop();
    screen_stack.top()->show();
    resize();
  }
}

void Game::change(const std::shared_ptr<Screen> &screen) {
  if (!screen_stack.empty()) {
    screen_stack.top()->hide();
    screen_stack.pop();
  }
  screen_stack.push(screen);
  screen_stack.top()->show();
  resize();
}

Resources &Game::get_resources() {
  return *resources.get();
}

bool Game::is_running() const {
  return running;
}

void Game::stop() {
  running = false;
}

void Game::handle_input(SDL_Event event) {
  if (!screen_stack.empty())
    screen_stack.top()->handle_input(event);
}

void Game::toggle_vsync() {
  vsync = !vsync;
  SDL_GL_SetSwapInterval(vsync ? 1 : 0);
}
