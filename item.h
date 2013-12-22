#ifndef _ITEM_H_
#define _ITEM_H_

#include <string>
#include "item_type.h"

class Item
{
public:
  Item(Item_type* T = NULL);
  ~Item();

  Item_type* type;

  glyph top_glyph();
  std::string get_name();
private:
};

#endif
