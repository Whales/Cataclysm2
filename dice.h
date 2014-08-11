#ifndef _DICE_H_
#define _DICE_H_

#include <istream>
#include <string>
#include <vector>

struct Dice
{
  int number;
  int sides;
  int bonus;

  bool negative;  // Used in others; if true, we SUBTRACT this!

  std::vector<Dice> others;

  Dice(int N = 0, int S = 1, int B = 0);
  ~Dice() { }

  int roll();
  Dice base() const;  // Strip off all others
  Dice& operator= (const Dice& rhs);
  Dice& operator+=(const Dice& rhs);
  Dice& operator-=(const Dice& rhs);
  Dice& operator+=(const int& rhs);
  Dice& operator-=(const int& rhs);

  std::string str();

  bool load_data(std::istream &data, std::string owner = "Unknown");

};

inline Dice operator+(Dice lhs, const Dice& rhs)
{
  lhs += rhs;
  return lhs;
}

inline Dice operator-(Dice lhs, const Dice& rhs)
{
  lhs -= rhs;
  return lhs;
}

inline Dice operator-(Dice lhs, const int& rhs)
{
  lhs -= rhs;
  return lhs;
}

inline Dice operator+(Dice lhs, const int& rhs)
{
  lhs += rhs;
  return lhs;
}

#endif
