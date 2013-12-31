#ifndef _ATTACK_H_
#define _ATTACK_H_

#include "enum.h"
#include <string>

struct Attack
{
  std::string verb;
  int weight;
  int speed;
  int to_hit;
  int damage[DAMAGE_MAX];

  Attack();
};

#endif
