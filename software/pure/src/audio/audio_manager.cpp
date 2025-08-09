#include "audio_manager.h"

#include <algorithm>
#include <cstring>
#include <vector>

AudioManager& AudioManager::instance() {
  static AudioManager instance;
  return instance;
}

AudioManager::~AudioManager() {
  shutdown();
}

bool AudioManager::initialize() {
  if (initialized_) {
    return true;
  }

  SDL_AudioSpec spec;
  spec.channels = 2;
  spec.format = SDL_AUDIO_F32;
  spec.freq = 44100;

  audio_stream_ = SDL_OpenAudioDeviceStream(
    SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, audio_callback, this);

  if (!audio_stream_) {
    SDL_Log("Failed to open audio device: %s", SDL_GetError());
    return false;
  }

  SDL_ResumeAudioStreamDevice(audio_stream_);
  initialized_ = true;
  return true;
}

void AudioManager::shutdown() {
  if (!initialized_) {
    return;
  }

  if (audio_stream_) {
    SDL_DestroyAudioStream(audio_stream_);
    audio_stream_ = nullptr;
  }

  active_sources_.clear();
  initialized_ = false;
}

void AudioManager::add_source(AudioSource* source) {
  if (!source) return;
  command_queue_.try_push(AudioCommand::add_source(source));
}

void AudioManager::remove_source(AudioSource* source) {
  if (!source) return;
  command_queue_.try_push(AudioCommand::remove_source(source));
}

void AudioManager::set_source_volume(AudioSource* source, float volume) {
  if (!source) return;
  command_queue_.try_push(AudioCommand::set_source_volume(source, volume));
}

void AudioManager::set_master_volume(float volume) {
  command_queue_.try_push(AudioCommand::set_master_volume(volume));
}

void AudioManager::start_source(AudioSource* source) {
  if (!source) return;
  command_queue_.try_push(AudioCommand::start_source(source));
}

void AudioManager::stop_source(AudioSource* source) {
  if (!source) return;
  command_queue_.try_push(AudioCommand::stop_source(source));
}

void AudioManager::pause_source(AudioSource* source) {
  if (!source) return;
  command_queue_.try_push(AudioCommand::pause_source(source));
}

void AudioManager::resume_source(AudioSource* source) {
  if (!source) return;
  command_queue_.try_push(AudioCommand::resume_source(source));
}

void AudioManager::audio_callback(void* userdata, SDL_AudioStream* stream,
                                 int additional_amount, int total_amount) {
  AudioManager* manager = static_cast<AudioManager*>(userdata);

  const int sample_count = additional_amount / (sizeof(float) * 2);

  std::vector<float> left_buffer(sample_count, 0.0f);
  std::vector<float> right_buffer(sample_count, 0.0f);

  manager->process_commands();
  manager->mix_audio(left_buffer.data(), right_buffer.data(), sample_count);

  std::vector<float> interleaved(sample_count * 2);
  for (int i = 0; i < sample_count; ++i) {
    interleaved[i * 2] = left_buffer[i];
    interleaved[i * 2 + 1] = right_buffer[i];
  }

  SDL_PutAudioStreamData(stream, interleaved.data(), additional_amount);
}

void AudioManager::process_commands() {
  AudioCommand command;
  while (command_queue_.try_pop(command)) {
    switch (command.type) {
      case AudioCommandType::ADD_SOURCE: {
        auto it = std::find(active_sources_.begin(), active_sources_.end(),
                           command.source);
        if (it == active_sources_.end()) {
          active_sources_.push_back(command.source);
        }
        break;
      }

      case AudioCommandType::REMOVE_SOURCE: {
        auto it = std::find(active_sources_.begin(), active_sources_.end(),
                           command.source);
        if (it != active_sources_.end()) {
          active_sources_.erase(it);
        }
        break;
      }

      case AudioCommandType::SET_SOURCE_VOLUME:
        command.source->set_volume(command.volume);
        break;

      case AudioCommandType::SET_MASTER_VOLUME:
        master_volume_.store(command.volume, std::memory_order_relaxed);
        break;

      case AudioCommandType::START_SOURCE:
        command.source->start();
        break;

      case AudioCommandType::STOP_SOURCE:
        command.source->stop();
        break;

      case AudioCommandType::PAUSE_SOURCE:
        command.source->pause();
        break;

      case AudioCommandType::RESUME_SOURCE:
        command.source->resume();
        break;
    }
  }
}

void AudioManager::mix_audio(float* left_buffer, float* right_buffer, int sample_count) {
  const float master_vol = master_volume_.load(std::memory_order_relaxed);

  for (AudioSource* source : active_sources_) {
    if (!source || !source->is_active()) {
      continue;
    }

    std::vector<float> source_left(sample_count);
    std::vector<float> source_right(sample_count);

    source->generate_samples(source_left.data(), source_right.data(), sample_count);

    const float source_volume = source->get_volume();
    const float final_volume = master_vol * source_volume;

    for (int i = 0; i < sample_count; ++i) {
      left_buffer[i] += source_left[i] * final_volume;
      right_buffer[i] += source_right[i] * final_volume;
    }
  }

  for (int i = 0; i < sample_count; ++i) {
    left_buffer[i] = std::clamp(left_buffer[i], -1.0f, 1.0f);
    right_buffer[i] = std::clamp(right_buffer[i], -1.0f, 1.0f);
  }
}
