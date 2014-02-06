#include "mapgen.h"
#include "window.h"
#include "globals.h"
#include "stringfunc.h"
#include "rng.h"
#include <stdlib.h>
#include <sstream>
#include <fstream>

Variable_terrain::Variable_terrain()
{
  total_chance = 0;
  lock = false;
  choice = NULL;
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

bool Variable_terrain::load_data(std::istream &data, std::string name,
                                 bool allow_nothing)
{
  std::string tile_ident;
  std::string terrain_name;
  Terrain_chance tmp_chance;
  while (data >> tile_ident) {
    tile_ident = no_caps(tile_ident);  // other stuff isn't case-sensitive
    if (tile_ident.substr(0, 2) == "w:") { // It's a weight, i.e. a chance
      tmp_chance.chance = atoi( tile_ident.substr(2).c_str() );
    } else if (tile_ident == "/") { // End of this option
      terrain_name = trim(terrain_name);
      Terrain* tmpter = TERRAIN.lookup_name(terrain_name);
      if (terrain_name != "nothing" && !tmpter) {
        debugmsg("Unknown terrain '%s' (%s)", terrain_name.c_str(),
                 name.c_str());
        return false;
      }
      tmp_chance.terrain = tmpter;
      add_terrain(tmp_chance);
      tmp_chance.chance  = 10;
      tmp_chance.terrain = NULL;
      terrain_name = "";
    } else { // Otherwise, it should be a terrain name
      terrain_name = terrain_name + " " + tile_ident;
    }
  }
// Add the last terrain def to our list, if the terrain is valid
  terrain_name = trim(terrain_name);
  Terrain* tmpter = TERRAIN.lookup_name(terrain_name);
  if (terrain_name != "nothing" && !tmpter) {
    debugmsg("Unknown terrain '%s' (%s)", terrain_name.c_str(),
             name.c_str());
    return false;
  }
  tmp_chance.terrain = tmpter;
  add_terrain(tmp_chance);
  return true;
}

void Variable_terrain::prepare()
{
  if (!lock) {
    return;
  }

  choice = pick(true);
}

Terrain* Variable_terrain::pick(bool refresh_choice)
{
  if (ter.empty()) {
    return NULL;
  }
  if (!refresh_choice && lock && choice) {
    return choice;
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

void Item_area::add_item(int chance, Item_type* item_type)
{
  Item_type_chance tmp(chance, item_type);
  add_item(tmp);
}

void Item_area::add_item(Item_type_chance item_type)
{
  item_types.push_back(item_type);
  total_chance += item_type.chance;
}

void Item_area::set_group(Item_group *group)
{
  if (!group) {
    return;
  }
  total_chance = group->total_chance;
  item_types   = group->item_types;
}

void Item_area::clear_points()
{
  locations.clear();
}

void Item_area::add_point(int x, int y)
{
  locations.push_back( Point(x, y) );
}

void Item_area::load_data(std::istream &data, std::string name)
{
  std::string item_ident;
  std::string item_name;
  Item_type_chance tmp_chance;
  while (data >> item_ident) {
    item_ident = no_caps(item_ident);  // other stuff isn't case-sensitive
    if (item_ident.substr(0, 2) == "w:") { // It's a weight, e.g. a chance
      tmp_chance.chance = atoi( item_ident.substr(2).c_str() );
    } else if (item_ident == "/") { // End of this option
      item_name = trim(item_name);
      Item_type* tmpitem = ITEM_TYPES.lookup_name(item_name);
      if (!tmpitem) {
        debugmsg("Unknown item '%s' (%s)", item_ident.c_str(),
                 name.c_str());
      }
      tmp_chance.item = tmpitem;
      add_item(tmp_chance);
      tmp_chance.chance = 10;
      tmp_chance.item   = NULL;
      item_name = "";
    } else { // Otherwise, it should be a item name
      item_name = item_name + " " + item_ident;
    }
  }
// Add the last item def to our list, if the item is valid
  item_name = trim(item_name);
  Item_type* tmpitem = ITEM_TYPES.lookup_name(item_name);
  if (!tmpitem) {
    debugmsg("Unknown item '%s' (%s)", item_name.c_str(),
             name.c_str());
  }
  tmp_chance.item = tmpitem;
  add_item(tmp_chance);
}

bool Item_area::place_item()
{
  if (overall_chance >= 100 || overall_chance <= 0) {
    return false;
  }
  return (rng(1, 100) <= overall_chance);
}

Item_type* Item_area::pick_type()
{
  if (item_types.empty()) {
    debugmsg("Using NULL item");
    return NULL;
  }
  int index = rng(1, total_chance);
  for (int i = 0; i < item_types.size(); i++) {
    index -= item_types[i].chance;
    if (index <= 0) {
      return item_types[i].item;
    }
  }
  return item_types.back().item;
}

Point Item_area::pick_location()
{
  if (locations.empty()) {
    return Point(-1, -1);
  }
  int index = rng(0, locations.size() - 1);
  return locations[index];
}

Item_group::Item_group()
{
  name = "unknown";
  uid = -1;
  total_chance = 0;
}

void Item_group::assign_uid(int id)
{
  uid = id;
}

std::string Item_group::get_data_name()
{
  return name;
}

bool Item_group::load_data(std::istream &data)
{
  std::string ident;

  while (ident != "done" && !data.eof()) {
    if ( ! (data >> ident) ) {
      return false;
    }
    ident = no_caps(ident);

    if (ident == "name:") {
      std::getline(data, name);

    } else if (ident == "items:") {
      std::string item_ident;
      std::string item_name;
      std::string item_line;
      std::getline(data, item_line);
      std::istringstream item_ss(item_line);
      Item_type_chance tmp_chance;
      while (item_ss >> item_ident) {
        item_ident = no_caps(item_ident);  // other stuff isn't case-sensitive
        if (item_ident.substr(0, 2) == "w:") { // It's a weight, e.g. a chance
          tmp_chance.chance = atoi( item_ident.substr(2).c_str() );
        } else if (item_ident == "/") { // End of this option
          item_name = trim(item_name);
          Item_type* tmpitem = ITEM_TYPES.lookup_name(item_name);
          if (!tmpitem) {
            debugmsg("Unknown item '%s' (%s)", item_ident.c_str(),
                     name.c_str());
          }
          tmp_chance.item = tmpitem;
          add_item(tmp_chance);
          tmp_chance.chance = 10;
          tmp_chance.item   = NULL;
          item_name = "";
        } else { // Otherwise, it should be a item name
          item_name = item_name + " " + item_ident;
        }
      }
    // Add the last item def to our list, if the item is valid
      item_name = trim(item_name);
      Item_type* tmpitem = ITEM_TYPES.lookup_name(item_name);
      if (!tmpitem) {
        debugmsg("Unknown item '%s' (%s)", item_name.c_str(),
                 name.c_str());
      }
      tmp_chance.item = tmpitem;
      add_item(tmp_chance);

    } else if (ident != "done") {
      debugmsg("Unknown identifier '%s' (%s)", ident.c_str(), name.c_str());
      return false;
    }
  }
  return true;
}

void Item_group::add_item(int chance, Item_type* item_type)
{
  Item_type_chance tmp(chance, item_type);
  add_item(tmp);
}

void Item_group::add_item(Item_type_chance item_type)
{
  item_types.push_back(item_type);
  total_chance += item_type.chance;
}


Tile_substitution::Tile_substitution()
{
  total_chance = 0;
  selected = '.';
}

void Tile_substitution::add_result(int chance, char result)
{
  Subst_chance tmp(chance, result);
  add_result(tmp);
}

void Tile_substitution::add_result(Subst_chance chance)
{
  chances.push_back(chance);
  total_chance += chance.chance;
}

void Tile_substitution::load_data(std::istream &data, std::string name)
{
  std::string ident;
  int weight = 10;
  std::string characters;
  while (data >> ident) {
    if (ident.substr(0, 2) == "w:" || ident.substr(0, 2) == "W:") {
      weight = atoi( ident.substr(2).c_str() );
    } else if (ident == "/") {
      for (int i = 0; i < characters.length(); i++) {
        add_result(weight, characters[i]);
      }
      weight = 10;
      characters = "";
    } else { // Should be a char or list of chars
      characters += ident;
    }
  }
// Add the last one to our list
  for (int i = 0; i < characters.length(); i++) {
    add_result(weight, characters[i]);
  }
}

void Tile_substitution::make_selection()
{
  if (chances.empty()) {
    selected = '.';
    return;
  }

  int index = rng(1, total_chance);
  for (int i = 0; i < chances.size(); i++) {
    index -= chances[i].chance;
    if (index <= 0) {
      selected = chances[i].result;
      return;
    }
  }
  selected = chances.back().result;
}

char Tile_substitution::current_selection()
{
  return selected;
}

Mapgen_spec::Mapgen_spec()
{
  name = "unknown";
  weight = 100;
  is_adjacent = false;
  num_neighbors = 0;
  z_level = 0;
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
      if (MAPGEN_SPECS.lookup_name(name)) {
        debugmsg("Loaded '%s' but it already exists!", name.c_str());
        return false;
      }

    } else if (ident == "subname:") {
      std::getline(data, subname);
      subname = trim(subname);

    } else if (ident == "type:") {
      std::getline(data, terrain_name);
      terrain_name = trim(terrain_name);

    } else if (ident == "adjacent") {
      is_adjacent = true;
      std::getline(data, junk);

    } else if (ident == "neighbors:") {
      data >> num_neighbors;
      std::getline(data, junk);

    } else if (ident == "level:" || ident == "floor:") {
      data >> z_level;
      std::getline(data, junk);

    } else if (ident == "base_terrain:") {
      std::string tile_line;
      std::getline(data, tile_line);
      std::istringstream tile_data(tile_line);
      base_terrain.load_data(tile_data, name, is_adjacent);

    } else if (ident == "weight:") {
      data >> weight;
      std::getline(data, junk);

    } else if (ident == "tile:" || ident == "tile_group:") {
      std::string tile_line;
      std::getline(data, tile_line);
      std::istringstream tile_data(tile_line);

      std::string symbols;
      std::string tile_ident;
      bool reading_symbols = true; // We start out reading symbols!

      Variable_terrain tmp_var;
      if (ident == "tile_group:") {
        tmp_var.lock = true;
      }

      while (reading_symbols && tile_data >> tile_ident) {
        if (tile_ident == "=") {
          reading_symbols = false;
        } else {
          symbols += tile_ident;
        }
      }
      if (!tmp_var.load_data(tile_data, name)) {
        debugmsg("Failed to load Variable_terrain (%s)", name.c_str());
        return false;
      }
// For every character in symbols, map that char to tmp_var
      for (int i = 0; i < symbols.length(); i++) {
        char ch = symbols[i];
        if (terrain_defs.count(ch) != 0) {
          debugmsg("Tried to map %c - already in use (%s)", ch, name.c_str());
        } else {
          terrain_defs[ch] = tmp_var;
        }
      }

// End if (ident == "tile:" || ident == "tile_group:") block

    } else if (ident == "subst:" || ident == "substitution:") {
      std::string subst_line;
      std::getline(data, subst_line);
      std::istringstream subst_data(subst_line);

      std::string symbols;
      std::string subst_ident;
      bool reading_symbols = true; // We start out reading symbols!

      Tile_substitution tmp_subst;

      while (reading_symbols && subst_data >> subst_ident) {
        if (subst_ident == "=") {
          reading_symbols = false;
        } else {
          symbols += subst_ident;
        }
      }
      tmp_subst.load_data(subst_data, name);
// For every character in symbols, map that char to tmp_var
      for (int i = 0; i < symbols.length(); i++) {
        char ch = symbols[i];
        if (substitutions.count(ch) != 0) {
          debugmsg("Tried to map substitution %c - already in use (%s)",
                   ch, name.c_str());
        } else {
          substitutions[ch] = tmp_subst;
        }
      }

    } else if (ident == "shuffle:") {
      std::string shuffle_line;
      std::getline(data, shuffle_line);
      std::istringstream shuffle_data(shuffle_line);

      std::string symbols;
      std::string shuffle_ident;

      while (shuffle_data >> shuffle_ident) {
        symbols += shuffle_ident;
      }
// For every character in symbols, map that char to results
      shuffles.push_back(symbols);

// End of (ident == "subst:" || ident == "substitution:") block
    } else if (ident == "item_group:") {
      Item_area tmp_area;

      data >> tmp_area.overall_chance;
      if (tmp_area.overall_chance < 1) {
        debugmsg("Item chance of '%d' corrected to 1 (%s)",
                 tmp_area.overall_chance, name.c_str());
      } else if (tmp_area.overall_chance > 99) {
        debugmsg("Item chance of '%d' corrected to 99 (%s)",
                 tmp_area.overall_chance, name.c_str());
      }

      std::string symbols;
      std::string item_ident;
      bool reading_symbols = true; // We start out reading symbols!

      while (reading_symbols && data >> item_ident) {
        if (item_ident == "=") {
          reading_symbols = false;
        } else {
          symbols += item_ident;
        }
      }
      std::string group_name;
      std::getline(data, group_name);

      Item_group* group = ITEM_GROUPS.lookup_name(group_name);
      if (!group) {
        debugmsg("Unknown item group '%s' (%s)", group_name.c_str(),
                 name.c_str());
      } else {
        tmp_area.set_group(group);
      }
// For every character in symbols, map that char to tmp_var
      for (int i = 0; i < symbols.length(); i++) {
        char ch = symbols[i];
        if (item_defs.count(ch) != 0) {
          debugmsg("Tried to map %c - already in use (%s)", ch, name.c_str());
        } else {
          item_defs[ch] = tmp_area;
        }
      }

    } else if (ident == "items:") {
      Item_area tmp_area;

      data >> tmp_area.overall_chance;

      if (tmp_area.overall_chance < 1) {
        debugmsg("Item chance of '%d' corrected to 1 (%s)",
                 tmp_area.overall_chance, name.c_str());
      } else if (tmp_area.overall_chance > 99) {
        debugmsg("Item chance of '%d' corrected to 99 (%s)",
                 tmp_area.overall_chance, name.c_str());
      }

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
/*
          if (item_defs.count(mapchars[i]) != 0) {
            item_defs[mapchars[i]].add_point(i, line);
          }
*/
        }
        line++;
      } while (mapchars != "endmap" && line < MAPGEN_SIZE);
      if (line != MAPGEN_SIZE) {
        debugmsg("Bad map height %d (%s)", line, name.c_str());
      }
    } else if (ident != "done" && ident != "endmap") {
      debugmsg("Unknown Mapgen_spec property '%s' (%s)",
               ident.c_str(), name.c_str());
    }
  } while (ident != "done" && !data.eof());
  return true;
}

Terrain* Mapgen_spec::pick_terrain(int x, int y)
{
  if (x < 0 || x >= MAPGEN_SIZE || y < 0 || y >= MAPGEN_SIZE) {
    return (is_adjacent ? NULL : base_terrain.pick());
  }
  char key = prepped_terrain[x][y];
  if (terrain_defs.count(key) == 0) {
    return (is_adjacent ? NULL : base_terrain.pick());
  }
  return terrain_defs[key].pick();
}

void Mapgen_spec::prepare(World_terrain* world_ter[5])
{
// Prep terrain_defs; if they're grouped, this picks and "locks in" the terrain
  for (std::map<char,Variable_terrain>::iterator it = terrain_defs.begin();
       it != terrain_defs.end();
       it++) {
    (it->second).prepare();
  }
// Shuffle before substitutions.
  std::map<char,char> post_shuffles;
  for (std::list<std::string>::iterator it = shuffles.begin();
       it != shuffles.end();
       it++) {
    std::string from_list = (*it);
    std::string shuffle_list;
// Shuffle the std::string
    while (!from_list.empty()) {
      int index = rng(0, from_list.size() - 1);
      shuffle_list.push_back(from_list[index]);
      from_list.erase(from_list.begin() + index);
    }
    for (int i = 1; i < shuffle_list.size(); i++) {
      char from = shuffle_list[i - 1];
      char to = shuffle_list[i];
      post_shuffles[from] = to;
    }
    if (shuffle_list.size() > 1) {
      post_shuffles[ shuffle_list[shuffle_list.size() - 1] ] = shuffle_list[0];
    }
  }
// Do any character substitutions; this also sets up prepped_terrain
  for (std::map<char, Tile_substitution>::iterator it = substitutions.begin();
       it != substitutions.end();
       it++) {
    (it->second).make_selection();
  }
  for (int x = 0; x < MAPGEN_SIZE; x++) {
    for (int y = 0; y < MAPGEN_SIZE; y++) {
      char ch = terrain[x][y];
      bool shuffled = false;
      if (post_shuffles.count(ch) > 0) {
        char newch = post_shuffles[ch];
        ch = newch;
        prepped_terrain[x][y] = newch;
        shuffled = true;
      }
      if (substitutions.count(ch) > 0) {
        char newch = substitutions[ch].current_selection();
        prepped_terrain[x][y] = newch;
      } else if (!shuffled) {
        prepped_terrain[x][y] = ch;
      }
    }
  }

// Rotate as required.
// TODO: Allow for a "norotate" flag?
// If we're a relational map, rotate based on neighbors...
  if (!is_adjacent && world_ter[0] && world_ter[0]->has_flag(WTF_RELATIONAL)) {
    std::vector<bool> neighbor;
    neighbor.push_back(false);
    for (int i = 1; i < 5; i++) {
      neighbor.push_back( (world_ter[i] == world_ter[0]) );
    }
    if (num_neighbors == 1) {
      if (neighbor[DIR_NORTH]) {
      } else if (neighbor[DIR_EAST]) {
        rotate(DIR_EAST);
      } else if (neighbor[DIR_SOUTH]) {
        rotate(DIR_SOUTH);
      } else if (neighbor[DIR_WEST]) {
        rotate(DIR_WEST);
      }
    } else if (num_neighbors == 2) {
      if (neighbor[DIR_EAST] && neighbor[DIR_SOUTH]) {
        rotate(DIR_EAST);
      } else if (neighbor[DIR_SOUTH] && neighbor[DIR_WEST]) {
        rotate(DIR_SOUTH);
      } else if (neighbor[DIR_WEST] && neighbor[DIR_NORTH]) {
        rotate(DIR_WEST);
      }
    } else if (num_neighbors == 3) { // Fast to check who DOESN'T have it
      if (!neighbor[DIR_NORTH]) {
        rotate(DIR_EAST);
      } else if (!neighbor[DIR_EAST]) {
        rotate(DIR_SOUTH);
      } else if (!neighbor[DIR_SOUTH]) {
        rotate(DIR_WEST);
      }
    } else if (num_neighbors != 4) {
      debugmsg("Used neighbor on a '%s' map with num_neighbors %d; '%s'/'%s'",
               world_ter[0]->name.c_str(), num_neighbors, name.c_str(),
               terrain_name.c_str());
    }
  } else if (!is_adjacent && world_ter[0] &&
             world_ter[0]->has_flag(WTF_FACE_ROAD)) {
    std::vector<Direction> valid_rotate;
    for (int i = 1; i < 5; i++) {
      if (world_ter[i]->has_flag(WTF_ROAD)) {
        valid_rotate.push_back( Direction(i) );
      }
    }
    if (valid_rotate.empty()) {
      random_rotate(); // Hopefully won't ever happen!
    } else {
      rotate( valid_rotate[ rng(0, valid_rotate.size() - 1) ] );
    }
  } else if (!is_adjacent && num_neighbors > 0 && z_level <= 0) {
    random_rotate();
  }
// Clear item locations
  for (std::map<char, Item_area>::iterator it = item_defs.begin();
       it != item_defs.end();
       it++) {
    it->second.clear_points();
  }
// Set up item locations
  
  for (int x = 0; x < MAPGEN_SIZE; x++) {
    for (int y = 0; y < MAPGEN_SIZE; y++) {
      char ch = prepped_terrain[x][y];
      if (item_defs.count(ch) != 0) {
        item_defs[ch].add_point(x, y);
      }
    }
  }
}

void Mapgen_spec::random_rotate()
{
  Direction dir = Direction(rng(DIR_NORTH, DIR_WEST));
  return rotate(dir);
}

void Mapgen_spec::rotate(Direction dir)
{
  rotation = dir;
  char tmp_terrain[MAPGEN_SIZE][MAPGEN_SIZE];
  for (int x = 0; x < MAPGEN_SIZE; x++) {
    for (int y = 0; y < MAPGEN_SIZE; y++) {
      int tx, ty;
      switch (dir) {
        case DIR_NORTH: tx = x;                   ty = y; break;
        case DIR_EAST:  tx = MAPGEN_SIZE - y - 1; ty = x; break;
        case DIR_SOUTH: tx = MAPGEN_SIZE - x - 1; ty = MAPGEN_SIZE - y - 1;
                        break;
        case DIR_WEST:  tx = y; ty = MAPGEN_SIZE - x - 1; break;
        default:        tx = x;                   ty = y; break;
      }
      tmp_terrain[tx][ty] = prepped_terrain[x][y];
    }
  }
  for (int x = 0; x < MAPGEN_SIZE; x++) {
    for (int y = 0; y < MAPGEN_SIZE; y++) {
      prepped_terrain[x][y] = tmp_terrain[x][y];
    }
  }
}

std::string Mapgen_spec::get_name()
{
  std::stringstream ret;
  ret << terrain_name << ":" << subname << ":" << name;
  return ret.str();
}

void Mapgen_spec::debug_output()
{
  std::ofstream fout;
  fout.open("debug.txt", std::fstream::app);
  fout << name << std::endl;
  for (int y = 0; y < MAPGEN_SIZE; y++) {
    for (int x = 0; x < MAPGEN_SIZE; x++) {
      fout << prepped_terrain[x][y];
    }
    fout << std::endl;
  }
  fout.close();
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
// Push into the subname map
  if (!tmp->is_adjacent && !tmp->subname.empty()) {
    if (subname_map.count(tmp->subname) == 0) {
      std::vector<Mapgen_spec*> tmpvec;
      tmpvec.push_back(tmp);
      subname_map[tmp->subname] = tmpvec;
      subname_total_chance[tmp->subname] = tmp->weight;
    } else {
      subname_map[tmp->subname].push_back(tmp);
      subname_total_chance[tmp->subname] += tmp->weight;
    }
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

Mapgen_spec* Mapgen_spec_pool::random_for_terrain(World_terrain* ptr,
                                                  std::vector<bool> neighbor)
{
  if (terrain_ptr_map.count(ptr) == 0) {
    return NULL;
  }
  std::vector<Mapgen_spec*> *vec = &(terrain_ptr_map[ptr]);
  std::vector<Mapgen_spec*> use;
  int num_neighbors = 0;
  for (int i = 1; i < neighbor.size(); i++) {
    if (neighbor[i]) {
      num_neighbors++;
    }
  }
// Special case "straight" ones - they're 1-neighbor too
  if (num_neighbors == 2 && ((neighbor[DIR_NORTH] && neighbor[DIR_SOUTH]) ||
                             (neighbor[DIR_EAST]  && neighbor[DIR_WEST] )   )) {
    num_neighbors = 1;
  }

  int new_total_chance = 0;
  for (int i = 0; i < vec->size(); i++) {
    if ( (*vec)[i]->num_neighbors == num_neighbors ) {
      use.push_back( (*vec)[i] );
      new_total_chance += (*vec)[i]->weight;
    }
  }
  if (use.empty()) {
    return NULL;
  }
  int index = rng(1, new_total_chance);
  for (int i = 0; i < use.size(); i++) {
    index -= use[i]->weight;
    if (index <= 0) {
      return use[i];
    }
  }
  return use.back();
}

Mapgen_spec* Mapgen_spec_pool::random_for_terrain(World_terrain* ptr,
                                                  std::string subname,
                                                  int z_level)
{
  if (terrain_ptr_map.count(ptr) == 0) {
    return NULL;
  }
  std::vector<Mapgen_spec*> *vec = &(terrain_ptr_map[ptr]);
  std::vector<Mapgen_spec*> use;
  int new_total_chance = 0;
  for (int i = 0; i < vec->size(); i++) {
    if ((*vec)[i]->z_level == z_level &&
        (subname.empty() || (*vec)[i]->subname == subname)) {
      use.push_back( (*vec)[i] );
      new_total_chance += (*vec)[i]->weight;
    }
  }
  if (use.empty()) {
    return NULL;
  }

  int index = rng(1, new_total_chance);
  for (int i = 0; i < use.size(); i++) {
    index -= use[i]->weight;
    if (index <= 0) {
      return use[i];
    }
  }
  return use.back();
}

Mapgen_spec* Mapgen_spec_pool::random_with_subname(std::string subname,
                                                   int z_level)
{
  if (subname.empty() || subname_map.count(subname) == 0) {
    return NULL;
  }
  std::vector<Mapgen_spec*> *vec = &(subname_map[subname]);
  std::vector<Mapgen_spec*> use;
  int new_total_chance = 0;
  for (int i = 0; i < vec->size(); i++) {
    if ((*vec)[i]->z_level == z_level) {
      use.push_back( (*vec)[i] );
      new_total_chance += (*vec)[i]->weight;
    }
  }
  if (use.empty()) {
    return NULL;
  }

  int index = rng(1, new_total_chance);
  for (int i = 0; i < use.size(); i++) {
    index -= use[i]->weight;
    if (index <= 0) {
      return use[i];
    }
  }
  return use.back();
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
