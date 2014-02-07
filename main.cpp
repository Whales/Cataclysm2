#include "globals.h"
#include "window.h"
#include "rng.h"
#include "game.h"
#include "files.h"
#include <ctime>
#include <stdio.h>  // Output errors on bad command line options
#include <getopt.h>
#include <unistd.h> // Required for getopt??

bool parse_options(int argc, char* argv[]);

int main(int argc, char* argv[])
{
  if (!parse_options(argc, argv)) {
    return 1;
  }

  srand(time(NULL));  // Seed the RNG - TODO: Wrap this up
  init_display();     // See window.cpp

  set_default_dirs(); // See files.cpp

  load_global_data(); // See globals.cpp

  GAME.setup();       // See game.cpp

  do {} while (GAME.main_loop()); // See game.cpp

  endwin();           // See window.cpp
  return 0;
}

bool parse_options(int argc, char* argv[])
{
// extern variables used by getopt
  extern char* optarg;
  std::string argument;
  int c;
// TODO: Move these long options (and this function?) to a different file
  static option long_options[] =
  {
    { "data-dir", required_argument, 0, 'd' },
    { "cuss-dir", required_argument, 0, 'c' },
    { 0, 0, 0, 0 }
  };
  int opt_index;

  while ( (c = getopt_long(argc, argv, "", long_options, &opt_index)) != -1) {

    switch (c) {

      case 'd':
        if (!set_dir(DATA_DIR, optarg)) {
          printf("'%s' is not a valid data directory.\n", optarg);
          return false;
        }
        break;

      case 'c':
        if (!set_dir(CUSS_DIR, optarg)) {
          printf("'%s' is not a valid cuss directory.\n", optarg);
          return false;
        }
        break;

      case '?':
// getopt_long handles printing these error messages.
        return false;

    }

  }

  return true;
}
