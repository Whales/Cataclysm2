#include "item.h"
#include "game.h"
#include "globals.h"
#include "cuss.h"
#include <sstream>

Item::Item(Item_type* T)
{
  type = T;
  count = 1;
  if (type) {
    uid = GAME.get_item_uid();
  } else {
    uid = -1;
  }
}

Item::Item(const Item &rhs)
{
  type  = rhs.type;
  count = rhs.count;
  uid   = rhs.uid;
}

Item::~Item()
{
}

Item& Item::operator=(const Item& rhs)
{
  type = rhs.type;

  return *this;
}

glyph Item::top_glyph()
{
  if (type) {
    return type->sym;
  }
  return glyph();
}

std::string Item::get_name()
{
  if (type) {
    return type->name;
  }
  return "Typeless item";
}

std::string Item::get_name_indefinite()
{
// TODO: Check Item_type for "plural" flag
// TODO: Unique items?
  if (type) {
    std::stringstream ret;
    ret << "a " << type->name;
    return ret.str();
  }
  return "a typeless item";
}

std::string Item::get_name_definite()
{
// TODO: Check Item_type for "plural" flag
// TODO: Unique items?
  if (type) {
    std::stringstream ret;
    ret << "the " << type->name;
    return ret.str();
  }
  return "the typeless item";
}

int Item::get_weight()
{
  if (type) {
    return type->weight;
  }
  return 0;
}

int Item::get_volume()
{
  if (type) {
    return type->volume;
  }
  return 0;
}

Item_action Item::show_info()
{
  if (!type) {
    return IACT_NULL;
  }
  Window w_info(0, 0, 80, 24);
  cuss::interface i_info;
  if (!i_info.load_from_file("cuss/i_item_info.cuss")) {
    debugmsg("Couldn't load cuss/i_item_info.cuss!");
    return IACT_NULL;
  }

  i_info.set_data("item_name", type->name);
  i_info.set_data("num_weight", type->weight);
  i_info.set_data("num_volume", type->volume);
  i_info.set_data("num_bash", type->bash);
  i_info.set_data("num_cut", type->cut);
  i_info.set_data("num_pierce", type->pierce);
  i_info.set_data("num_to_hit", type->to_hit);
  i_info.set_data("num_speed", type->attack_speed);
  i_info.set_data("description", type->description);
  i_info.draw(&w_info);
  while (true) {
    long ch = input();
    if (ch == 'd' || ch == 'D') {
      return IACT_DROP;
    } else if (ch == 'w') {
      return IACT_WIELD;
    } else if (ch == KEY_ESC || ch == 'q' || ch == 'Q') {
      return IACT_NULL;
    }
  }
  return IACT_NULL;
}

std::string list_items(std::vector<Item> *items)
{
  std::stringstream item_text;
  for (int i = 0; i < items->size(); i++) {
    item_text << (*items)[i].get_name_indefinite();
    if (i == items->size() - 1 && items->size() > 1) {
      item_text << " and ";
    } else if (i < items->size() - 1) {
      item_text << ", ";
    }
  }
  item_text << ".";
  return item_text.str();
}
