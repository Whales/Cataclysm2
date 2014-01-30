#ifndef _DICE_H_
#define _DICE_H_

#include <istream>
#include <string>

struct Dice
{
  int number;
  int sides;
  int bonus;

  Dice(int N = 0, int S = 1, int B = 0) : number (N), sides (S), bonus (B) { }
  ~Dice() { }

  int roll();

  bool load_data(std::istream &data, std::string owner_name = "Unknown");
};

#endif
