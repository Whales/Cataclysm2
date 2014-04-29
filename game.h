#ifndef _GAME_H_
#define _GAME_H_

#include "map.h"
#include "cuss.h"
#include "worldmap.h"
#include "monster.h"
#include "player.h"
#include "keybind.h"
#include "pathfind.h"
#include "time.h"

// Identical messages within MESSAGE_GAP turns of each other are combined
#define MESSAGE_GAP 3

struct Game_message
{
  std::string text;
  int turn;
  int count;
  Game_message() { turn = 0; count = 1; text = ""; }
  Game_message(std::string T, int TN) : text (T), turn (TN) { count = 1; }
};

class Game
{
public:
  Game();
  ~Game();

/**** Setup - Called only once ****/
  bool setup_ui();
  bool starting_menu(); // Choose from create character, load character, etc.
/* setup_new_game() creates the Worldmap, Map, and Player objects.
 * If a world_index is passed to it, it'll attempt to load that Worldmap - the
 * index refers to an element in worldmap_names.  Otherwise, it'll create a new
 * Worldmap.
 */
  bool setup_new_game(int world_index = -1);
  int  world_screen(); // Returns a new world_index
  void create_world();

/**** Engine - Main loop functions ****/
  bool main_loop();
  void reset_temp_values();
  void do_action(Interface_action act);
  void move_entities();
  void clean_up_dead_entities();
  void handle_player_activity();
  void complete_player_activity();
  void process_active_items();

/**** Engine - Called-as-needed ****/
  void shift_if_needed();  // Shift the map, if the player's not in the center
  void make_sound(std::string desc, Tripoint pos);
  void make_sound(std::string desc, Point pos);
  void make_sound(std::string desc, int x, int y);

// Pass NULL as shooter and Item() as it
  void launch_projectile(Ranged_attack attack,
                         Tripoint origin, Tripoint target);
// Pass NULL as shooter
  void launch_projectile(Item it, Ranged_attack attack,
                         Tripoint origin, Tripoint target);
// Pass Item() as it
  void launch_projectile(Entity* shooter, Ranged_attack attack,
                         Tripoint origin, Tripoint target);
// This one is the *real* one
  void launch_projectile(Entity* shooter, Item it, Ranged_attack attack,
                         Tripoint origin, Tripoint target);

  void player_move(int xdif, int ydif); // Handles all aspects of moving player
  void player_move_vertical(int zdif);

  void add_msg(std::string msg, ...);
// msg_query_yn adds a message, refreshes the HUD, and accepts Y/N input
  bool msg_query_yn(std::string msg, ...);


  void add_active_item(Item* it);
  void remove_active_item(Item* it);
  void remove_active_item_uid(int uid);
  bool destroy_item(Item* it, int uid = -1);
  bool destroy_item_uid(int uid); // destroy_item(NULL, uid)
// Temp value mutators
  void set_temp_light_level(int level);

/**** UI - Output functions ****/
  void draw_all();
  void update_hud();
  void print_messages();

/**** UI - Special screens and inputs ****/
  void debug_command();
  void pickup_items(Tripoint pos);
  void pickup_items(Point    pos);
  void pickup_items(int posx, int posy);
// TODO: Both are limited in that they can not return a point that the player
//       cannot currently see (they return Tripoint() instead).
  Tripoint target_selector(int startx = -1, int starty = -1,
                           int range  = -1, bool target_entites = false,
                           bool show_path = false);
  std::vector<Tripoint> path_selector(int startx = -1, int starty = -1,
                                      int range  = -1,
                                      bool target_entities = false,
                                      bool show_path = true);

/**** Data - Universal access functions ****/
  int get_item_uid();     // Get the next available UID (and increment)
  int get_furniture_uid();// Get the next available UID (and increment)
  bool minute_timer(int minutes); // Returns true once every $minutes minutes
  bool turn_timer(int turns);     // Returns true once every $turns turns
  int get_light_level();          // Current light distance, based on the time

/* find_item() returns the location of the item.  If it == NULL, use the uid;
 * otherwise, use it.  If it == NULL and uid == -1, just fail immediately.
 * Returns [-1, -1, -1] on fail.
 */
  Tripoint find_item(Item* it, int uid = -1);
  Tripoint find_item_uid(int uid);  // find_item(NULL, uid)

/**** Contained data ****/
  Map*        map;
  Worldmap*   worldmap;
// Note that player should always == &(entities[0])
  Player*     player;
  Entity_pool entities;

  Time time;

// Not used; TODO: Remove this?
  Generic_map scent_map;

private:
  Window *w_map;
  Window *w_hud;
  cuss::interface i_hud;
  std::vector<Game_message> messages;
  std::vector<Item*> active_items;
  std::vector<std::string> worldmap_names;
  int last_target;
  int new_messages;
  int next_item_uid;
  int next_furniture_uid;
  bool game_over;

// Temp values; all reset in reset_temp_values()
  int temp_light_level;

// Just adds the message, after it's been formatted and everything.
// Used by add_msg(), msg_query_yn(), and any other similar functions.
  void add_msg_static(std::string message);
};

#endif
