#include "globals.h"
#include "window.h"
#include "rng.h"
#include "game.h"
#include "files.h"
#include <ctime>

int main()
{
  srand(time(NULL));  // Seed the RNG - TODO: Wrap this up
  init_display();     // See window.cpp

  set_default_dirs(); // See files.cpp

  load_global_data(); // See globals.cpp

  GAME.setup();       // See game.cpp

  do {} while (GAME.main_loop()); // See game.cpp

  endwin();           // See window.cpp
  return 0;
}
