#ifndef _MONSTER_H_
#define _MONSTER_H_

class Monster
{
public:
  Monster(Monster_type *T = NULL);
  ~Monster();

  Monster_type *type;

  glyph top_glyph();
  std::string get_name();
private:
};

#endif
