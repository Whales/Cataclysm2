#ifndef _MONSTER_TYPE_H_
#define _MONSTER_TYPE_H_

#include "glyph.h"
#include "enum.h"
#include "attack.h"
#include "entity_ai.h"
#include "dice.h"
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
  std::string display_name;
  std::string display_name_plural;
  int uid;
  glyph sym;

  Dice hp_dice;
  bool hp_set;
  int speed;
  int chance;
  std::vector<Attack> attacks;
  std::vector<Ranged_attack> ranged_attacks;
  int total_attack_weight;
  int total_ranged_attack_weight;
  Entity_AI AI;

  void set_genus(Monster_genus *mg);
  void assign_uid(int id);
  std::string get_data_name();
  std::string get_name();
  std::string get_name_plural();
  bool load_data(std::istream &data);

  bool has_sense(Sense_type type);

private:
  bool attacks_copied_from_genus;
  bool ranged_attacks_copied_from_genus;
  bool senses_copied_from_genus;
  std::vector<bool> senses;

};

struct Monster_genus
{
  Monster_genus();
  ~Monster_genus();

  std::string name;
  std::string display_name;
  int uid;

  Monster_type default_values; // Default values for monsters in this genus
// Spawning stuff
  std::vector<Monster_type*> members;
  int total_chance;

  void add_member(Monster_type* member);
  Monster_type* random_member();

  void assign_uid(int id);
  std::string get_data_name();
  std::string get_name();
  bool load_data(std::istream &data);

};

#endif
