cmake_minimum_required(VERSION 3.18)
project(cmake_sdl2_opengl CUDA CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 20)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# Include FetchContent for downloading dependencies
include(FetchContent)

# SDL2
FetchContent_Declare(
  SDL2
  URL https://github.com/libsdl-org/SDL/releases/download/release-2.30.3/SDL2-2.30.3.tar.gz
  URL_HASH SHA256=820440072f8f5b50188c1dae104f2ad25984de268785be40c41a099a510f0aec
)
FetchContent_MakeAvailable(SDL2)

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
add_executable(prog 
  src/main.cu
  src/cuda_test.cu
  src/cuda_test.cuh
)

# Set CUDA properties
set_target_properties(prog PROPERTIES 
  CUDA_SEPARABLE_COMPILATION ON
)

# Link libraries
target_link_libraries(prog PRIVATE
  SDL2::SDL2
  glad
  glm::glm
  OpenMP::OpenMP_CXX
)

# Include GLAD headers
target_include_directories(prog PRIVATE ${glad_SOURCE_DIR}/include)

# Platform-specific configurations
if(WIN32)
  target_link_libraries(prog PRIVATE SDL2::SDL2main)
  if(MSVC)
    # Add win_subsystem:windows to remove console window
    set_target_properties(prog PROPERTIES
      LINK_FLAGS "/SUBSYSTEM:WINDOWS"
      LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"
      LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE"
    )
  endif()
endif()

# Copy SDL2 DLL to output directory on Windows
if(WIN32)
  add_custom_command(TARGET prog POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL2::SDL2>
    $<TARGET_FILE_DIR:prog>
  )
endif()