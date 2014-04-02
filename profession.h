#ifndef _PROFESSION_H_
#define _PROFESSION_H_

#include "skill.h"
#include "mapgen.h" // For Item_type_chance
#include <string>
#include <vector>
#include <istream>

struct Profession
{
  Profession();
  ~Profession();

// Functions for Datapool
  void assign_uid(int id);
  std::string get_data_name();
  bool load_data(std::istream& data);

  int uid;
  std::string name;
  std::string description;

/* We won't use most features of Item_group (like chances or a name); all we
 * really need is a list of quantities of items.  But Item_group handles that,
 * AND has a built-in loading function!
 */
  Item_group items;

  Skill_set skills;
};

#endif
