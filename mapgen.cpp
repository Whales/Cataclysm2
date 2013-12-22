#include <stdlib.h>
#include <sstream>
#include <fstream>
#include "mapgen.h"
#include "window.h"
#include "globals.h"
#include "stringfunc.h"
#include "rng.h"

Variable_terrain::Variable_terrain()
{
  total_chance = 0;
}

void Variable_terrain::add_terrain(int chance, Terrain *terrain)
{
  Terrain_chance tmp(chance, terrain);
  add_terrain(tmp);
}

void Variable_terrain::add_terrain(Terrain_chance terrain)
{
  total_chance += terrain.chance;
  ter.push_back(terrain);
}

void Variable_terrain::load_data(std::istream &data, std::string name)
{
  std::string tile_ident;
  Terrain_chance tmp_chance;
  bool okay_to_push_null = false;
  while (data >> tile_ident) {
    tile_ident = no_caps(tile_ident);  // other stuff isn't case-sensitive
    if (tile_ident.substr(0, 2) == "w:") { // It's a weight, e.g. a chance
      tmp_chance.chance = atoi( tile_ident.substr(2).c_str() );
    } else if (tile_ident == "/") { // End of this option
      add_terrain(tmp_chance);
      tmp_chance.chance  = 10;
      tmp_chance.terrain = NULL;
      okay_to_push_null = false;
    } else if (tile_ident == "nothing") {
// Should only be used by adjacent generators...
      tmp_chance.terrain = NULL;
      okay_to_push_null = true;
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
  if (tmp_chance.terrain || okay_to_push_null) {
    add_terrain(tmp_chance);
  }
}

Terrain* Variable_terrain::pick()
{
  if (ter.empty()) {
    return NULL;
  }
  int index = rng(1, total_chance);
  for (int i = 0; i < ter.size(); i++) {
    index -= ter[i].chance;
    if (index <= 0) {
      return ter[i].terrain;
    }
  }
  return ter.back().terrain;
}

Item_area::Item_area()
{
  total_chance = 0;
}

void Item_area::add_item(int chance, Itemtype* itemtype)
{
  Itemtype_chance tmp(chance, itemtype);
  add_item(tmp);
}

void Item_area::add_item(Itemtype_chance itemtype)
{
  itemtypes.push_back(itemtype);
  total_chance += itemtype.chance;
}

void Item_area::add_point(int x, int y)
{
  locations.push_back( Point(x, y) );
}

void Item_area::load_data(std::istream &data, std::string name)
{
  std::string item_ident;
  Itemtype_chance tmp_chance;
  while (data >> item_ident) {
    item_ident = no_caps(item_ident);  // other stuff isn't case-sensitive
    if (item_ident.substr(0, 2) == "w:") { // It's a weight, e.g. a chance
      tmp_chance.chance = atoi( item_ident.substr(2).c_str() );
    } else if (item_ident == "/") { // End of this option
      add_item(tmp_chance);
      tmp_chance.chance = 10;
      tmp_chance.item   = NULL;
    } else { // Otherwise, it should be a item name
      Itemtype* tmpitem = ITEMTYPES.lookup_name(item_ident);
      if (!tmpitem) {
        debugmsg("Unknown item '%s' (%s)", item_ident.c_str(),
                 name.c_str());
      }
      tmp_chance.item = tmpitem;
    }
  }
// Add the last item def to our list, if the item is valid
  if (tmp_chance.item) {
    add_item(tmp_chance);
  }
}

bool Item_area::place_item()
{
  if (overall_chance >= 100 || overall_chance <= 0) {
    return false;
  }
  return (rng(1, 100) <= overall_chance);
}

Itemtype* Item_area::pick_type()
{
  if (itemtypes.empty()) {
    return NULL;
  }
  int index = rng(1, total_chance);
  for (int i = 0; i < itemtypes.size(); i++) {
    index -= itemtypes[i].chance;
    if (index <= 0) {
      return itemtypes[i].item;
    }
  }
  return itemtypes.back().item;
}

Point Item_area::pick_location()
{
  if (locations.empty()) {
    return Point(-1, -1);
  }
  int index = rng(0, locations.size() - 1);
  return locations[index];
}

Mapgen_spec::Mapgen_spec()
{
  name = "unknown";
  weight = 100;
  is_adjacent = false;
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
    if (!ident.empty() && ident[0] == '#') {
      std::getline(data, junk); // It's a comment - clear the line
    } else if (ident == "name:") {
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
    } else if (ident == "adjacent") {
      is_adjacent = true;
      std::getline(data, junk);
    } else if (ident == "weight:") {
      data >> weight;
      std::getline(data, junk);
    } else if (ident == "tile:") {
      std::string tile_line;
      std::getline(data, tile_line);
      std::istringstream tile_data(tile_line);

      std::string symbols;
      std::string tile_ident;
      bool reading_symbols = true; // We start out reading symbols!

      Variable_terrain tmp_var;

      while (reading_symbols && tile_data >> tile_ident) {
        if (tile_ident == "=") {
          reading_symbols = false;
        } else {
          symbols += tile_ident;
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
    } else if (ident == "items:") {
      Item_area tmp_area;

      data >> tmp_area.overall_chance;

      std::string item_line;
      std::getline(data, item_line);
      std::istringstream item_data(item_line);

      std::string symbols;
      std::string item_ident;
      bool reading_symbols = true; // We start out reading symbols!

      while (reading_symbols && item_data >> item_ident) {
        if (item_ident == "=") {
          reading_symbols = false;
        } else {
          symbols += item_ident;
        }
      }
      tmp_area.load_data(item_data, name);
// For every character in symbols, map that char to tmp_var
      for (int i = 0; i < symbols.length(); i++) {
        char ch = symbols[i];
        if (item_defs.count(ch) != 0) {
          debugmsg("Tried to map %c - already in use (%s)", ch, name.c_str());
        } else {
          item_defs[ch] = tmp_area;
        }
      }
// End if (ident == "items:") block
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
          if (item_defs.count(mapchars[i]) != 0) {
            item_defs[mapchars[i]].add_point(i, line);
          }
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

Terrain* Mapgen_spec::pick_terrain(int x, int y)
{
  if (x < 0 || x >= MAPGEN_SIZE || y < 0 || y >= MAPGEN_SIZE) {
    return (is_adjacent ? NULL : base_terrain.pick());
  }
  char key = terrain[x][y];
  if (terrain_defs.count(key) == 0) {
    return (is_adjacent ? NULL : base_terrain.pick());
  }
  return terrain_defs[key].pick();
}

Mapgen_spec Mapgen_spec::random_rotate()
{
  Direction dir = Direction(rng(DIR_NORTH, DIR_WEST));
  return rotate(dir);
}

Mapgen_spec Mapgen_spec::rotate(Direction dir)
{
  Mapgen_spec ret;
  ret.uid = uid;
// TODO: Tell us how it was rotated!
  ret.name = name + " [rotated]";
  ret.terrain_name = terrain_name;
  ret.is_adjacent = is_adjacent;  // Don't think we need this but y'know
  ret.weight = weight;            // Ditto
  ret.terrain_defs = terrain_defs;
  ret.item_defs = item_defs;
  ret.base_terrain = base_terrain;
  for (int x = 0; x < MAPGEN_SIZE; x++) {
    for (int y = 0; y < MAPGEN_SIZE; y++) {
      int tx, ty;
      switch (dir) {
        case DIR_NORTH: tx = x;                   ty = y; break;
        case DIR_EAST:  tx = MAPGEN_SIZE - y - 1; ty = x; break;
        case DIR_SOUTH: tx = MAPGEN_SIZE - x - 1; ty = MAPGEN_SIZE - y - 1;
                        break;
        case DIR_WEST:  tx = y; ty = MAPGEN_SIZE - x - 1; break;
      }
      ret.terrain[tx][ty] = terrain[x][y];
    }
  }
  return ret;
}

Mapgen_spec_pool::Mapgen_spec_pool()
{
  next_uid = 0;
}

Mapgen_spec_pool::~Mapgen_spec_pool()
{
  for (std::list<Mapgen_spec*>::iterator it = instances.begin();
       it != instances.end();
       it++) {
    delete (*it);
  }
}

bool Mapgen_spec_pool::load_from(std::string filename)
{
  std::ifstream fin;
  fin.open(filename.c_str());
  if (!fin.is_open()) {
    debugmsg("Failed to open '%s'", filename.c_str());
    return false;
  }

  while (!fin.eof()) {
    if (!load_element(fin)) {
      return false;
    }
  }
  return true;
}

bool Mapgen_spec_pool::load_element(std::istream &data)
{
  Mapgen_spec* tmp = new Mapgen_spec;
  if (!tmp->load_data(data)) {
    return false;
  }
  tmp->uid = next_uid;
  instances.push_back(tmp);
  uid_map[next_uid] = tmp;
  name_map[tmp->name] = tmp;
  std::map<std::string,std::vector<Mapgen_spec*> > *name_map;
  std::map<std::string,int> *chance_map;
  std::map<World_terrain*,std::vector<Mapgen_spec*> > *ptr_map;
  std::map<World_terrain*,int> *ptr_chance_map;
  if (tmp->is_adjacent) {
    name_map = &adjacent_name_map;
    chance_map = &adjacent_name_total_chance;
    ptr_map = &adjacent_ptr_map;
    ptr_chance_map = &adjacent_ptr_total_chance;
  } else {
    name_map = &terrain_name_map;
    chance_map = &terrain_name_total_chance;
    ptr_map = &terrain_ptr_map;
    ptr_chance_map = &terrain_ptr_total_chance;
  }
// Push into the World_terrain* maps
  World_terrain* wter = WORLD_TERRAIN.lookup_name(tmp->terrain_name);
  if (wter) {
    if (ptr_map->count(wter) == 0) {
      std::vector<Mapgen_spec*> tmpvec;
      tmpvec.push_back(tmp);
      (*ptr_map)[wter] = tmpvec;
      (*ptr_chance_map)[wter] = tmp->weight;
    } else {
      (*ptr_map)[wter].push_back(tmp);
      (*ptr_chance_map)[wter] += tmp->weight;
    }
  }
// Push into the name-based maps
  if (name_map->count(tmp->terrain_name) == 0) {
    std::vector<Mapgen_spec*> tmpvec;
    tmpvec.push_back(tmp);
    (*name_map)[tmp->terrain_name] = tmpvec;
    (*chance_map)[tmp->terrain_name] = tmp->weight;
  } else {
    (*name_map)[tmp->terrain_name].push_back(tmp);
    (*chance_map)[tmp->terrain_name] += tmp->weight;
  }
  return true;
}

Mapgen_spec* Mapgen_spec_pool::lookup_uid(int uid)
{
  if (uid_map.count(uid) == 0) {
    return NULL;
  }
  return uid_map[uid];
}

Mapgen_spec* Mapgen_spec_pool::lookup_name(std::string name)
{
  if (name_map.count(name) == 0) {
    return NULL;
  }
  return name_map[name];
}

std::vector<Mapgen_spec*>
Mapgen_spec_pool::lookup_terrain_name(std::string name)
{
  if (terrain_name_map.count(name) == 0) {
    std::vector<Mapgen_spec*> tmp;
    return tmp;
  }
  return terrain_name_map[name];
}

Mapgen_spec* Mapgen_spec_pool::random_for_terrain(std::string name)
{
  if (terrain_name_map.count(name) == 0) {
    return NULL;
  }
  int index = rng(1, terrain_name_total_chance[name]);
  std::vector<Mapgen_spec*> *vec = &(terrain_name_map[name]);
  for (int i = 0; i < vec->size(); i++) {
    index -= (*vec)[i]->weight;
    if (index <= 0) {
      return (*vec)[i];
    }
  }
  return vec->back();
}

std::vector<Mapgen_spec*>
Mapgen_spec_pool::lookup_terrain_ptr(World_terrain* ptr)
{
  if (!ptr || terrain_ptr_map.count(ptr) == 0) {
    std::vector<Mapgen_spec*> ret;
    return ret;
  }
  return terrain_ptr_map[ptr];
}

Mapgen_spec*
Mapgen_spec_pool::random_for_terrain(World_terrain* ptr)
{
  if (!ptr || terrain_ptr_map.count(ptr) == 0) {
    return NULL;
  }
  int index = rng(1, terrain_ptr_total_chance[ptr]);
  std::vector<Mapgen_spec*> *vec = &(terrain_ptr_map[ptr]);
  for (int i = 0; i < vec->size(); i++) {
    index -= (*vec)[i]->weight;
    if (index <= 0) {
      return (*vec)[i];
    }
  }
  return vec->back();
}

std::vector<Mapgen_spec*>
Mapgen_spec_pool::lookup_adjacent_name(std::string name)
{
  if (adjacent_name_map.count(name) == 0) {
    std::vector<Mapgen_spec*> tmp;
    return tmp;
  }
  return adjacent_name_map[name];
}

Mapgen_spec* Mapgen_spec_pool::random_adjacent_to(std::string name)
{
  if (adjacent_name_map.count(name) == 0) {
    return NULL;
  }
  int index = rng(1, adjacent_name_total_chance[name]);
  std::vector<Mapgen_spec*> *vec = &(adjacent_name_map[name]);
  for (int i = 0; i < vec->size(); i++) {
    index -= (*vec)[i]->weight;
    if (index <= 0) {
      return (*vec)[i];
    }
  }
  return vec->back();
}

std::vector<Mapgen_spec*>
Mapgen_spec_pool::lookup_adjacent_ptr(World_terrain* ptr)
{
  if (!ptr || adjacent_ptr_map.count(ptr) == 0) {
    std::vector<Mapgen_spec*> ret;
    return ret;
  }
  return adjacent_ptr_map[ptr];
}

Mapgen_spec* Mapgen_spec_pool::random_adjacent_to(World_terrain *ptr)
{
  if (!ptr || adjacent_ptr_map.count(ptr) == 0) {
    return NULL;
  }
  int index = rng(1, adjacent_ptr_total_chance[ptr]);
  std::vector<Mapgen_spec*> *vec = &(adjacent_ptr_map[ptr]);
  for (int i = 0; i < vec->size(); i++) {
    index -= (*vec)[i]->weight;
    if (index <= 0) {
      return (*vec)[i];
    }
  }
  return vec->back();
}

int Mapgen_spec_pool::size()
{
  return instances.size();
}
