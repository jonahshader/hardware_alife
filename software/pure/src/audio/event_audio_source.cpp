#include "event_audio_source.h"
#include <cmath>
#include <numbers>
#include <random>
#include <algorithm>

EventAudioSource::EventAudioSource() = default;

void EventAudioSource::generate_samples(float* left_buffer, float* right_buffer, int sample_count) {
  uint64_t buffer_start = sample_position_.load(std::memory_order_relaxed);
  
  for (int i = 0; i < sample_count; ++i) {
    uint64_t current_sample = buffer_start + i;
    float mixed_sample = 0.0f;
    
    // Process all active sounds - we need to iterate through entire queue
    // to handle overlapping sounds and out-of-order jittered events
    std::vector<SoundInstance> continuing_sounds;
    
    SoundInstance sound;
    while (active_sounds_.try_pop(sound)) {
      if (sound.should_play_at(current_sample)) {
        mixed_sample += sound.get_sample_value(current_sample, SAMPLE_RATE);
        
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
    
    // Apply to both channels (mono for now)
    left_buffer[i] += mixed_sample;
    right_buffer[i] += mixed_sample;
  }
  
  sample_position_.store(buffer_start + sample_count, std::memory_order_relaxed);
}

void EventAudioSource::trigger_click(float amplitude, float jitter_ms) {
  trigger_event(EventType::CLICK, amplitude, jitter_ms);
}

void EventAudioSource::trigger_beep(float amplitude, float jitter_ms) {
  trigger_event(EventType::BEEP, amplitude, jitter_ms);
}

void EventAudioSource::trigger_explosion(float amplitude, float jitter_ms) {
  trigger_event(EventType::EXPLOSION, amplitude, jitter_ms);
}

void EventAudioSource::trigger_event(EventType type, float amplitude, float jitter_ms) {
  static thread_local std::random_device rd;
  static thread_local std::mt19937 gen(rd());
  static thread_local std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
  
  SoundInstance sound;
  sound.type = type;
  sound.amplitude = amplitude;
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
  
  switch (type) {
    case EventType::CLICK:
      return EventAudioSource::generate_click_sample(local_sample, amplitude, sample_rate);
    case EventType::BEEP:
      return EventAudioSource::generate_beep_sample(local_sample, amplitude, sample_rate);
    case EventType::EXPLOSION:
      return EventAudioSource::generate_explosion_sample(local_sample, amplitude, sample_rate);
  }
  return 0.0f;
}

float EventAudioSource::generate_click_sample(uint64_t local_sample, float amplitude, float sample_rate) {
  // Simple click: short burst of noise with exponential decay
  const float click_duration_ms = 10.0f;  // 10ms click
  const float click_duration_samples = click_duration_ms * 0.001f * sample_rate;
  
  if (local_sample >= click_duration_samples) {
    return 0.0f;  // Click finished
  }
  
  // Generate noise component
  static thread_local std::random_device rd;
  static thread_local std::mt19937 gen(rd());
  static thread_local std::uniform_real_distribution<float> noise_dist(-1.0f, 1.0f);
  
  float noise = noise_dist(gen);
  
  // Apply exponential decay envelope
  float t = static_cast<float>(local_sample) / click_duration_samples;
  float envelope = std::exp(-t * 8.0f);  // Fast decay
  
  return noise * envelope * amplitude;
}

float EventAudioSource::generate_beep_sample(uint64_t local_sample, float amplitude, float sample_rate) {
  // Simple beep: sine wave with envelope
  const float beep_duration_ms = 100.0f;  // 100ms beep
  const float beep_duration_samples = beep_duration_ms * 0.001f * sample_rate;
  const float frequency = 800.0f;  // 800Hz tone
  
  if (local_sample >= beep_duration_samples) {
    return 0.0f;  // Beep finished
  }
  
  float t = static_cast<float>(local_sample) / sample_rate;
  float sine = std::sin(2.0f * std::numbers::pi_v<float> * frequency * t);
  
  // Apply triangular envelope (fade in and out)
  float env_t = static_cast<float>(local_sample) / beep_duration_samples;
  float envelope;
  if (env_t < 0.1f) {
    envelope = env_t / 0.1f;  // Fade in
  } else if (env_t > 0.9f) {
    envelope = (1.0f - env_t) / 0.1f;  // Fade out
  } else {
    envelope = 1.0f;  // Sustain
  }
  
  return sine * envelope * amplitude;
}

float EventAudioSource::generate_explosion_sample(uint64_t local_sample, float amplitude, float sample_rate) {
  // Explosion: low-frequency rumble with long decay
  const float explosion_duration_ms = 500.0f;  // 500ms explosion
  const float explosion_duration_samples = explosion_duration_ms * 0.001f * sample_rate;
  
  if (local_sample >= explosion_duration_samples) {
    return 0.0f;  // Explosion finished
  }
  
  static thread_local std::random_device rd;
  static thread_local std::mt19937 gen(rd());
  static thread_local std::uniform_real_distribution<float> noise_dist(-1.0f, 1.0f);
  
  // Mix of low-frequency sine and noise
  float t = static_cast<float>(local_sample) / sample_rate;
  float low_freq = std::sin(2.0f * std::numbers::pi_v<float> * 60.0f * t);  // 60Hz rumble
  float noise = noise_dist(gen);
  float mixed = 0.7f * low_freq + 0.3f * noise;
  
  // Long exponential decay
  float env_t = static_cast<float>(local_sample) / explosion_duration_samples;
  float envelope = std::exp(-env_t * 3.0f);
  
  return mixed * envelope * amplitude;
}