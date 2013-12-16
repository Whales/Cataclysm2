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
  std::string ident, junk;
  do {
    if ( ! (data >> ident) ) {
      return false;
    }
    ident = no_caps(ident);
    if (ident == "name:") {
      std::getline(data, name);
      name = trim(name);
    } else if (ident == "glyph:") {
      sym.load_data_text(data);
      std::getline(data, junk);
    }
  } while (ident != "done" && !data.eof());
// TODO: Flag loading.
  return true;
}
