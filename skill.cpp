#include "skill.h"
#include "stringfunc.h"
#include "window.h" // For debugmsg()

#include <string>

Skill_set::Skill_set()
{
  for (int i = 0; i < SKILL_MAX; i++) {
    level[i] = 0;
    max_level[i] = 0;
    if (!is_skill_mental( Skill_type(i) )) {
      unlocked[i] = true;
    } else {
      unlocked[i] = false;
    }
  }
}

Skill_set::~Skill_set()
{
}

Skill_set& Skill_set::operator=(const Skill_set& rhs)
{
  if (this == &rhs) {
    return (*this);
  }

  for (int i = 0; i < SKILL_MAX; i++) {
    level[i] = rhs.get_level( Skill_type(i) );
  }

  return (*this);
}

int Skill_set::get_level(Skill_type type) const
{
  return level[type];
}

void Skill_set::set_level(Skill_type type, int lev)
{
  if (lev < 0) {
    debugmsg("Setting skill level to %d (%s)!",
             lev, skill_type_name(type).c_str());
    return;
  }
  level[type] = lev;
}

void Skill_set::increase_level(Skill_type type, int amount)
{
  level[type] += amount;
  if (level[type] < 0) {
    level[type] = 0;
  }
}

int Skill_set::get_max_level(Skill_type type) const
{
  return max_level[type];
}

void Skill_set::set_max_level(Skill_type type, int lev)
{
  if (lev < 0) {
    debugmsg("Setting skill max level to %d (%s)!",
             lev, skill_type_name(type).c_str());
    return;
  }
  max_level[type] = lev;
}

void Skill_set::increase_max_level(Skill_type type, int amount)
{
  max_level[type] += amount;
  if (max_level[type] < 0) {
    max_level[type] = 0;
  }
}

void Skill_set::unlock_skill(Skill_type type)
{
  unlocked[type] = true;
}

int Skill_set::improve_cost(Skill_type type)
{
// To get level x, we need to spend (3x)^2 + 50, rounded down to multiple of 5
  int ret = (1 + get_level(type)) * 3;
  ret *= ret;
  ret += 50;
  ret -= ret % 5;
  return ret;
}

bool Skill_set::is_unlocked(Skill_type type) const
{
  return unlocked[type];
}

bool Skill_set::maxed_out(Skill_type type) const
{
  return (!unlocked[type] && level[type] >= max_level[type]);
}

Skill_type lookup_skill_type(std::string name)
{
  name = no_caps( trim( name ) );
  for (int i = 0; i < SKILL_MAX; i++) {
    Skill_type ret = Skill_type(i);
    if (no_caps( skill_type_name(ret) ) == name) {
      return ret;
    }
  }
  return SKILL_NULL;
}

std::string skill_type_name(Skill_type type)
{
  switch (type) {
    case SKILL_NULL:          return "NULL";

    case SKILL_MELEE:         return "melee";
    case SKILL_UNARMED:       return "unarmed";
    case SKILL_BASH:          return "bashing_weapons";
    case SKILL_CUT:           return "cutting_weapons";
    case SKILL_PIERCE:        return "piercing_weapons";

    case SKILL_DODGE:         return "dodge";
    case SKILL_THROWING:      return "throwing";

    case SKILL_LAUNCHERS:     return "launchers";
    case SKILL_HANDGUNS:      return "handguns";
    case SKILL_SHOTGUNS:      return "shotguns";
    case SKILL_SMGS:          return "SMGs";
    case SKILL_RIFLES:        return "rifles";
    case SKILL_BOWS:          return "bows";

    case SKILL_COOKING:       return "cooking";
    case SKILL_MECHANICS:     return "mechanics";
    case SKILL_ELECTRONICS:   return "electronics";
    case SKILL_CONSTRUCTION:  return "construction";

    case SKILL_SPEECH:        return "speech";
    case SKILL_BARTER:        return "barter";

    case SKILL_FIRST_AID:     return "first_aid";
    case SKILL_BOTANY:        return "botany";
    case SKILL_SURVIVAL:      return "survival";
    case SKILL_DRIVING:       return "driving";

    case SKILL_MAX:           return "ERROR - SKILL_MAX";
    default:                  return "ERROR - Unnamed skill";
  }
  return "ERROR - Escaped skill_type_name() switch!";
}

std::string skill_type_user_name(Skill_type type)
{
  switch (type) {
    case SKILL_NULL:          return "NULL";

    case SKILL_MELEE:         return "Melee";
    case SKILL_UNARMED:       return "Unarmed Combat";
    case SKILL_BASH:          return "Bashing Weapons";
    case SKILL_CUT:           return "Cutting Weapons";
    case SKILL_PIERCE:        return "Piercing Weapons";

    case SKILL_DODGE:         return "Dodge";
    case SKILL_THROWING:      return "Throwing";

    case SKILL_LAUNCHERS:     return "Launchers";
    case SKILL_HANDGUNS:      return "Handguns";
    case SKILL_SHOTGUNS:      return "Shotguns";
    case SKILL_SMGS:          return "SMGs";
    case SKILL_RIFLES:        return "Rifles";
    case SKILL_BOWS:          return "Bows";

    case SKILL_COOKING:       return "Cooking";
    case SKILL_MECHANICS:     return "Mechanics";
    case SKILL_ELECTRONICS:   return "Electronics";
    case SKILL_CONSTRUCTION:  return "Construction";

    case SKILL_SPEECH:        return "Speech";
    case SKILL_BARTER:        return "Barter";

    case SKILL_FIRST_AID:     return "First Aid";
    case SKILL_BOTANY:        return "Botany";
    case SKILL_SURVIVAL:      return "Survival";
    case SKILL_DRIVING:       return "Driving";

    case SKILL_MAX:           return "ERROR - SKILL_MAX";
    default:                  return "ERROR - Unnamed skill";
  }
  return "ERROR - Escaped skill_type_name() switch!";
}

bool is_skill_mental(Skill_type type)
{
  switch (type) {
    case SKILL_NULL:          return false;

    case SKILL_MELEE:         return false;
    case SKILL_UNARMED:       return false;
    case SKILL_BASH:          return false;
    case SKILL_CUT:           return false;
    case SKILL_PIERCE:        return false;

    case SKILL_DODGE:         return false;
    case SKILL_THROWING:      return false;

    case SKILL_LAUNCHERS:     return false;
    case SKILL_HANDGUNS:      return false;
    case SKILL_SHOTGUNS:      return false;
    case SKILL_SMGS:          return false;
    case SKILL_RIFLES:        return false;
    case SKILL_BOWS:          return false;

    case SKILL_COOKING:       return true;
    case SKILL_MECHANICS:     return true;
    case SKILL_ELECTRONICS:   return true;
    case SKILL_CONSTRUCTION:  return true;

// Social skills may SEEM mental, but really they're applied.
    case SKILL_SPEECH:        return false;
    case SKILL_BARTER:        return false;

    case SKILL_FIRST_AID:     return true;
    case SKILL_BOTANY:        return true;
    case SKILL_SURVIVAL:      return true;
// Can't learn driving from a book!
    case SKILL_DRIVING:       return false;

    case SKILL_MAX:
      debugmsg("ERROR - tried to check if SKILL_MAX is mental.");
      return false;

    default:
      debugmsg("ERROR - is_skill_mental() missing %s!",
               skill_type_name(type).c_str());
      return false;
  }
  debugmsg("ERROR - Escaped is_skill_mental() switch!");
  return false;
}
