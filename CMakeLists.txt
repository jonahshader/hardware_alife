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
  GIT_TAG release-3.2.20
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

# CNL (Compositional Numeric Library)
FetchContent_Declare(
  cnl
  GIT_REPOSITORY https://github.com/johnmcfarlane/cnl.git
  GIT_TAG main
)

# Disable CNL tests and benchmarks to speed up build
set(CNL_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(CNL_ENABLE_BENCHMARKS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(cnl)

# Eigen
FetchContent_Declare(
  Eigen3
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG 3.4.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Eigen3)

# GTest
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(googletest)

# Find OpenMP
find_package(OpenMP REQUIRED)

# VHDL to Verilator pipeline functions
function(add_vhdl_module MODULE_NAME)
    set(options "")
    set(oneValueArgs "TOP_ENTITY")
    set(multiValueArgs "VHDL_FILES")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    # Set top entity name (default to module name)
    if(NOT ARG_TOP_ENTITY)
        set(ARG_TOP_ENTITY ${MODULE_NAME})
    endif()
    
    # Output paths
    set(VERILOG_FILE ${CMAKE_BINARY_DIR}/hardware/generated/verilog/${MODULE_NAME}.v)
    set(GHDL_WORKDIR ${CMAKE_BINARY_DIR}/hardware/ghdl_work/${MODULE_NAME})
    
    # Create directories during configuration
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/hardware/generated/verilog)
    file(MAKE_DIRECTORY ${GHDL_WORKDIR})
    
    # Generate Verilog during configuration using execute_process
    message(STATUS "GHDL: Synthesizing ${MODULE_NAME} (VHDL → Verilog)")
    
    # Step 1: GHDL analyze phase
    execute_process(
        COMMAND ghdl -i --std=08 -fsynopsys --workdir=${GHDL_WORKDIR} ${ARG_VHDL_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/hardware/vhdl
        RESULT_VARIABLE GHDL_ANALYZE_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )
    
    if(NOT GHDL_ANALYZE_RESULT EQUAL 0)
        message(FATAL_ERROR "GHDL analyze failed for ${MODULE_NAME}")
    endif()
    
    # Step 2: GHDL elaborate phase
    execute_process(
        COMMAND ghdl -m --std=08 -fsynopsys --workdir=${GHDL_WORKDIR} ${ARG_TOP_ENTITY}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/hardware/vhdl
        RESULT_VARIABLE GHDL_ELABORATE_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )
    
    if(NOT GHDL_ELABORATE_RESULT EQUAL 0)
        message(FATAL_ERROR "GHDL elaborate failed for ${MODULE_NAME}")
    endif()
    
    # Step 3: GHDL synthesis to Verilog
    execute_process(
        COMMAND ghdl synth --std=08 -fsynopsys --workdir=${GHDL_WORKDIR} --out=verilog ${ARG_TOP_ENTITY}
        OUTPUT_FILE ${VERILOG_FILE}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/hardware/vhdl
        RESULT_VARIABLE GHDL_SYNTH_RESULT
        ERROR_QUIET
    )
    
    if(NOT GHDL_SYNTH_RESULT EQUAL 0)
        message(FATAL_ERROR "GHDL synthesis failed for ${MODULE_NAME}")
    endif()
    
    message(STATUS "GHDL: Generated ${VERILOG_FILE}")
    
endfunction()

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
