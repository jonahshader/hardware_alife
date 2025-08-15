#include "event_audio_source.h"
#include <random>
#include <algorithm>

EventAudioSource::EventAudioSource() = default;

void EventAudioSource::generate_samples(float* left_buffer, float* right_buffer, int sample_count) {
  uint64_t buffer_start = sample_position_.load(std::memory_order_relaxed);
  
  for (int i = 0; i < sample_count; ++i) {
    uint64_t current_sample = buffer_start + i;
    float mixed_left = 0.0f;
    float mixed_right = 0.0f;
    
    // Process all active sounds - we need to iterate through entire queue
    // to handle overlapping sounds and out-of-order jittered events
    std::vector<SoundInstance> continuing_sounds;
    
    SoundInstance sound;
    while (active_sounds_.try_pop(sound)) {
      if (sound.should_play_at(current_sample)) {
        float sample = sound.get_sample_value(current_sample, SAMPLE_RATE);
        
        // Apply panning: -1.0 = left, 0.0 = center, 1.0 = right
        float left_gain = (1.0f - sound.pan) * 0.5f;   // 1.0 when pan=-1, 0.5 when pan=0, 0.0 when pan=1
        float right_gain = (1.0f + sound.pan) * 0.5f;  // 0.0 when pan=-1, 0.5 when pan=0, 1.0 when pan=1
        
        mixed_left += sample * left_gain;
        mixed_right += sample * right_gain;
        
        // Keep sound active if not finished
        if (!sound.finished) {
          continuing_sounds.push_back(sound);
        }
      } else if (current_sample < sound.start_sample) {
        // Future sound, put back for later
        continuing_sounds.push_back(sound);
      }
      // Else: past sound that's finished, discard
    }
    
    // Put continuing sounds back in queue
    for (const auto& continuing_sound : continuing_sounds) {
      active_sounds_.try_push(continuing_sound);
    }
    
    // Apply mixed samples to channels
    left_buffer[i] += mixed_left;
    right_buffer[i] += mixed_right;
  }
  
  sample_position_.store(buffer_start + sample_count, std::memory_order_relaxed);
}

void EventAudioSource::trigger_click(float amplitude, float jitter_ms, float pan) {
  trigger_event(EventType::CLICK, amplitude, jitter_ms, pan);
}

void EventAudioSource::trigger_beep(float amplitude, float jitter_ms, float pan) {
  trigger_event(EventType::BEEP, amplitude, jitter_ms, pan);
}

void EventAudioSource::trigger_explosion(float amplitude, float jitter_ms, float pan) {
  trigger_event(EventType::EXPLOSION, amplitude, jitter_ms, pan);
}

void EventAudioSource::trigger_event(EventType type, float amplitude, float jitter_ms, float pan) {
  static thread_local std::random_device rd;
  static thread_local std::mt19937 gen(rd());
  static thread_local std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
  
  SoundInstance sound;
  sound.type = type;
  sound.amplitude = amplitude;
  sound.pan = pan;
  sound.finished = false;
  
  // Apply jitter: convert ms to samples and add random offset
  double jitter_samples = jitter_ms * 0.001 * SAMPLE_RATE * dist(gen);
  uint64_t current_pos = sample_position_.load(std::memory_order_relaxed);
  
  // Ensure we don't trigger in the past
  int64_t target_sample = current_pos + static_cast<int64_t>(jitter_samples);
  sound.start_sample = std::max(target_sample, static_cast<int64_t>(current_pos));
  
  active_sounds_.try_push(sound);
}

float EventAudioSource::SoundInstance::get_sample_value(uint64_t current_sample, float sample_rate) {
  if (!should_play_at(current_sample)) return 0.0f;
  
  uint64_t local_sample = current_sample - start_sample;
  SoundGenerators::SoundType sound_type = EventAudioSource::event_to_sound_type(type);
  
  return SoundGenerators::generate_sample(sound_type, local_sample, amplitude, sample_rate);
}

SoundGenerators::SoundType EventAudioSource::event_to_sound_type(EventType type) {
  switch (type) {
    case EventType::CLICK:
      return SoundGenerators::SoundType::CLICK;
    case EventType::BEEP:
      return SoundGenerators::SoundType::BEEP;
    case EventType::EXPLOSION:
      return SoundGenerators::SoundType::EXPLOSION;
  }
  return SoundGenerators::SoundType::CLICK;  // Default fallback
}