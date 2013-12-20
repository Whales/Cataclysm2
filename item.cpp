#include "item.h"

Item::Item(Itemtype* T)
{
  type = T;
}

Item::~Item()
{
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
