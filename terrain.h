#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include "glyph.h"
#include "enum.h"
#include "dice.h"
#include <vector>
#include <map>
#include <list>

Terrain_flag lookup_terrain_flag(std::string name);
std::string terrain_flag_name(Terrain_flag flag);

struct Terrain;

struct Terrain_smash
{
  Terrain_smash();
  ~Terrain_smash(){}

  std::string failure_sound;
  std::string success_sound;
  Dice armor[DAMAGE_MAX];
  int ignore_chance;

  bool load_data(std::istream &data, std::string name = "unknown");
};

// For signal handling.
// TODO: Generalize this for entities & items, too?
struct Stat_bonus
{
  Stat_bonus(Stat_id _stat = STAT_NULL, Math_operator _op = MATH_NULL,
             int _amount = 0, int _static = 0) :
    stat (_stat), op (_op), amount (_amount), amount_static (_static) {}
  ~Stat_bonus(){}

  bool load_data(std::istream& data, std::string owner_name);

  Stat_id stat;
  Math_operator op;
  int amount;
  int amount_static;  // Used if op is a comparison operator
};

// For signal handling.
struct Terrain_flag_bonus
{
  Terrain_flag_bonus(Terrain_flag _flag = TF_NULL, int _amount = 0) :
    flag (_flag), amount (_amount) {}

  bool load_data(std::istream& data, std::string owner_name);

  Terrain_flag flag;
  int amount;
};


struct Terrain_signal_handler
{
  Terrain_signal_handler();
  ~Terrain_signal_handler(){}

  bool load_data(std::istream& data, std::string owner_name);

  std::string result;   // The terrain we become
  int success_rate;     // Percentage rate of success

  std::list<Stat_bonus> stat_bonuses;  // Bonuses to success_rate based on stats
  std::list<Terrain_flag_bonus> terrain_flag_bonuses;  // based on terrain_flags

  std::string success_message;    // Message when we succeed
  std::string failure_message;    // Message when we fail
};


struct Terrain
{
  int uid;
  std::string name;
  std::string display_name;
  glyph sym;
  unsigned int movecost;
/* Height ranged from 0-100 and reflects how much vertical space is blocked (for
 * line of sight calculations).  It defaults to 100.
 */
  unsigned int height;
  unsigned int hp;      // Defaults to 0.  0 HP means it's indestructible.

  std::string inverse;  // For stairs - the opposite direction

  Terrain_smash smash;
  bool smashable;
  std::string destroy_result;
// A map of what happens when a tool's signal is applied
  std::map<std::string,Terrain_signal_handler> signal_handlers;

  bool can_smash() { return smashable; }

  Terrain();
  ~Terrain(){}

  std::string get_data_name();
  std::string get_name();
  void assign_uid(int id) { uid = id; }

  bool load_data(std::istream &data);

  bool has_flag(Terrain_flag flag);

private:
  std::vector<bool> flags;
};

struct Item_group;

struct Furniture_type
{
  int uid;
  std::string name;
  std::string display_name;
  glyph sym;

  unsigned int move_cost;
  unsigned int height;
  unsigned int weight;
  unsigned int hp;

  Terrain_smash smash;
  bool smashable;
// Items dropped when we destroy it
  Item_group* components;

  Furniture_type();
  ~Furniture_type();

  std::string get_data_name();
  std::string get_name();
  void assign_uid(int id);

  bool has_flag(Terrain_flag flag);

  bool load_data(std::istream& data);

private:
  std::vector<bool> flags;
/* If owns_components is true, then components was created by us, and should be
 * deleted in our destructor.
 * If it's false, then components is a pointer to something in ITEM_GROUPS and
 * should NOT be deleted in our destructor.
 * It defaults to true, since "components = new Item_group" is in our
 * constructor.  If we have a "preset_components:" line in our data file, then
 * we reference an Item_group in ITEM_GROUPS and set owns_components to false.
 */
  bool owns_components;
};

#endif
