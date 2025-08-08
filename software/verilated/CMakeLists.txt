# Verilated software implementation
# TODO: Add Verilator integration and generated C++ sources

# Placeholder main file for verilated implementation
add_executable(software_verilated src/main.cpp)

# Link libraries
target_link_libraries(software_verilated PRIVATE
  SDL3::SDL3
  glad
  glm::glm
  OpenMP::OpenMP_CXX
)

# Include GLAD headers
target_include_directories(software_verilated PRIVATE ${glad_SOURCE_DIR}/include)

# TODO: Add Verilator-generated sources and includes
# target_include_directories(software_verilated PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/verilated_sources)
# target_sources(software_verilated PRIVATE ${VERILATOR_GENERATED_SOURCES})

# Platform-specific configurations
if(WIN32)
  target_link_libraries(software_verilated PRIVATE SDL3::SDL3-shared)
  if(MSVC)
    set_target_properties(software_verilated PROPERTIES
      LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
  endif()
endif()

# Copy SDL3 DLL to output directory on Windows
if(WIN32)
  add_custom_command(TARGET software_verilated POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3::SDL3-shared>
    $<TARGET_FILE_DIR:software_verilated>
  )
endif()