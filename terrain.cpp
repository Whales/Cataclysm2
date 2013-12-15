#include "terrain.h"
#include "stringfunc.h"

Terrain::Terrain()
{
  uid = -1;
  name = "ERROR";
  sym = glyph('x', c_white, c_red);
  movecost = 100;
  flags = 0;
}

bool Terrain::load_data(std::istream &data)
{
  if (data.eof()) {
    return false;
  }
  name = load_to_character(data, ":;\n", true);
  if (data.eof()) {
    return false;
  }
  sym.load_data(data);
  if (data.eof()) {
    return false;
  }
  data >> movecost;
// TODO: Flag loading.
  return true;
}
