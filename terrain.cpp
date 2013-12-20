#include <sstream>
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
  for (int i = 0; i < TF_MAX; i++) {
    flags.push_back(false);
  }
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
    } else if (ident == "flags:") {
      std::getline(data, junk);
      std::istringstream flagdata(junk);
      std::string flagname;
      while (flagdata >> flagname) {
        flags[ lookup_terrain_flag(flagname) ] = true;
      }
    }
  }
// TODO: Flag loading.
  return true;
}

Terrain_flag lookup_terrain_flag(std::string name)
{
  name = no_caps(name);
  for (int i = 0; i < TF_MAX; i++) {
    if ( terrain_flag_name( Terrain_flag(i) ) == name) {
      return Terrain_flag(i);
    }
  }
  return TF_NULL;
}

// Note: ALL terrain flag names must be all lowercase!
std::string terrain_flag_name(Terrain_flag flag)
{
  switch (flag) {
    case TF_NULL:     return "null";
    case TF_OPAQUE:   return "opaque";
    default:          return "ERROR"; // All caps means it'll never be used
  }
  return "ERROR";
}
