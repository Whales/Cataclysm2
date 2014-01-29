#ifndef _MONSTER_TYPE_H_
#define _MONSTER_TYPE_H_

#include "glyph.h"
#include "enum.h"
#include "attack.h"
#include <string>
#include <istream>
#include <vector>

struct Monster_genus;

struct Monster_type
{
  Monster_type();
  ~Monster_type();

  Monster_genus *genus;
  std::string name;
  std::string name_plural;
  int uid;
  glyph sym;

  Item_type *corpse;

  int minimum_hp, maximum_hp;
  int speed;
  std::vector<Attack> attacks;
  int total_attack_weight;
  Intel_level intel;

  void set_genus(Monster_genus *mg);
  void assign_uid(int id);
  std::string get_name();
  bool load_data(std::istream &data);

  bool has_sense(Sense_type type);

private:
  bool attacks_copied_from_genus;
  bool senses_copied_from_genus;
  std::vector<bool> senses;

};

struct Monster_genus
{
  Monster_genus();
  ~Monster_genus();

  std::string name;
  int uid;

  Monster_type default_values; // Default values for monsters in this genus

  void assign_uid(int id);
  std::string get_name();
  bool load_data(std::istream &data);

};

#endif
