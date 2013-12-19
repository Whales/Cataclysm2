#include "item.h"

Item::Item()
{
  type = NULL;
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
