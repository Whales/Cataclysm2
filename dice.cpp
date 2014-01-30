#include "dice.h"
#include "rng.h"
#include "window.h"

int Dice::roll()
{
  return dice(number, sides) + bonus;
}

bool Dice::load_data(std::istream &data, std::string owner_name)
{
  if (!data.good()) {
    return false;
  }

  int current_number = 0;
  bool set_number = false, set_sides = false, negative_bonus = false;
  while (data.good()) {
    char ch = data.get();
    if (data.good()) {
      if (ch >= '0' && ch <= '9') {
        current_number = current_number * 10 + (ch - '0');
      } else if (ch == 'd' || ch == 'D') {
        if (set_number) {
          debugmsg("Two 'd' characters in dice spec (%s)", owner_name.c_str());
          return false;
        }
        set_number = true;
        number = current_number;
        current_number = 0;
      } else if (ch == '+' || ch == '-') {
        if (set_sides) {
          debugmsg("More than one bonus in dice spec (%s)", owner_name.c_str());
          return false;
        }
        set_sides = true;
        sides = current_number;
        current_number = 0;
        if (ch == '-') {
          negative_bonus = true;
        }
      } else if (ch != ' ') {
        debugmsg("Extraneous character '%c' in dice spec (%s)", ch,
                 owner_name.c_str());
        return false;
      }
    } // if (data.good())
  } // while (data.good())

  if (negative_bonus) {
    bonus = 0 - current_number;
  } else {
    bonus = current_number;
  }
  return true;
}
