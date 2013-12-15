#include "globals.h"
#include "window.h"
#include "rng.h"

int main()
{
  srand(time(NULL));
  init_display();

  load_global_data();

  endwin();
  return 0;
}
