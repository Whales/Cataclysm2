#ifndef _DAMAGE_SET_H_
#define _DAMAGE_SET_H_

#include "enum.h" // For DAMAGE_MAX

struct Damage_set
{
  Damage_set();
  ~Damage_set();

  void set_damage(Damage_type type, int amount);
  void set_damage(int index, int amount);
  int  get_damage(Damage_type type) const;
  int  get_damage(int index) const;

  int  total_damage();

  Damage_set& operator+=(const Damage_set& rhs);
  Damage_set& operator-=(const Damage_set& rhs);

private:
  int damage[DAMAGE_MAX];
};
inline Damage_set operator+(Damage_set lhs, const Damage_set& rhs)
{
  lhs += rhs;
  return lhs;
}
inline Damage_set operator-(Damage_set lhs, const Damage_set& rhs)
{
  lhs -= rhs;
  return lhs;
}

#endif
