#ifndef _GAME_H_
#define _GAME_H_

#include "map.h"
#include "overmap.h"
#include "cuss.h"

class Game
{
public:
  Game();
  ~Game();
  bool setup();
  Map* map;
  Overmap* overmap;

private:
  Window *w_map;
  cuss::interface i_main;
};

#endif
