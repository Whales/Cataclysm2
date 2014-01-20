#include "globals.h"
#include "window.h"
#include "rng.h"
#include "game.h"
#include <ctime>

int main()
{
  srand(time(NULL));
  init_display();

  load_global_data();

  GAME.setup();

  do {} while (GAME.main_loop());

  endwin();
  return 0;
}
