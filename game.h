#ifndef _GAME_H_
#define _GAME_H_

#include "map.h"
#include "cuss.h"
#include "worldmap.h"
#include "monster.h"
#include "player.h"
#include "keybind.h"
#include "pathfind.h"

struct Game_message
{
// TODO: Add a turn number.
  int count;
  std::string text;
  Game_message() { count = 1; text = ""; }
  Game_message(std::string T) : text (T) { count = 1; }
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
  void move_entities();
  void clean_up_dead_entities();
  void handle_player_activity();
  void complete_player_activity();

// Engine - Called-as-needed
  void shift_if_needed();  // Shift the map, if the player's not in the center
  void make_sound(std::string desc, Tripoint pos);
  void make_sound(std::string desc, Point pos);
  void make_sound(std::string desc, int x, int y);

  void launch_projectile(Ranged_attack attack,
                         Point origin, Point target);
  void launch_projectile(Item it, Ranged_attack attack,
                         Point origin, Point target);
  void launch_projectile(Entity* shooter, Ranged_attack attack,
                         Point origin, Point target);
  void launch_projectile(Entity* shooter, Item it, Ranged_attack attack,
                         Point origin, Point target);

  void player_move(int xdif, int ydif); // Handles all aspects of moving player
  void player_move_vertical(int zdif);
  void add_msg(std::string msg, ...);

// UI - Output functions
  void update_hud();
  void print_messages();

// UI - Special screens
  void pickup_items(Tripoint pos);
  void pickup_items(Point    pos);
  void pickup_items(int posx, int posy);
// TODO: Both are limited in that they can not return a point that the player
//       cannot currently see (they return Point() instead).
  Point target_selector           (int startx = -1, int starty = -1);
  std::vector<Point> path_selector(int startx = -1, int starty = -1);

// Data - Universal access functions
  int get_item_uid();

  Map*        map;
  Worldmap*   worldmap;
  Player*     player;
  Entity_pool entities;

  Generic_map scent_map;

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
