#ifndef _ITEM_H_
#define _ITEM_H_

#include <string>
#include "itemtype.h"

class Item
{
public:
  Item();
  ~Item();

  Itemtype* type;

  glyph top_glyph();
  std::string get_name();
private:
};

#endif
