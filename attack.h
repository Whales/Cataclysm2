#ifndef _ATTACK_H_
#define _ATTACK_H_

#include "enum.h"
#include "dice.h"
#include "geometry.h" // For Tripoint
#include "damage_set.h"
#include "field.h"
#include "skill.h"
#include <string>
#include <vector>
#include <istream>

Body_part random_body_part_to_hit();

struct Stats;
class Item;
class Field_type;
class Field_pool;

/* TODO:  Special effects.  It'd be nice to specify that an attack (or an item
 *        from which attacks are derived) has the ability to knock the target
 *        back a tile or two, to poison them, to stun them, etc.
 * TODO:  Special effects bound to damage type.  Like in Cataclysm 1.  Bashing
 *        could stun the target, cutting could cause bleeding or something, and
 *        piercing could "skewer" them, reducing speed but also possibly yanking
 *        the attacker's weapon out of their hands.
 */

enum Melee_hit_type
{
  MELEE_HIT_NULL = 0,
  MELEE_HIT_GRAZE,
  MELEE_HIT_NORMAL,
  MELEE_HIT_CRITICAL,
  MELEE_HIT_MAX
};

struct Attack
{
  std::string verb_second;
  std::string verb_third;
  int weight; // For monster attacks - how likely this attack is to be used
  int speed;
  int to_hit;
  int damage[DAMAGE_MAX];
  bool using_weapon;

  Attack();
  ~Attack();

  bool load_data(std::istream &data, std::string owner_name = "unknown");

  void use_weapon(Item weapon, Stats stats);  // Stats is only used for speed
  void adjust_with_stats(Stats stats);        // Adjusts damage
  void adjust_with_skills(Skill_set skills);  // Adjusts damage, speed, to_hit

  Damage_set roll_damage(Melee_hit_type hit_type = MELEE_HIT_NORMAL);
  int roll_damage_type(Damage_type type,
                       Melee_hit_type hit_type = MELEE_HIT_NORMAL);
};

enum Ranged_hit_type
{
  RANGED_HIT_NULL = 0,
  RANGED_HIT_GRAZE,     // Damage is between 0 and full
  RANGED_HIT_NORMAL,    // Damage is between 80% and 100%
  RANGED_HIT_CRITICAL,  // Damage is between 100% and 150%
  RANGED_HIT_HEADSHOT,  // Damage is between 300% and 500%
  RANGED_HIT_MAX
};

struct Ranged_attack
{
  Ranged_attack();
  ~Ranged_attack();

  std::string verb_second;
  std::string verb_third;
  int weight;       // Monster attacks - how likely this attack is to be used
  int speed;        // AP used
  int charge_time;  // Also for monsters - how frequently can we use this?
  int range;        // Max range of the attack
  Dice variance;    // In 1/10ths of a degree
  int damage       [DAMAGE_MAX];
  int armor_divisor[DAMAGE_MAX];  // Times 10; 20 = halve the armor, 5 = double it

/* TODO: Add the following as they're implemented:
 *      Status_effect this causes (Blinding, stunning, etc)
 *      Other???
 */

// Field_pool that we leave in our wake
  Field_pool wake_field;
// Field_pool that is created at the tile hit
  Field_pool target_field;

  bool load_data(std::istream &data, std::string owner_name = "unknown");

  int roll_variance();
  Damage_set roll_damage(Ranged_hit_type hit = RANGED_HIT_NORMAL);
};

#endif
