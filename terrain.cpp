#include "terrain.h"
#include "stringfunc.h"
#include "files.h"
#include "window.h"

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
  std::string ident, junk;
  while (ident != "done" && !data.eof()) {
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
    } else if (ident == "movecost:") {
      data >> movecost;
      std::getline(data, junk);
    }
  }
// TODO: Flag loading.
  return true;
}
