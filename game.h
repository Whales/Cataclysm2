#ifndef _GAME_H_
#define _GAME_H_

#include "map.h"
#include "cuss.h"
#include "player.h"
#include "worldmap.h"

class Game
{
public:
  Game();
  ~Game();

  bool setup();
  bool main_loop();

  Map*      map;
  Worldmap* worldmap;
  Player*   player;
  

private:
  Window *w_map;
  cuss::interface i_main;
};

#endif
