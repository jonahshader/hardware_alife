#pragma once

#include <cstdint>

class AudioSource {
public:
  virtual ~AudioSource() = default;
  
  virtual void generate_samples(float* left_buffer, float* right_buffer, int sample_count) = 0;
  
  virtual bool is_active() const { return true; }
  
  virtual float get_volume() const { return 1.0f; }
  
  virtual void set_volume(float volume) {}
  
  virtual void start() {}
  virtual void stop() {}
  virtual void pause() {}
  virtual void resume() {}
};