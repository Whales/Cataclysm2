#include "mapgen_spec.h"

bool Mapgen_spec::load_data(std::istream &data)
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
    } else if (ident == "type:") {
      std::getline(data, terrain_name);
      terrain_name = trim(terrain_name);
    } else if (ident == "tile:") {
      std::string tile_line;
      std::stringstream tile_data;
      std::getline(data, tile_line);
      tile_data << tile_line;

      std::string symbols;
      std::string tile_ident;
      bool reading_symbols = true; // We start out reading symbols!

      Terrain_chance tmp_chance(10, NULL);
      Variable_terrain tmp_var;

      while (tile_data >> tile_ident) {
        if (reading_symbols) {
          if (tile_ident == "=") {
            reading_symbols = false;
          } else {
            symbols += tile_ident;
          }
        } else { // Not reading in symbols
          tile_ident = no_caps(tile_ident);  // other stuff isn't case-sensitive
          if (tile_info.substr(0, 2) == "w:") { // It's a weight, e.g. a chance
            tile_data >> tmp_chance.
            tmp_chance.chance = 
