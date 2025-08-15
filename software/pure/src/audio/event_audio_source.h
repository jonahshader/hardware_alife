#pragma once

#include "audio_source.h"
#include "lock_free_ring_buffer.h"
#include "sound_generators.h"
#include <atomic>
#include <chrono>
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
  void trigger_click(float amplitude = 0.5f, float jitter_ms = 0.0f, float pan = 0.0f);
  void trigger_beep(float amplitude = 0.3f, float jitter_ms = 0.0f, float pan = 0.0f);
  void trigger_explosion(float amplitude = 1.0f, float jitter_ms = 0.0f, float pan = 0.0f);
  
  // Get current sample position for external timing calculations
  uint64_t get_current_sample() const { return sample_position_.load(std::memory_order_relaxed); }

private:
  struct SoundInstance {
    uint64_t start_sample;
    EventType type;
    float amplitude;
    float pan;  // -1.0 = left, 0.0 = center, 1.0 = right
    bool finished = false;
    
    bool should_play_at(uint64_t current_sample) const {
      return current_sample >= start_sample && !finished;
    }
    
    float get_sample_value(uint64_t current_sample, float sample_rate);
  };
  
  void trigger_event(EventType type, float amplitude, float jitter_ms, float pan);
  
  // Convert EventType to SoundGenerators::SoundType
  static SoundGenerators::SoundType event_to_sound_type(EventType type);
  
  LockFreeRingBuffer<SoundInstance, 1024> active_sounds_;
  std::atomic<uint64_t> sample_position_{0};
  std::atomic<std::chrono::high_resolution_clock::time_point> last_callback_time_;
  
  static constexpr float SAMPLE_RATE = 44100.0f;
};