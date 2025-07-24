#include <cstdint>
#include <iostream>
#include <string>

#include "screens/demo.cuh"
#include "systems/game.cuh"

const std::string TITLE = "CMake SDL2 OpenGL CUDA";

int main(int argc, char *argv[]) {
  std::cout << TITLE << std::endl;
  // Determine if we are running headless
  bool headless = false;
  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]) == "--headless") {
      headless = true;
      break;
    }
  }
  std::cout << "Running " << (headless ? "headless" : "with graphics") << std::endl;

  if (!headless) {
    // Run with a window
    Game game(800, 600, false, TITLE);
    game.push(std::make_shared<DemoScreen>(game));
    game.run();
  }

  return 0;
}
