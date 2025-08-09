#pragma once

#include "audio_source.h"
#include "lock_free_ring_buffer.h"
#include <atomic>
#include <cstdint>

enum class EventType {
  CLICK,
  BEEP,
  EXPLOSION
};

class EventAudioSource : public AudioSource {
public:
  EventAudioSource();
  ~EventAudioSource() override = default;
  
  void generate_samples(float* left_buffer, float* right_buffer, int sample_count) override;
  
  // Trigger events from any thread
  void trigger_click(float amplitude = 0.5f, float jitter_ms = 0.0f);
  void trigger_beep(float amplitude = 0.3f, float jitter_ms = 0.0f);
  void trigger_explosion(float amplitude = 1.0f, float jitter_ms = 0.0f);
  
  // Get current sample position for external timing calculations
  uint64_t get_current_sample() const { return sample_position_.load(std::memory_order_relaxed); }

private:
  struct SoundInstance {
    uint64_t start_sample;
    EventType type;
    float amplitude;
    bool finished = false;
    
    bool should_play_at(uint64_t current_sample) const {
      return current_sample >= start_sample && !finished;
    }
    
    float get_sample_value(uint64_t current_sample, float sample_rate);
  };
  
  void trigger_event(EventType type, float amplitude, float jitter_ms);
  
  // Generate specific sound types
  static float generate_click_sample(uint64_t local_sample, float amplitude, float sample_rate);
  static float generate_beep_sample(uint64_t local_sample, float amplitude, float sample_rate);
  static float generate_explosion_sample(uint64_t local_sample, float amplitude, float sample_rate);
  
  LockFreeRingBuffer<SoundInstance, 1024> active_sounds_;
  std::atomic<uint64_t> sample_position_{0};
  
  static constexpr float SAMPLE_RATE = 44100.0f;
};