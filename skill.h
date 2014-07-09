#ifndef _SKILL_H_
#define _SKILL_H_

#include <string>

enum Skill_type
{
  SKILL_NULL = 0,
// Melee stuff
  SKILL_MELEE,
  SKILL_UNARMED,
  SKILL_BASH,
  SKILL_CUT,
  SKILL_PIERCE,
  SKILL_DODGE,

// Ranged stuff
  SKILL_THROWING,
  SKILL_LAUNCHERS,
  SKILL_HANDGUNS,
  SKILL_SHOTGUNS,
  SKILL_SMGS,
  SKILL_RIFLES,
  SKILL_BOWS,

// Crafting & Construction
  SKILL_COOKING,
  SKILL_MECHANICS,
  SKILL_ELECTRONICS,
  SKILL_CONSTRUCTION,

// Interpersonal
  SKILL_SPEECH,
  SKILL_BARTER,

// Other stuff
  SKILL_FIRST_AID,
  SKILL_BOTANY,
  SKILL_SURVIVAL,
  SKILL_DRIVING,

  SKILL_MAX
};

Skill_type lookup_skill_type(std::string name);
std::string skill_type_name(Skill_type type);

struct Skill_set
{
  Skill_set();
  ~Skill_set();

  Skill_set& operator=(const Skill_set& rhs);

  int  get_level(Skill_type type) const;
  void set_level(Skill_type type, int lev);

private:
  int level[SKILL_MAX];
};

#endif
