#include "cached_audio_source.h"
#include <algorithm>
#include <random>

CachedAudioSource::CachedAudioSource() {
  // Pre-cache all sound types at construction
  cache_sound(SoundGenerators::SoundType::CLICK);
  cache_sound(SoundGenerators::SoundType::BEEP);
  cache_sound(SoundGenerators::SoundType::EXPLOSION);
}

void CachedAudioSource::generate_samples(float *left_buffer, float *right_buffer,
                                         int sample_count) {
  // Record precise time at start of callback for interpolation
  last_callback_time_.store(std::chrono::high_resolution_clock::now(), std::memory_order_relaxed);
  
  uint64_t buffer_start = sample_position_.load(std::memory_order_relaxed);

  for (int i = 0; i < sample_count; ++i) {
    uint64_t current_sample = buffer_start + i;
    float mixed_left = 0.0f;
    float mixed_right = 0.0f;

    // Process all active sounds - similar to EventAudioSource logic
    std::vector<PlaybackInstance> continuing_sounds;

    PlaybackInstance sound;
    while (active_sounds_.try_pop(sound)) {
      if (sound.should_play_at(current_sample)) {
        uint64_t local_sample = current_sample - sound.start_sample;
        float sample = get_cached_sample(sound.type, local_sample, sound.amplitude);

        // Apply panning: -1.0 = left, 0.0 = center, 1.0 = right
        float left_gain =
            (1.0f - sound.pan) * 0.5f; // 1.0 when pan=-1, 0.5 when pan=0, 0.0 when pan=1
        float right_gain =
            (1.0f + sound.pan) * 0.5f; // 0.0 when pan=-1, 0.5 when pan=0, 1.0 when pan=1

        mixed_left += sample * left_gain;
        mixed_right += sample * right_gain;

        // Check if sound is finished
        const auto &cached = cached_sounds_.at(sound.type);
        if (local_sample >= cached.duration_samples) {
          sound.finished = true;
        }

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
    for (const auto &continuing_sound : continuing_sounds) {
      active_sounds_.try_push(continuing_sound);
    }

    // Apply mixed samples to channels
    left_buffer[i] += mixed_left;
    right_buffer[i] += mixed_right;
  }

  sample_position_.store(buffer_start + sample_count, std::memory_order_relaxed);
}

void CachedAudioSource::trigger_click(float amplitude, float jitter_ms, float pan) {
  trigger_sound(SoundGenerators::SoundType::CLICK, amplitude, jitter_ms, pan);
}

void CachedAudioSource::trigger_beep(float amplitude, float jitter_ms, float pan) {
  trigger_sound(SoundGenerators::SoundType::BEEP, amplitude, jitter_ms, pan);
}

void CachedAudioSource::trigger_explosion(float amplitude, float jitter_ms, float pan) {
  trigger_sound(SoundGenerators::SoundType::EXPLOSION, amplitude, jitter_ms, pan);
}

void CachedAudioSource::trigger_sound(SoundGenerators::SoundType type, float amplitude,
                                      float jitter_ms, float pan) {
  // Get current high-precision time
  auto now = std::chrono::high_resolution_clock::now();
  auto last_callback = last_callback_time_.load(std::memory_order_relaxed);
  
  // Calculate elapsed time since last audio callback (avoiding double)
  auto elapsed_nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now - last_callback);
  
  // Convert nanoseconds to samples: samples = nanos * sample_rate / 1e9
  // Rearrange to avoid floating point: samples = (nanos * sample_rate) / 1000000000
  uint64_t elapsed_samples = (elapsed_nanos.count() * static_cast<uint64_t>(SAMPLE_RATE)) / 1000000000ULL;
  
  // Interpolated current position
  uint64_t base_sample = sample_position_.load(std::memory_order_relaxed);
  uint64_t interpolated_sample = base_sample + elapsed_samples;

  // Apply jitter if specified
  uint64_t jitter_samples = 0;
  if (jitter_ms > 0.0f) {
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    std::uniform_real_distribution<float> jitter_dist(0.0f, jitter_ms * 0.001f * SAMPLE_RATE);
    jitter_samples = static_cast<uint64_t>(jitter_dist(gen));
  }

  PlaybackInstance sound;
  sound.start_sample = interpolated_sample + jitter_samples;
  sound.type = type;
  sound.amplitude = amplitude;
  sound.pan = pan;
  sound.finished = false;

  active_sounds_.try_push(sound);
}

void CachedAudioSource::cache_sound(SoundGenerators::SoundType type) {
  uint64_t duration = SoundGenerators::get_sound_duration_samples(type, SAMPLE_RATE);

  CachedSound cached;
  cached.duration_samples = duration;
  cached.samples.reserve(duration);

  // Set a fixed seed for consistent cached sound generation
  // This ensures the same click/beep/explosion sound every time
  std::mt19937 fixed_seed_gen(44); // Fixed seed for deterministic audio

  // Generate all samples for this sound type at unit amplitude using fixed seed
  for (uint64_t i = 0; i < duration; ++i) {
    float sample = SoundGenerators::generate_sample(type, i, 1.0f, SAMPLE_RATE, &fixed_seed_gen);
    cached.samples.push_back(sample);
  }

  cached_sounds_[type] = std::move(cached);
}

float CachedAudioSource::get_cached_sample(SoundGenerators::SoundType type, uint64_t local_sample,
                                           float amplitude) const {
  const auto it = cached_sounds_.find(type);
  if (it == cached_sounds_.end()) {
    return 0.0f; // Sound type not cached
  }

  const auto &cached = it->second;
  if (local_sample >= cached.samples.size()) {
    return 0.0f; // Past end of sound
  }

  return cached.samples[local_sample] * amplitude;
}
