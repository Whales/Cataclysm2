#ifndef _ATTACK_H_
#define _ATTACK_H_

#include "enum.h"
#include "item.h"
#include <string>
#include <vector>

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

  void use_weapon(Item weapon, int strength, int dexterity);

  Damage_set roll_damage();
};

#endif
