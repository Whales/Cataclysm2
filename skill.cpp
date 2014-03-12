#include "skill.h"

Skill_set::Skill_set()
{
  for (int i = 0; i < SKILL_MAX; i++) {
    level[i] = 0;
  }
}

Skill_set::~Skill_set()
{
}

int Skill_set::get_level(Skill_type type)
{
  return level[type];
}

void Skill_set::set_level(Skill_type type, int lev)
{
  level[type] = lev;
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
  return SKILL_TYPE_NULL;
}

std::string skill_type_name(Skill_type type)
{
  switch (type) {
    case SKILL_NULL:      return "NULL";
    case SKILL_MELEE:     return "melee";
    case SKILL_UNARMED:   return "unarmed";
    case SKILL_BASH:      return "bashing_weapons";
    case SKILL_CUT:       return "cutting_weapons";
    case SKILL_PIERCE:    return "piercing_weapons";
    case SKILL_DODGE:     return "dodge";
    case SKILL_LAUNCHERS: return "launchers";
    case SKILL_HANDGUNS:  return "handguns";
    case SKILL_SHOTGUNS:  return "shotguns";
    case SKILL_SMGS:      return "SMGs";
    case SKILL_RIFLES:    return "rifles";
    case SKILL_FIRST_AID: return "first_aid";
    case SKILL_MAX:       return "ERROR - SKILL_MAX";
    default:              return "ERROR - Unnamed skill";
  }
  return "ERROR - Escaped skill_type_name() switch!";
}
