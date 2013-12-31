#ifndef _ITEM_H_
#define _ITEM_H_

#include "item_type.h"
#include <string>
#include <vector>

enum Item_action
{
  IACT_NULL = 0,
  IACT_WIELD,
  IACT_WEAR,
  IACT_DROP,
  IACT_MAX
};

class Item
{
public:
  Item(Item_type* T = NULL);
  Item(const Item &rhs);
  ~Item();

  Item& operator=(const Item &rhs);

  Item_type* type;
  Item_class get_item_class() { return type->get_class(); }

// Info fetching
  glyph top_glyph();
  std::string get_name();
  std::string get_name_indefinite();
  std::string get_name_definite();
  int get_weight();
  int get_volume();

// Interfaces
  Item_action show_info();

  int count;
  int uid;
private:
};

std::string list_items(std::vector<Item> *items);

#endif
