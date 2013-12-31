#ifndef _MONSTER_TYPE_H_
#define _MONSTER_TYPE_H_

#include <string>
#include <istream>
#include <vector>
#include "glyph.h"
#include "enum.h"
#include "attack.h"

struct Monster_type
{
  Monster_type();
  ~Monster_type();

  std::string name;
  int uid;
  glyph sym;

  int speed;
  std::vector<Attack> attacks;
  int total_attack_weight;

  void assign_uid(int id);
  std::string get_name();
  bool load_data(std::istream &data);

  bool has_sense(Sense_type type);

private:
  std::vector<bool> senses;

};

#endif
