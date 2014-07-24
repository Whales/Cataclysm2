#ifndef _MONSTER_ABILITY_H_
#define _MONSTER_ABILITY_H_

#include "var_string.h"
#include "dice.h"
#include <istream>
#include <string>

/* Note:
 * All Monster_ability substructs have a function effect(), which the Monster
 * using the ability should call.  This function returns false if the ability
 * was canceled before being attempted!  If the Monster legitimately attempted
 * to use their ability, but failed for some reason, it should return true.
 * The Monster should use this return value to determine whether to use up some
 * AP, etc.
 */

class Monster;

enum Monster_ability_type
{
  MON_ABILITY_NULL = 0, // nuffin
  MON_ABILITY_SUMMON,   // create one or more named monsters
  MON_ABILITY_SIGNAL,   // apply signal to nearby terrain
  MON_ABILITY_TERRAIN,  // Change (i.e. generate) nearby terrain
  MON_ABILITY_TELEPORT, // Instantly teleport.
  MON_ABILITY_FIELDS,   // Place fields around us.
  MON_ABILITY_MAX
};

// Base struct
struct Monster_ability
{
  Monster_ability();
  virtual ~Monster_ability();

  virtual Monster_ability_type type() { return MON_ABILITY_NULL; }

  bool load_data(std::istream& data, std::string owner);
  virtual bool handle_data(std::string ident, std::istream& data,
                           std::string owner);

  virtual bool effect(Monster* user);

  int frequency;    // How much using this increases the monster's special_timer
  int weight;       // How likely we are to used this (compared to other abils)
  int AP_cost;      // How much AP using this costs (defaults to 0).
  int HP_cost;      // HP lost when using this ability (defaults to 0).
  std::string verb; // "The <monster_name> <verb>!" (so use 3rd person)

};

struct Monster_ability_summon : public Monster_ability
{
  Monster_ability_summon();
  ~Monster_ability_summon(){}

  virtual Monster_ability_type type() { return MON_ABILITY_SUMMON; }

  virtual bool handle_data(std::string ident, std::istream& data,
                           std::string owner);

  virtual bool effect(Monster* user);

  Variable_string monster; // So that summons may vary.
  Dice number;  // How many monsters to place.  Defaults to 1.
  int range;  // How far away can the monster be placed?  Defaults to 1.
  int max_summons;  // Maximum # of children we can have. 0 = no limit (default)
};

// Right now, Monster_ability_signal applies to ALL terrain within <range>.
// We can change this if so desired, but choosing a target may be VERY complex.
struct Monster_ability_signal : public Monster_ability
{
  Monster_ability_signal();
  ~Monster_ability_signal(){}

  virtual Monster_ability_type type() { return MON_ABILITY_SIGNAL; }

  virtual bool handle_data(std::string ident, std::istream& data,
                           std::string owner);

  virtual bool effect(Monster* user);

  Variable_string signal; // A signal to be sent to terrain (may be randomized)
  int range;  // Radius of ability; defaults to 1
};

struct Monster_ability_terrain : public Monster_ability
{
  Monster_ability_terrain();
  ~Monster_ability_terrain(){}

  virtual Monster_ability_type type() { return MON_ABILITY_TERRAIN; }

  virtual bool handle_data(std::string ident, std::istream& data,
                           std::string owner);

  virtual bool effect(Monster* user);

  bool always_replace;  // Defaults to false; if true, always replace terrain

/* Damage is only used if always_replace is false (it defaults to false).  The
 * targeted terrain will be automatically replaced anyway if its HP is 0;
 * otherwise, we reduce its HP by a roll of <damage> and if it reaches 0 (or
 * less), then we do the replacement.
 * Note that by setting <damage> but leaving <terrain> empty, this ability is
 * effectively a terrain-damaging ability (destroyed terrain will be replaced by
 * its <destroy_result>).
 */
  Dice damage;
  Variable_string terrain;  // So that it can be randomized
  int range;  // Radius of ability; defaults to 1
  Dice tiles_affected;  // How many tiles to affect; defaults to 1
};


struct Monster_ability_teleport : public Monster_ability
{
  Monster_ability_teleport();
  ~Monster_ability_teleport(){}

  virtual Monster_ability_type type() { return MON_ABILITY_TELEPORT; }

  virtual bool handle_data(std::string ident, std::istream& data,
                           std::string owner);

  virtual bool effect(Monster* user);

  int range;  // Max range of teleport.
/* If <always_use_max_range> is true, we'll use a random open tile exactly
 * <range> tiles away.  If none are available, we'll try <range> - 1, and so on.
 * If false, we'll teleport to any open tile within <range>.
 * Defaults to false.
 */
  bool always_use_max_range;
/* If <controlled> is false, then the teleport is truly random; any available
 * tile is a potential result.
 * If false, then we'll teleport as close as possible to our target if we're
 * attacking; or as far as possible from our target if we're fleeing.
 * Defaults to false.
 */
  bool controlled;
/* If <phase> is true, then any tile within <range> is a valid target.
 * If false, then only targets which we can sense AND which have a uninterrupted
 * line to them are valid.
 * Defaults to false.
 */
  bool phase;
};

struct Monster_ability_fields
{
  Monster_ability_fields();
  ~Monster_ability_fields(){}

  virtual Monster_ability_type type() { return MON_ABILITY_FIELDS; }

  virtual bool handle_data(std::string ident, std::istream& data,
                           std::string owner);

  virtual bool effect(Monster* user);

  int range;  // Radius of affected tiles; defaults to 1
  bool affect_all_tiles;  // If true, all tiles affected; defaults to false
  bool affect_self; // If true, affect tile under monster; defaults to false
  Dice tiles_affected;  // How many tiles to hit; ignored if <affect_all_tiles>
  Variable_string field_type; // Type of field (maybe random)
  Dice duration;  // Duration of field(s) generated; if <0 rolled, no field made
};

#endif
