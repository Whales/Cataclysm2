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
// The internal data name, used in data files
std::string skill_type_name(Skill_type type);
// The name as we display it to the user
std::string skill_type_user_name(Skill_type type);
bool is_skill_mental(Skill_type type);

struct Skill_set
{
  Skill_set();
  ~Skill_set();

  Skill_set& operator=(const Skill_set& rhs);

  int  get_level(Skill_type type) const;
  void set_level(Skill_type type, int lev);
  void increase_level(Skill_type type, int amount = 1);

  int  get_max_level(Skill_type type) const;
  void set_max_level(Skill_type type, int lev);
  void increase_max_level(Skill_type type, int amount = 1);

  void unlock_skill(Skill_type type); // No max level anymore!

  int  improve_cost(Skill_type type);
  bool is_unlocked(Skill_type type) const;
  bool maxed_out(Skill_type type) const;  // true if level >= max_level

private:
  int     level[SKILL_MAX];
  int max_level[SKILL_MAX];
  bool unlocked[SKILL_MAX];
};

#endif
