#include "screens/snn_test.h"
#include "screens/test_screen.h"
#include "systems/game.h"
#include <memory>

int main(int argc, char *argv[]) {
  // Create game first
  Game game(128, 128, "Hardware ALife Test");

  // Create and add test screen
  auto test_screen = std::make_unique<SNNTestScreen>(game);
  game.push_screen(std::move(test_screen));
  game.set_vsync(false);
  // game.set_framerate_limit(600.0f);

  // Run the game
  game.run();

  return 0;
}
