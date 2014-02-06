#ifndef _ATTACK_H_
#define _ATTACK_H_

#include "enum.h"
#include "dice.h"
#include "geometry.h" // For Tripoint
#include <string>
#include <vector>
#include <istream>

Body_part random_body_part_to_hit();

struct Stats;
class Item;
class Field_type;

struct Damage_set
{
  Damage_set();
  ~Damage_set();

  void set_damage(Damage_type type, int amount);
  void set_damage(int index, int amount);
  int  get_damage(Damage_type type) const;
  int  get_damage(int index) const;

  int  total_damage();

  Damage_set& operator+=(const Damage_set& rhs);
  Damage_set& operator-=(const Damage_set& rhs);

private:
  int damage[DAMAGE_MAX];
};
inline Damage_set operator+(Damage_set lhs, const Damage_set& rhs)
{
  lhs += rhs;
  return lhs;
}
inline Damage_set operator-(Damage_set lhs, const Damage_set& rhs)
{
  lhs -= rhs;
  return lhs;
}

/* TODO:  Special effects.  It'd be nice to specify that an attack (or an item
 *        from which attacks are derived) has the ability to knock the target
 *        back a tile or two, to poison them, to stun them, etc.
 * TODO:  Special effects bound to damage type.  Like in Cataclysm 1.  Bashing
 *        could stun the target, cutting could cause bleeding or something, and
 *        piercing could "skewer" them, reducing speed but also possibly yanking
 *        the attacker's weapon out of their hands.
 */
struct Attack
{
  std::string verb_second;
  std::string verb_third;
  int weight; // For monster attacks - how likely this attack is to be used
  int speed;
  int to_hit;
  int damage[DAMAGE_MAX];

  Attack();
  ~Attack();

  bool load_data(std::istream &data, std::string owner_name = "unknown");
  void use_weapon(Item weapon, Stats stats);

  Damage_set roll_damage();
};

struct Field_pool
{
  Field_pool();
  ~Field_pool();

  Field_type* type;
  Dice duration;
  Dice tiles;

  bool exists();

  void drop(Tripoint pos, std::string creator = "");

  bool load_data(std::istream& data, std::string owner_name = "Unknown");
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
  int armor_divisor[DAMAGE_MAX];

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
  Damage_set roll_damage();
};

#endif
