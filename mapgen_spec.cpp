#include <stdlib.h>
#include <sstream>
#include "mapgen_spec.h"
#include "window.h"
#include "globals.h"
#include "stringfunc.h"

void Variable_terrain::add_terrain(int chance, Terrain *terrain)
{
  Terrain_chance tmp(chance, terrain);
  ter.push_back(tmp);
}

void Variable_terrain::add_terrain(Terrain_chance terrain)
{
  ter.push_back(terrain);
}

void Variable_terrain::load_data(std::istream &data, std::string name)
{
  std::string tile_ident;
  Terrain_chance tmp_chance;
  while (data >> tile_ident) {
    tile_ident = no_caps(tile_ident);  // other stuff isn't case-sensitive
    if (tile_ident.substr(0, 2) == "w:") { // It's a weight, e.g. a chance
      tmp_chance.chance = atoi( tile_ident.substr(2).c_str() );
    } else if (tile_ident == "/") { // End of this option
      add_terrain(tmp_chance);
      tmp_chance.chance  = 10;
      tmp_chance.terrain = NULL;
    } else { // Otherwise, it should be a terrain name
      Terrain* tmpter = TERRAIN.lookup_name(tile_ident);
      if (!tmpter) {
        debugmsg("Unknown terrain '%s' (%s)", tile_ident.c_str(),
                 name.c_str());
      }
      tmp_chance.terrain = tmpter;
    }
  }
// Add the last terrain def to our list, if the terrain is valid
  if (tmp_chance.terrain) {
    add_terrain(tmp_chance);
  }
}

Mapgen_spec::Mapgen_spec()
{
  name = "unknown";
  for (int x = 0; x < MAPGEN_SIZE; x++) {
    for (int y = 0; y < MAPGEN_SIZE; y++) {
      terrain[x][y] = 0;
    }
  }
}

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
    } else if (ident == "base_terrain:") {
      std::string tile_line;
      std::getline(data, tile_line);
      std::istringstream tile_data(tile_line);
      base_terrain.load_data(tile_data, name);
    } else if (ident == "type:") {
      std::getline(data, terrain_name);
      terrain_name = trim(terrain_name);
    } else if (ident == "tile:") {
      std::string tile_line;
      std::getline(data, tile_line);
      std::istringstream tile_data(tile_line);

      std::string symbols;
      std::string tile_ident;
      bool reading_symbols = true; // We start out reading symbols!

      Terrain_chance tmp_chance(10, NULL);
      Variable_terrain tmp_var;

      while (tile_data >> tile_ident && reading_symbols) {
        if (reading_symbols) {
          if (tile_ident == "=") {
            reading_symbols = false;
          } else {
            symbols += tile_ident;
          }
        }
      }
      tmp_var.load_data(tile_data, name);
// For every character in symbols, map that char to tmp_var
      for (int i = 0; i < symbols.length(); i++) {
        char ch = symbols[i];
        if (terrain_defs.count(ch) != 0) {
          debugmsg("Tried to map %c - already in use (%s)", ch, name.c_str());
        } else {
          terrain_defs[ch] = tmp_var;
        }
      }
// End if (ident == "tile:") block
    } else if (ident == "map:") {
      std::string mapchars;
      std::getline(data, mapchars);
      int line = 0;
      do {
        std::getline(data, mapchars);
        if (mapchars != "endmap" && mapchars.length() != MAPGEN_SIZE) {
          debugmsg("Bad map width '%s' (%s)", mapchars.c_str(), name.c_str());
        }
        for (int i = 0; i < mapchars.length(); i++) {
          terrain[i][line] = mapchars[i];
        }
        line++;
      } while (mapchars != "endmap" && line < MAPGEN_SIZE);
      if (line != MAPGEN_SIZE) {
        debugmsg("Bad map height %d (%s)", line, name.c_str());
      }
    }
  } while (ident != "done");
  return true;
}
