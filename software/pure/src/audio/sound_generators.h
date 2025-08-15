#pragma once

#include <cstdint>

namespace SoundGenerators {

enum class SoundType {
  CLICK,
  BEEP,
  EXPLOSION
};

// Generate a single sample for the given sound type
// local_sample: sample offset from start of sound (0-based)
// amplitude: volume multiplier (0.0 to 1.0)
// sample_rate: samples per second (typically 44100)
// Returns: sample value (-1.0 to 1.0), or 0.0 if sound is finished
float generate_click_sample(uint64_t local_sample, float amplitude, float sample_rate);
float generate_beep_sample(uint64_t local_sample, float amplitude, float sample_rate);
float generate_explosion_sample(uint64_t local_sample, float amplitude, float sample_rate);

// Get the duration of a sound type in samples
uint64_t get_sound_duration_samples(SoundType type, float sample_rate);

// Convenience function to generate a sample for any sound type
float generate_sample(SoundType type, uint64_t local_sample, float amplitude, float sample_rate);

}  // namespace SoundGenerators