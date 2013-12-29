#include "item.h"
#include <sstream>

Item::Item(Item_type* T)
{
  type = T;
}

Item::Item(const Item &rhs)
{
  type = rhs.type;
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

std::string Item::get_name_with_article()
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
