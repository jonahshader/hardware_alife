#pragma once

#include <cstdint>
#include <memory>

class AudioSource;

enum class AudioCommandType {
  ADD_SOURCE,
  REMOVE_SOURCE,
  SET_SOURCE_VOLUME,
  SET_MASTER_VOLUME,
  START_SOURCE,
  STOP_SOURCE,
  PAUSE_SOURCE,
  RESUME_SOURCE
};

struct AudioCommand {
  AudioCommandType type;
  AudioSource* source = nullptr;
  float volume = 1.0f;
  
  static AudioCommand add_source(AudioSource* src) {
    AudioCommand cmd;
    cmd.type = AudioCommandType::ADD_SOURCE;
    cmd.source = src;
    return cmd;
  }
  
  static AudioCommand remove_source(AudioSource* src) {
    AudioCommand cmd;
    cmd.type = AudioCommandType::REMOVE_SOURCE;
    cmd.source = src;
    return cmd;
  }
  
  static AudioCommand set_source_volume(AudioSource* src, float vol) {
    AudioCommand cmd;
    cmd.type = AudioCommandType::SET_SOURCE_VOLUME;
    cmd.source = src;
    cmd.volume = vol;
    return cmd;
  }
  
  static AudioCommand set_master_volume(float vol) {
    AudioCommand cmd;
    cmd.type = AudioCommandType::SET_MASTER_VOLUME;
    cmd.volume = vol;
    return cmd;
  }
  
  static AudioCommand start_source(AudioSource* src) {
    AudioCommand cmd;
    cmd.type = AudioCommandType::START_SOURCE;
    cmd.source = src;
    return cmd;
  }
  
  static AudioCommand stop_source(AudioSource* src) {
    AudioCommand cmd;
    cmd.type = AudioCommandType::STOP_SOURCE;
    cmd.source = src;
    return cmd;
  }
  
  static AudioCommand pause_source(AudioSource* src) {
    AudioCommand cmd;
    cmd.type = AudioCommandType::PAUSE_SOURCE;
    cmd.source = src;
    return cmd;
  }
  
  static AudioCommand resume_source(AudioSource* src) {
    AudioCommand cmd;
    cmd.type = AudioCommandType::RESUME_SOURCE;
    cmd.source = src;
    return cmd;
  }
};