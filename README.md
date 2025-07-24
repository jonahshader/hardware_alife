# cmake_sdl2_opengl

Minimal CMake project with SDL2, OpenGL, GLAD, CUDA, and some other stuff.

## Prerequisites

- CMake
- C++ Compiler
- CUDA Toolkit

## Build

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Run

Working directory must be the `resources` directory. In Windows this looks like:

```bash
cd resources
..\build\Debug\prog.exe
```

Windows: Might be necessary to run the above from the Visual Studio Developer Command Prompt.
