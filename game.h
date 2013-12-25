#ifndef _GAME_H_
#define _GAME_H_

#include "map.h"
#include "cuss.h"
#include "worldmap.h"
#include "monster.h"
#include "player.h"
#include "keybind.h"

struct Game_message
{
// TODO: Add a turn number.
  int count;
  std::string text;
  Game_message() { count = 1; text = ""; };
  Game_message(std::string T) : text (T) { count = 1; };
};

class Game
{
public:
  Game();
  ~Game();

  bool setup();
  bool main_loop();
  void do_action(Interface_action act);
  //void draw();
  void update_hud();

  void move_monsters();

  void player_move(int xdif, int ydif); // Handles all aspects of moving player

  void add_msg(const char* msg, ...);
  void print_messages();

  Map*      map;
  Worldmap* worldmap;
  Player*   player;
  Monster_pool monsters;

private:
  Window *w_map;
  Window *w_hud;
  cuss::interface i_hud;
  std::vector<Game_message> messages;
  int new_messages;
};

#endif
