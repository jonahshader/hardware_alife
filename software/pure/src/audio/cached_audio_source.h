#pragma once

#include "audio_source.h"
#include "sound_generators.h"
#include "lock_free_ring_buffer.h"
#include <atomic>
#include <vector>
#include <unordered_map>
#include <cstdint>

class CachedAudioSource : public AudioSource {
public:
  CachedAudioSource();
  ~CachedAudioSource() override = default;
  
  void generate_samples(float* left_buffer, float* right_buffer, int sample_count) override;
  
  // Trigger events from any thread - same interface as EventAudioSource
  void trigger_click(float amplitude = 0.5f, float jitter_ms = 0.0f, float pan = 0.0f);
  void trigger_beep(float amplitude = 0.3f, float jitter_ms = 0.0f, float pan = 0.0f);
  void trigger_explosion(float amplitude = 1.0f, float jitter_ms = 0.0f, float pan = 0.0f);
  
  // Get current sample position for external timing calculations
  uint64_t get_current_sample() const { return sample_position_.load(std::memory_order_relaxed); }

private:
  struct CachedSound {
    std::vector<float> samples;
    uint64_t duration_samples;
  };
  
  struct PlaybackInstance {
    uint64_t start_sample;
    SoundGenerators::SoundType type;
    float amplitude;
    float pan;  // -1.0 = left, 0.0 = center, 1.0 = right
    bool finished = false;
    
    bool should_play_at(uint64_t current_sample) const {
      return current_sample >= start_sample && !finished;
    }
  };
  
  void trigger_sound(SoundGenerators::SoundType type, float amplitude, float jitter_ms, float pan);
  void cache_sound(SoundGenerators::SoundType type);
  float get_cached_sample(SoundGenerators::SoundType type, uint64_t local_sample, float amplitude) const;
  
  // Pre-computed sound buffers
  std::unordered_map<SoundGenerators::SoundType, CachedSound> cached_sounds_;
  
  // Active playback instances
  LockFreeRingBuffer<PlaybackInstance, 1024> active_sounds_;
  std::atomic<uint64_t> sample_position_{0};
  
  static constexpr float SAMPLE_RATE = 44100.0f;
};