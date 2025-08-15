#include "sound_generators.h"
#include <cmath>
#include <numbers>
#include <random>

namespace SoundGenerators {

float generate_click_sample(uint64_t local_sample, float amplitude, float sample_rate, std::mt19937* rng) {
  // Simple click: short burst of noise with exponential decay
  const float click_duration_ms = 10.0f;  // 10ms click
  const float click_duration_samples = click_duration_ms * 0.001f * sample_rate;
  
  if (local_sample >= click_duration_samples) {
    return 0.0f;  // Click finished
  }
  
  // Generate noise component
  float noise;
  if (rng) {
    // Use provided generator (for fixed seed)
    std::uniform_real_distribution<float> noise_dist(-1.0f, 1.0f);
    noise = noise_dist(*rng);
  } else {
    // Use thread_local generator (for random)
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    static thread_local std::uniform_real_distribution<float> noise_dist(-1.0f, 1.0f);
    noise = noise_dist(gen);
  }
  
  // Apply exponential decay envelope
  float t = static_cast<float>(local_sample) / click_duration_samples;
  float envelope = std::exp(-t * 8.0f);  // Fast decay
  
  return noise * envelope * amplitude;
}

float generate_beep_sample(uint64_t local_sample, float amplitude, float sample_rate, std::mt19937* rng) {
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

float generate_explosion_sample(uint64_t local_sample, float amplitude, float sample_rate, std::mt19937* rng) {
  // Explosion: low-frequency rumble with long decay
  const float explosion_duration_ms = 500.0f;  // 500ms explosion
  const float explosion_duration_samples = explosion_duration_ms * 0.001f * sample_rate;
  
  if (local_sample >= explosion_duration_samples) {
    return 0.0f;  // Explosion finished
  }
  
  // Mix of low-frequency sine and noise
  float t = static_cast<float>(local_sample) / sample_rate;
  float low_freq = std::sin(2.0f * std::numbers::pi_v<float> * 60.0f * t);  // 60Hz rumble
  
  float noise;
  if (rng) {
    // Use provided generator (for fixed seed)
    std::uniform_real_distribution<float> noise_dist(-1.0f, 1.0f);
    noise = noise_dist(*rng);
  } else {
    // Use thread_local generator (for random)
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    static thread_local std::uniform_real_distribution<float> noise_dist(-1.0f, 1.0f);
    noise = noise_dist(gen);
  }
  float mixed = 0.7f * low_freq + 0.3f * noise;
  
  // Long exponential decay
  float env_t = static_cast<float>(local_sample) / explosion_duration_samples;
  float envelope = std::exp(-env_t * 3.0f);
  
  return mixed * envelope * amplitude;
}

uint64_t get_sound_duration_samples(SoundType type, float sample_rate) {
  switch (type) {
    case SoundType::CLICK:
      return static_cast<uint64_t>(10.0f * 0.001f * sample_rate);  // 10ms
    case SoundType::BEEP:
      return static_cast<uint64_t>(100.0f * 0.001f * sample_rate);  // 100ms
    case SoundType::EXPLOSION:
      return static_cast<uint64_t>(500.0f * 0.001f * sample_rate);  // 500ms
  }
  return 0;
}

float generate_sample(SoundType type, uint64_t local_sample, float amplitude, float sample_rate, std::mt19937* rng) {
  switch (type) {
    case SoundType::CLICK:
      return generate_click_sample(local_sample, amplitude, sample_rate, rng);
    case SoundType::BEEP:
      return generate_beep_sample(local_sample, amplitude, sample_rate, rng);
    case SoundType::EXPLOSION:
      return generate_explosion_sample(local_sample, amplitude, sample_rate, rng);
  }
  return 0.0f;
}

}  // namespace SoundGenerators