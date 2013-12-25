#ifndef _MONSTER_TYPE_H_
#define _MONSTER_TYPE_H_

#include <string>
#include <istream>
#include <vector>
#include "glyph.h"
#include "enum.h"

struct Monster_attack
{
  std::string verb;
  int speed;
  int to_hit;
  int damage[DAMAGE_MAX];

  Monster_attack();
};

struct Monster_type
{
  Monster_type();
  ~Monster_type();

  std::string name;
  int uid;
  glyph sym;

  int speed;
  std::vector<Monster_attack> attacks;

  void assign_uid(int id);
  std::string get_name();
  bool load_data(std::istream &data);

  bool has_sense(Sense_type type);
private:
  std::vector<bool> senses;

};

#endif
