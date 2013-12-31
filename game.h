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

// Setup - Called only once
  bool setup();

// Engine - Main loop functions
  bool main_loop();
  void do_action(Interface_action act);
  void move_monsters();

// Engine - Called-as-needed
  void shift_if_needed();  // Shift the map, if the player's not in the center

// Engine - Game-altering functions
  void player_move(int xdif, int ydif); // Handles all aspects of moving player
  void add_msg(const char* msg, ...);

// UI - Output functions
  void update_hud();
  void print_messages();

// UI - Special screens
  void pickup_items(int posx, int posy);

// Data - Universal access functions
  int get_item_uid();

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
  int next_item_uid;
  bool game_over;
};

#endif
