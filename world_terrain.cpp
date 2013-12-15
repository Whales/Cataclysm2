#include "world_terrain.h"
#include "stringfunc.h"

World_terrain::World_terrain()
{
  uid = -1;
  name = "ERROR";
  sym = glyph('x', c_white, c_red);
}

bool World_terrain::load_data(std::istream &data)
{
  if (data.eof()) {
    return false;
  }
  name = load_to_character(data, ":;\n", true);
  if (data.eof()) {
    return false;
  }
  sym.load_data(data);
  return true;
}
