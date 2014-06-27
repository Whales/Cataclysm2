#include "world_terrain.h"
#include "stringfunc.h"
#include "window.h"
#include "globals.h"
#include "rng.h"
#include <sstream>

Variable_world_terrain::Variable_world_terrain()
{
  total_chance = 0;
}

void Variable_world_terrain::add_terrain(int chance, World_terrain *terrain)
{
  if (!terrain) {
    return;
  }
  World_terrain_chance tmp(chance, terrain);
  add_terrain(tmp);
}

void Variable_world_terrain::add_terrain(World_terrain_chance terrain)
{
  total_chance += terrain.chance;
  ter.push_back(terrain);
}

bool Variable_world_terrain::load_data(std::istream &data, std::string name)
{
  std::string tile_ident;
  std::string terrain_name;
  World_terrain_chance tmp_chance;
  while (data >> tile_ident) {
    tile_ident = no_caps(tile_ident);
    if (tile_ident.substr(0, 2) == "w:") { // It's a weight, i.e. a chance
      tmp_chance.chance = atoi( tile_ident.substr(2).c_str() );
    } else if (tile_ident == "/") { // End of this option
      terrain_name = trim(terrain_name);
      World_terrain* tmpter = WORLD_TERRAIN.lookup_name(terrain_name);
      if (!tmpter) {
        debugmsg("Unknown world terrain '%s' (%s)", terrain_name.c_str(),
                 name.c_str());
        return false;
      }
      tmp_chance.terrain = tmpter;
      add_terrain(tmp_chance);
      tmp_chance.chance  = 10;
      tmp_chance.terrain = NULL;
      terrain_name = "";
    } else { // Otherwise it should be a terrain name
      terrain_name = terrain_name + " " + tile_ident;
    }
  }
// Add the last terrain def to our list
  terrain_name = trim(terrain_name);
  World_terrain* tmpter = WORLD_TERRAIN.lookup_name(terrain_name);
  tmp_chance.terrain = tmpter;
  if (!tmpter) {
    debugmsg("Unknown world terrain '%s' (%s)", terrain_name.c_str(),
             name.c_str());
    return false;
  }
  add_terrain(tmp_chance);
  return true;
}

bool Variable_world_terrain::empty()
{
  return ter.empty();
}

World_terrain* Variable_world_terrain::pick()
{
  if (ter.empty()) {
    return NULL;
  }

  int index = rng(1, total_chance);
  for (int i = 0; i < ter.size(); i++) {
    if (i < 0 || i >= ter.size()) {
      debugmsg("i = %d, ter.size() = %d", i, ter.size());
    }
    index -= ter[i].chance;
    if (index <= 0) {
      return ter[i].terrain;
    }
  }
  return ter.back().terrain;
}

World_terrain::World_terrain()
{
  uid = -1;
  name = "ERROR";
  beach_range = -1;
  road_cost = 100;
  spread_cost = 1;
  sym = glyph();
  for (int i = 0; i < WTF_MAX; i++) {
    flags.push_back(false);
  }
}

std::string World_terrain::get_data_name()
{
  return name;
}

std::string World_terrain::get_name()
{
  if (display_name.empty()) {
    return name;
  }
  return display_name;
}

bool World_terrain::load_data(std::istream &data)
{
  std::string ident, junk;
  do {
    if ( ! (data >> ident) ) {
      return false;
    }

    ident = no_caps(ident);

    if (!ident.empty() && ident[0] == '#') {
// It's a comment
      std::getline(data, junk);

    } else if (ident == "name:") {
      std::getline(data, name);
      name = trim(name);

    } else if (ident == "display_name:") {
      std::getline(data, display_name);
      display_name = trim(display_name);

    } else if (ident == "beach:") {
      std::getline(data, beach_name);
      beach_name = trim(beach_name);

    } else if (ident == "beach_range:") {
      data >> beach_range;
      std::getline(data, junk);

    } else if (ident == "connector:") {
      std::string conn;
      std::getline(data, conn);
      conn = trim(conn);
      connectors.push_back(conn);

    } else if (ident == "road_cost:") {
      data >> road_cost;
      std::getline(data, junk);

    } else if (ident == "spread_cost:") {
      data >> spread_cost;
      std::getline(data, junk);

    } else if (ident == "spread:") {
      if (!spread.load_data(data, name)) {
        return false;
      }

    } else if (ident == "spread_options:") {
      std::getline(data, spread_options);
      spread_options = trim(spread_options);
/*
      std::istringstream spread_data(spread_line);
      if (!spread_options.load_data(spread_data, name)) {
        debugmsg("Error loading spread_options for '%s'", name.c_str());
        return false;
      }
*/

    } else if (ident == "glyph:") {
      sym.load_data_text(data);
      std::getline(data, junk);

    } else if (ident == "flags:") {
      std::string flag_line;
      std::getline(data, flag_line);
      std::istringstream flag_data(flag_line);
      std::string flag_name;
      while (flag_data >> flag_name) {
        World_terrain_flag flag = lookup_world_terrain_flag(flag_name);
        if (flag == WTF_NULL) {
          debugmsg("Unknown world terrain flag '%s' (%s)", flag_name.c_str(),
                   name.c_str());
        }
        flags[flag] = true;
      }

    } else if (ident != "done") {
      debugmsg("Unknown World_terrain property '%s' (%s)",
               ident.c_str(), name.c_str());
    }
  } while (ident != "done" && !data.eof());

// TODO: Flag loading.
  return true;
}

bool World_terrain::has_flag(World_terrain_flag flag)
{
  return flags[flag];
}

World_terrain* make_into_beach(World_terrain* original)
{
  if (original->beach_name.empty()) {
    return original;
  }
  return WORLD_TERRAIN.lookup_name(original->beach_name);
}

World_terrain_flag lookup_world_terrain_flag(std::string name)
{
  name = no_caps(name);
  for (int i = 0; i < WTF_MAX; i++) {
    World_terrain_flag ret = World_terrain_flag(i);
    if ( no_caps( world_terrain_flag_name(ret) ) == name ) {
      return ret;
    }
  }
  return WTF_NULL;
}

std::string world_terrain_flag_name(World_terrain_flag flag)
{
  switch (flag) {
    case WTF_NULL:          return "NULL";
    case WTF_WATER:         return "water";
    case WTF_NO_RIVER:      return "no_river";
    case WTF_SALTY:         return "salty";
    case WTF_BRIDGE:        return "bridge";
    case WTF_NO_ROAD:       return "no_road";
    case WTF_LINE_DRAWING:  return "line_drawing";
    case WTF_RELATIONAL:    return "relational";
    case WTF_SHOP:          return "shop";
    case WTF_FACE_ROAD:     return "face_road";
    case WTF_ROAD:          return "road";
    case WTF_MAX:           return "BUG - WTF_MAX";
    default:                return "BUG - Unnamed World_terrain_flag";
  }
  return "BUG - Escaped switch in world_terrain_flag_name()";
}
