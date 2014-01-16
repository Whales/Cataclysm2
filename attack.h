#ifndef _ATTACK_H_
#define _ATTACK_H_

#include "enum.h"
#include "item.h"
#include <string>
#include <vector>
#include <istream>

Body_part random_body_part_to_hit();

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

struct Attack
{
  std::string verb_first;
  std::string verb_third;
  int weight; // For monster attacks - how likely this attack is to be used
  int speed;
  int to_hit;
  int damage[DAMAGE_MAX];

  Attack();
  ~Attack();

  bool load_data(std::istream &data, std::string owner_name = "unknown");
  void use_weapon(Item weapon, int strength, int dexterity);

  Damage_set roll_damage();
};

struct Ranged_attack
{
  Ranged_attack();
  ~Ranged_attack();

  std::string verb_first;
  std::string verb_third;
  int weight; // For monster attacks - how likely this attack is to be used
  int speed; // AP used
  int charge_time; // Also for monsters - how frequently can we use this?
  int range;    // Max range of the attack
  std::vector<int> variance; // In 1/10ths of a degree
  int damage[DAMAGE_MAX];
  int armor_divisor[DAMAGE_MAX];

/* TODO: Add the following as they're implemented:
 *      Status_effect this causes (Blinding, stunning, etc)
 *      Field this leaves in its wake (smoke, slime, etc)
 *      Field pool this plants at its destination (acid, fire)
 *      Other???
 */

  bool load_data(std::istream &data, std::string owner_name = "unknown");

  int roll_variance();
  Damage_set roll_damage();
};

#endif
