cmake_minimum_required(VERSION 3.14)
project(hardware_alife CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_SOFTWARE_PURE "Build pure software implementation" ON)
option(BUILD_SOFTWARE_VERILATED "Build verilated software implementation" OFF)

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
  GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# Find OpenMP
find_package(OpenMP REQUIRED)

# Add subdirectories based on build options
if(BUILD_SOFTWARE_PURE)
    add_subdirectory(software/pure)
endif()

if(BUILD_SOFTWARE_VERILATED)
    add_subdirectory(software/verilated)
endif()

# Always build common directory if it exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/common/CMakeLists.txt")
    add_subdirectory(common)
endif()