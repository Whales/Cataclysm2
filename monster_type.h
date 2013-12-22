#ifndef _MONSTER_TYPE_H_
#define _MONSTER_TYPE_H_

#include <string>
#include <istream>
#include "glyph.h"

struct Monster_type
{
  Monster_type();
  ~Monster_type();

  std::string name;
  int uid;
  glyph sym;

  int speed;

  void assign_uid(int id);
  std::string get_name();
  bool load_data(std::istream &data);
};

#endif
