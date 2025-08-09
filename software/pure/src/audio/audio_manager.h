#pragma once

#include "audio_source.h"
#include "audio_command.h"
#include "lock_free_ring_buffer.h"

#include <SDL3/SDL.h>
#include <vector>
#include <memory>
#include <atomic>
#include <algorithm>

class AudioManager {
public:
  static AudioManager& instance();
  
  ~AudioManager();
  
  bool initialize();
  void shutdown();
  
  void add_source(AudioSource* source);
  void remove_source(AudioSource* source);
  void set_source_volume(AudioSource* source, float volume);
  void set_master_volume(float volume);
  void start_source(AudioSource* source);
  void stop_source(AudioSource* source);
  void pause_source(AudioSource* source);
  void resume_source(AudioSource* source);

private:
  AudioManager() = default;
  AudioManager(const AudioManager&) = delete;
  AudioManager& operator=(const AudioManager&) = delete;
  
  static void audio_callback(void* userdata, SDL_AudioStream* stream, 
                           int additional_amount, int total_amount);
  
  void process_commands();
  void mix_audio(float* left_buffer, float* right_buffer, int sample_count);
  
  SDL_AudioStream* audio_stream_{nullptr};
  LockFreeRingBuffer<AudioCommand, 512> command_queue_;
  
  std::vector<AudioSource*> active_sources_;
  std::atomic<float> master_volume_{0.5f};
  
  bool initialized_{false};
};