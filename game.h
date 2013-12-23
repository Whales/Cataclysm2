#ifndef _GAME_H_
#define _GAME_H_

#include "map.h"
#include "cuss.h"
#include "entity.h"
#include "worldmap.h"

class Game
{
public:
  Game();
  ~Game();

  bool setup();
  bool main_loop();

  void player_move(int xdif, int ydif); // Handles all aspects of moving player

  Map*      map;
  Worldmap* worldmap;
  Player*   player;
  Monster_pool monsters;

private:
  Window *w_map;
  cuss::interface i_main;
};

#endif
