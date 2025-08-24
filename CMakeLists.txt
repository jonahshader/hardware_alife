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
    
    # Convert relative paths to absolute paths
    set(VHDL_ABS_FILES "")
    foreach(FILE ${ARG_VHDL_FILES})
        get_filename_component(ABS_FILE "${CMAKE_SOURCE_DIR}/hardware/vhdl/${FILE}" ABSOLUTE)
        list(APPEND VHDL_ABS_FILES ${ABS_FILE})
    endforeach()
    
    # Output paths
    set(VERILOG_FILE ${CMAKE_BINARY_DIR}/hardware/generated/verilog/${MODULE_NAME}.v)
    set(VERILATED_DIR ${CMAKE_BINARY_DIR}/hardware/verilated/obj_dir/${MODULE_NAME})
    set(VERILATED_MAKEFILE ${VERILATED_DIR}/V${ARG_TOP_ENTITY}.mk)
    set(VERILATED_LIB ${VERILATED_DIR}/V${ARG_TOP_ENTITY}__ALL.a)
    
    # Create directories
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/hardware/generated/verilog)
    file(MAKE_DIRECTORY ${VERILATED_DIR})
    
    # GHDL work directory
    set(GHDL_WORKDIR ${CMAKE_BINARY_DIR}/hardware/ghdl_work/${MODULE_NAME})
    file(MAKE_DIRECTORY ${GHDL_WORKDIR})
    
    # Step 1a: GHDL analyze phase
    add_custom_command(
        OUTPUT ${GHDL_WORKDIR}/.analyzed
        COMMAND ghdl -i --std=08 -fsynopsys --workdir=${GHDL_WORKDIR} ${ARG_VHDL_FILES}
        COMMAND ghdl -m --std=08 -fsynopsys --workdir=${GHDL_WORKDIR} ${ARG_TOP_ENTITY}
        COMMAND ${CMAKE_COMMAND} -E touch ${GHDL_WORKDIR}/.analyzed
        DEPENDS ${VHDL_ABS_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/hardware/vhdl
        COMMENT "Analyzing VHDL files for: ${MODULE_NAME}"
        VERBATIM
    )
    
    # Step 1b: GHDL synthesis to Verilog
    add_custom_command(
        OUTPUT ${VERILOG_FILE}
        COMMAND ghdl synth --std=08 -fsynopsys --workdir=${GHDL_WORKDIR} --out=verilog ${ARG_TOP_ENTITY} > ${VERILOG_FILE}
        DEPENDS ${GHDL_WORKDIR}/.analyzed
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/hardware/vhdl
        COMMENT "Synthesizing VHDL to Verilog: ${MODULE_NAME}"
        VERBATIM
    )
    
    # The Verilator step is now handled by verilate() function
    # Create a target for this module  
    add_custom_target(${MODULE_NAME}_verilog DEPENDS ${VERILOG_FILE})
    
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
