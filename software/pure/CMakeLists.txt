# Pure software implementation
add_executable(software_pure src/main.cpp)

# Link libraries
target_link_libraries(software_pure PRIVATE
  SDL3::SDL3
  glad
  glm::glm
  OpenMP::OpenMP_CXX
)

# Include GLAD headers
target_include_directories(software_pure PRIVATE ${glad_SOURCE_DIR}/include)

# Platform-specific configurations
if(WIN32)
  target_link_libraries(software_pure PRIVATE SDL3::SDL3-shared)
  if(MSVC)
    # Add win_subsystem:console to keep console window for now
    set_target_properties(software_pure PROPERTIES
      LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
  endif()
endif()

# Copy SDL3 DLL to output directory on Windows
if(WIN32)
  add_custom_command(TARGET software_pure POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3::SDL3-shared>
    $<TARGET_FILE_DIR:software_pure>
  )
endif()