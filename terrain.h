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

struct Terrain_signal_handler
{
  Terrain_signal_handler();
  ~Terrain_signal_handler(){}

  bool load_data(std::istream& data, std::string owner_name);

  std::string result;   // The terrain we become
  int success_rate;     // Percentage rate of success

  std::list<Stat_bonus> bonuses;  // Bonuses to success_rate based on stats

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

  Terrain_smash smash;
  bool smashable;
  std::string open_result;
  std::string close_result;
  std::string destroy_result;
// A map of what happens when a tool' terrain_action is applied
  std::map<std::string,Terrain_signal_handler> signal_handlers;

  bool can_smash() { return smashable; }
  bool can_open()  { return !open_result.empty();  }
  bool can_close() { return !close_result.empty(); }

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

#endif
