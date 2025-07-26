cmake_minimum_required(VERSION 3.14)
project(cmake_sdl3_opengl CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include FetchContent for downloading dependencies
include(FetchContent)

# SDL3
FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG main
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL3)

# GLAD
FetchContent_Declare(
  glad
  GIT_REPOSITORY https://github.com/jonahshader/glad_opengl46.git
  GIT_TAG main
)
FetchContent_GetProperties(glad)
if(NOT glad_POPULATED)
  FetchContent_Populate(glad)
  add_subdirectory(${glad_SOURCE_DIR} ${glad_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# GLM
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

# Find OpenMP
find_package(OpenMP REQUIRED)

# Create executable
add_executable(prog src/main.cpp)

# Link libraries
target_link_libraries(prog PRIVATE
  SDL3::SDL3
  glad
  glm::glm
  OpenMP::OpenMP_CXX
)

# Include GLAD headers
target_include_directories(prog PRIVATE ${glad_SOURCE_DIR}/include)

# Platform-specific configurations
if(WIN32)
  target_link_libraries(prog PRIVATE SDL3::SDL3-shared)
  if(MSVC)
    # Add win_subsystem:console to keep console window for now
    set_target_properties(prog PROPERTIES
      LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
  endif()
endif()

# Copy SDL3 DLL to output directory on Windows
if(WIN32)
  add_custom_command(TARGET prog POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3::SDL3-shared>
    $<TARGET_FILE_DIR:prog>
  )
endif()