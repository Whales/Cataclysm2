#ifndef _EXPLOSION_H_
#define _EXPLOSION_H_

#include "dice.h"
#include "geometry.h" // For Tripoint
#include <istream>
#include <string>

struct Explosion
{
  Explosion();
  ~Explosion();

  Dice radius;  // Total radius
  Dice force;   // Concussive force at epicenter; decreases with distance

  Dice shrapnel_count;  // Number of pieces of shrapnel to throw
  Dice shrapnel_damage; // Damage dealt by shrapnel

  std::string field_name; // Name of field to spawn
  int field_chance;     // Percentage chance of placing field in each tile
  Dice field_duration;  // Duration of field; if <= 0 no field is placed

  std::string reason;   // What caused the explosion - for kill credit

  bool load_data(std::istream& data, std::string owner_name);
  void explode(Tripoint epicenter);
};

#endif
