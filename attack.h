#ifndef _ATTACK_H_
#define _ATTACK_H_

#include "enum.h"
#include "item.h"
#include <string>

Body_part random_body_part_to_hit();

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
};

#endif
