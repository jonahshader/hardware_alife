cmake_minimum_required(VERSION 3.18)
project(prog CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
  GIT_REPOSITORY https://github.com/jonahshader/glad_gles2.0.git
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

# Eigen (header-only library)
FetchContent_Declare(
  eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG 3.4.0
)
FetchContent_GetProperties(eigen)
if(NOT eigen_POPULATED)
  FetchContent_Populate(eigen)
  add_library(eigen INTERFACE)
  target_include_directories(eigen INTERFACE ${eigen_SOURCE_DIR})
endif()

# Artery Font (header-only library)
FetchContent_Declare(
        artery_font
        GIT_REPOSITORY https://github.com/Chlumsky/artery-font-format.git
        GIT_TAG master
)
FetchContent_GetProperties(artery_font)
if(NOT artery_font_POPULATED)
    FetchContent_Populate(artery_font)
    add_library(artery_font INTERFACE)
    target_include_directories(artery_font INTERFACE ${artery_font_SOURCE_DIR}/artery-font)
endif()

# lodepng
FetchContent_Declare(
  lodepng
  GIT_REPOSITORY https://github.com/lvandeve/lodepng.git
  GIT_TAG master
)
FetchContent_GetProperties(lodepng)
if(NOT lodepng_POPULATED)
  FetchContent_Populate(lodepng)
endif()

# # GoogleTest
# FetchContent_Declare(
#   googletest
#   GIT_REPOSITORY https://github.com/google/googletest.git
#   GIT_TAG v1.14.0
# )
# # For Windows: Prevent overriding the parent project's compiler/linker settings
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)

# Find OpenMP
find_package(OpenMP REQUIRED)

# Create executable
add_executable(prog
  src/main.cpp
  ${lodepng_SOURCE_DIR}/lodepng.cpp
  src/graphics/renderers/base_renderer.h
  src/graphics/renderers/circle.cpp
  src/graphics/renderers/circle.h
  src/graphics/renderers/font.cpp
  src/graphics/renderers/font.h
  src/graphics/renderers/line.cpp
  src/graphics/renderers/line.h
  src/graphics/renderers/rect.cpp
  src/graphics/renderers/rect.h
  src/graphics/renderers/simple_rect.cpp
  src/graphics/renderers/simple_rect.h
  src/graphics/viewports/extend.cpp
  src/graphics/viewports/extend.h
  src/graphics/shader.cpp
  src/graphics/shader.h
  src/screens/default.cpp
  src/screens/default.h
  src/screens/demo.cpp
  src/screens/demo.h
  src/screens/screen.h
  src/systems/game.cpp
  src/systems/game.h
  src/systems/resources.h
)


# Link libraries
target_link_libraries(prog PRIVATE
  SDL2::SDL2
  glad
  glm::glm
  eigen
  artery_font
  OpenMP::OpenMP_CXX
)

# Include headers
target_include_directories(prog PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${glad_SOURCE_DIR}/include
  ${lodepng_SOURCE_DIR}
)

# # Add test executable
# add_executable(matrix_tests
#   tests/eigen_cublas_test.cu
#   src/systems/eigen_impl.cpp
# )

# # Set CUDA properties for tests
# set_target_properties(matrix_tests PROPERTIES
#   CUDA_SEPARABLE_COMPILATION ON
# )

# # Set source file properties to compile eigen_impl.cpp as C++, not CUDA
# set_source_files_properties(src/systems/eigen_impl.cpp PROPERTIES
#   LANGUAGE CXX
# )

# # Link test libraries
# target_link_libraries(matrix_tests PRIVATE
#   eigen
#   curand
#   cublas
#   cublasLt
#   GTest::gtest_main
#   GTest::gtest
# )

# target_include_directories(matrix_tests PRIVATE
#   ${CMAKE_CURRENT_SOURCE_DIR}/src
#   ${CMAKE_CURRENT_SOURCE_DIR}/tests
# )

# # Enable testing
# enable_testing()
# add_test(NAME MatrixTests COMMAND matrix_tests)


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

    # Use the MultiThreaded DLL runtime library for both Debug and Release
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
  endif()

  # Copy SDL2 DLL to output directory on Windows
  add_custom_command(TARGET prog POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL2::SDL2>
    $<TARGET_FILE_DIR:prog>
  )
endif()
