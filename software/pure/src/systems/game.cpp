#include "game.h"

#include <SDL3/SDL.h> // asdf

Game::Game(int internal_width, int internal_height, const std::string &title)
    : internal_width(internal_width), internal_height(internal_height) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  SDL_CreateWindowAndRenderer("Hardware ALife", internal_width * 4, internal_height * 4, 0, &window, &renderer);

  SDL_SetRenderVSync(renderer, vsync);
  SDL_SetWindowFullscreen(window, fullscreen);

  // create virtual framebuffer
  frame = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
                            internal_width, internal_height);
  // set nearest-neighbor scaling
  SDL_SetTextureScaleMode(frame, SDL_SCALEMODE_NEAREST);

  // audio setup
  SDL_AudioSpec spec;
  spec.channels = 2;
  spec.format = SDL_AUDIO_S16;
  spec.freq = 44100;
  audio_stream = SDL_OpenAudioDeviceStream(
    SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, AudioCallback, &audio_data);
}

Game::~Game() {

}
