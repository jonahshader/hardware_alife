# Pure software implementation
add_executable(software_pure
  src/main.cpp
  src/audio/audio_command.h
  src/audio/audio_manager.cpp
  src/audio/audio_manager.h
  src/audio/audio_source.h
  src/audio/cached_audio_source.cpp
  src/audio/cached_audio_source.h
  src/audio/event_audio_source.cpp
  src/audio/event_audio_source.h
  src/audio/lock_free_ring_buffer.h
  src/audio/sound_generators.cpp
  src/audio/sound_generators.h
  src/screens/screen_context.h
  src/screens/screen.cpp
  src/screens/screen.h
  src/screens/snn_test.cpp
  src/screens/snn_test.h
  src/screens/test_screen.cpp
  src/screens/test_screen.h
  src/systems/game.cpp
  src/systems/game.h
  src/systems/fp/fp.h
  src/systems/fp/fp_core.h
  src/systems/fp/fp_math.h
  src/systems/fp/fp_io.h
  src/systems/fp/fp_literals.h
  src/systems/fp/fp_types.h
)

# Link libraries
target_link_libraries(software_pure PRIVATE
  SDL3::SDL3
  glad
  glm::glm
  OpenMP::OpenMP_CXX
)

# Include GLAD headers
target_include_directories(software_pure PRIVATE ${glad_SOURCE_DIR}/include)

# Include project source directories
target_include_directories(software_pure PRIVATE src)

# Platform-specific configurations
if(WIN32)
  target_link_libraries(software_pure PRIVATE SDL3::SDL3-shared)
  if(MSVC)
    # Add win_subsystem:console to keep console window for now
    set_target_properties(software_pure PROPERTIES
      LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
  endif()

  # Copy SDL3 DLL to output directory on Windows
  add_custom_command(TARGET software_pure POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3::SDL3-shared>
    $<TARGET_FILE_DIR:software_pure>
  )
endif()
