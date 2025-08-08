#include <SDL3/SDL.h>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>

struct AudioData {
  float phase = 0.0f;
  float frequency = 440.0f;
  float pan = 0.5f;  // 0.0 = left, 1.0 = right, 0.5 = center
  float volume = 0.1f;  // Keep volume low
};

static void AudioCallback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount) {
  AudioData *audio_data = static_cast<AudioData*>(userdata);
  const int sample_count = additional_amount / (sizeof(float) * 2);  // Stereo samples
  
  std::vector<float> samples(sample_count * 2);  // Stereo
  const float sample_rate = 44100.0f;
  
  for (int i = 0; i < sample_count; ++i) {
    float sample = std::sin(audio_data->phase * 2.0f * std::numbers::pi_v<float>) * audio_data->volume;
    
    // Apply panning
    float left_sample = sample * (1.0f - audio_data->pan);
    float right_sample = sample * audio_data->pan;
    
    samples[i * 2] = left_sample;      // Left channel
    samples[i * 2 + 1] = right_sample; // Right channel
    
    audio_data->phase += audio_data->frequency / sample_rate;
    if (audio_data->phase >= 1.0f) {
      audio_data->phase -= 1.0f;
    }
  }
  
  SDL_PutAudioStreamData(stream, samples.data(), additional_amount);
}

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_CreateWindowAndRenderer("SDL3 Audio Interactive Example", 800, 600, 0, &window, &renderer);
  
  // Audio setup
  AudioData audio_data;
  SDL_AudioSpec spec;
  spec.channels = 2;  // Stereo for panning
  spec.format = SDL_AUDIO_F32;
  spec.freq = 44100;
  
  SDL_AudioStream *audio_stream = SDL_OpenAudioDeviceStream(
    SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, AudioCallback, &audio_data);
  
  if (!audio_stream) {
    SDL_Log("Failed to open audio: %s", SDL_GetError());
  } else {
    SDL_ResumeAudioStreamDevice(audio_stream);  // Make sure audio is playing
  }
  
  // Enable VSync for proper frame rate limiting
  SDL_SetRenderVSync(renderer, 1);

  SDL_Texture *texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 128);

  // Set nearest-neighbor scaling
  SDL_SetTextureScaleMode(
      texture,
      SDL_SCALEMODE_NEAREST); // Defaults to linear otherwise :contentReference[oaicite:3]{index=3}

  bool running = true;
  SDL_Event e;
  uint32_t framebuffer[128 * 128];
  float mouse_x = 0.5f, mouse_y = 0.5f;

  while (running) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT) {
        running = false;
      } else if (e.type == SDL_EVENT_MOUSE_MOTION) {
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        mouse_x = (float)e.motion.x / w;  // Normalize to 0-1
        mouse_y = (float)e.motion.y / h;  // Normalize to 0-1
        
        // Update audio parameters
        audio_data.pan = mouse_x;  // Pan based on X position
        audio_data.frequency = 220.0f + (1.0f - mouse_y) * 660.0f;  // Pitch based on Y (inverted)
      }
    }

    for (int i = 0; i < 128 * 128; ++i) {
      framebuffer[i] = 0xFFFF00FF; // solid yellow (RGBA)
    }

    SDL_UpdateTexture(texture, nullptr, framebuffer, 128 * sizeof(uint32_t));
    SDL_RenderClear(renderer);

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    float scale = std::min(w / 128.0f, h / 128.0f);

    SDL_FRect dst = {.x = (w - 128 * scale) / 2.0f,
                     .y = (h - 128 * scale) / 2.0f,
                     .w = 128 * scale,
                     .h = 128 * scale};

    SDL_RenderTexture(renderer, texture, nullptr, &dst);
    SDL_RenderPresent(renderer);
  }

  SDL_DestroyAudioStream(audio_stream);
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
