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

bool Variable_terrain::contains(std::string terrain_name)
{
  Terrain* t = TERRAIN.lookup_name(terrain_name);
  if (!t) {
    return false;
  }
  return contains(t);
}

bool Variable_terrain::contains(Terrain* terrain)
{
  for (int i = 0; i < ter.size(); i++) {
    if (ter[i].terrain == terrain) {
      return true;
    }
  }
  return false;
}

Item_area::Item_area()
{
  total_chance = 0;
  use_all_items = false;
  all_index = 0;
  all_count = 0;
}

void Item_area::add_item(int chance, Item_type* item_type)
{
  Item_type_chance tmp(chance, 1, item_type);
  add_item(tmp);
}

void Item_area::add_item(int chance, int number, Item_type* item_type)
{
  Item_type_chance tmp(chance, number, item_type);
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
    } else if(item_ident.substr(0, 2) == "c:") { // It's a count - for all_items
      tmp_chance.number = atoi( item_ident.substr(2).c_str() );
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

void Item_area::reset()
{
  all_index = 0;
  all_count = 0;
}

bool Item_area::place_item()
{
// If use_all_items is true, we just check if the index is in bounds
  if (use_all_items) {
    return (all_index < item_types.size());
  }
  if (overall_chance >= 100 || overall_chance <= 0) {
    return false;
  }
  return (rng(1, 100) <= overall_chance);
}

Item_type* Item_area::pick_type()
{
// If use_all_items is true, we just scroll through the list
  if (use_all_items) {
    if (all_index >= item_types.size()) {
      return NULL;
    }
    Item_type* ret = item_types[all_index].item;
    all_count++;
    if (all_count >= item_types[all_index].number) {
      all_count = 0;
      all_index++;
    }
    return ret;
  }

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
  std::string ident, junk;

  while (ident != "done" && !data.eof()) {
    if ( ! (data >> ident) ) {
      return false;
    }
    ident = no_caps(ident);

    if (!ident.empty() && ident[0] == '#') {
      std::getline(data, junk); // It's a comment - clear the line

    } else if (ident == "name:") {
      std::getline(data, name);
      name = no_caps(name);
      name = trim(name);

    } else if (ident == "items:") {
      if (!load_item_data(data, name)) {
        return false;
      }

    } else if (ident != "done") {
      debugmsg("Unknown Item_group identifier '%s' (%s)",
               ident.c_str(), name.c_str());
      return false;
    }
  }
  return true;
}

bool Item_group::load_item_data(std::istream& data, std::string owner_name)
{
  std::string item_ident;
  std::string item_name;
  std::string item_line;
  std::getline(data, item_line);
  std::istringstream item_ss(item_line);
  Item_type_chance tmp_chance;

  while (item_ss >> item_ident) {
    item_ident = no_caps(item_ident);  // Nothing is case-sensitive

    if (item_ident.substr(0, 2) == "w:") { // It's a weight, e.g. a chance
      tmp_chance.chance = atoi( item_ident.substr(2).c_str() );

    } else if (item_ident.substr(0, 2) == "c:") { // For all_items groups
      tmp_chance.number = atoi( item_ident.substr(2).c_str() );

    } else if (item_ident == "/") { // End of this option
      item_name = trim(item_name);
      Item_type* tmpitem = ITEM_TYPES.lookup_name(item_name);
      if (!tmpitem) {
        debugmsg("Unknown item '%s' (%s)", item_name.c_str(),
                 owner_name.c_str());
        return false;
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
             owner_name.c_str());
  }
  tmp_chance.item = tmpitem;
  add_item(tmp_chance);
  return true;
}


void Item_group::add_item(int chance, Item_type* item_type)
{
  Item_type_chance tmp(chance, 1, item_type);
  add_item(tmp);
}

void Item_group::add_item(int chance, int number, Item_type* item_type)
{
  Item_type_chance tmp(chance, number, item_type);
  add_item(tmp);
}

void Item_group::add_item(Item_type_chance item_type)
{
  item_types.push_back(item_type);
  total_chance += item_type.chance;
}

std::vector<Item_type*> Item_group::get_all_item_types()
{
  std::vector<Item_type*> ret;
  for (int i = 0; i < item_types.size(); i++) {
    ret.push_back(item_types[i].item);
  }
  return ret;
}

Item_type* Item_group::pick_type()
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

Item_amount_area::Item_amount_area()
{
  total_chance = 0;
}

void Item_amount_area::add_item(Dice amount, int chance, Item_type* item)
{
  Item_amount tmp;
  tmp.amount = amount;
  tmp.chance = chance;
  tmp.item = item;
  add_item(tmp);
}

void Item_amount_area::add_item(Item_amount item)
{
  items.push_back(item);
  total_chance += item.chance;
}

void Item_amount_area::clear_points()
{
  locations.clear();
}

void Item_amount_area::add_point(int x, int y)
{
  locations.push_back( Point(x, y) );
}

bool Item_amount_area::load_data(std::istream& data, std::string name)
{
  std::string ident;
  std::string item_name;
  Item_amount tmp_chance;

  while (data >> ident) {
    ident = no_caps(ident);  // other stuff isn't case-sensitive

    if (ident.substr(0, 2) == "w:") { // It's a weight, e.g. a chance
      tmp_chance.chance = atoi( ident.substr(2).c_str() );

    } else if (ident == "c:") { // It's a count
      if (!tmp_chance.amount.load_data(data, name + " chance")) {
        debugmsg("Item_amount_area chance failed to load.");
        return false;
      }

    } else if (ident == "/") { // End of this option
      item_name = trim(item_name);
      Item_type* tmpitem = ITEM_TYPES.lookup_name(item_name);
      if (!tmpitem) {
        debugmsg("Unknown Item '%s' (%s)", item_name.c_str(),
                 name.c_str());
        return false;
      }
      tmp_chance.item = tmpitem;
      add_item(tmp_chance);
      tmp_chance.chance = 10;
      tmp_chance.item  = NULL;
      tmp_chance.amount = Dice(0, 1, 1);
      item_name = "";

    } else { // Otherwise, it should be a group name
      item_name = item_name + " " + ident;
    }

  }
// Add the last group group to our list, if the group is valid
  item_name = trim(item_name);
  Item_type* tmpitem = ITEM_TYPES.lookup_name(item_name);
  if (!tmpitem) {
    debugmsg("Unknown Item '%s' (%s)", item_name.c_str(),
             name.c_str());
  }
  tmp_chance.item = tmpitem;
  add_item(tmp_chance);
  return true;
}

Item_amount Item_amount_area::pick_item()
{
  if (items.empty()) {
    return Item_amount();
  }

  int index = rng(1, total_chance);
  for (int i = 0; i < items.size(); i++) {
    index -= items[i].chance;
    if (index <= 0) {
      return items[i];
    }
  }
  return items.back();
}

Point Item_amount_area::pick_location()
{
  if (locations.empty()) {
    return Point(-1, -1);
  }
  int index = rng(0, locations.size() - 1);
  return locations[index];
}

Item_group_amount_area::Item_group_amount_area()
{
  total_chance = 0;
}

void Item_group_amount_area::add_group(Dice amount, int chance, Item_group* group)
{
  Item_group_amount tmp;
  tmp.amount = amount;
  tmp.chance = chance;
  tmp.group = group;
  add_group(tmp);
}

void Item_group_amount_area::add_group(Item_group_amount group)
{
  groups.push_back(group);
  total_chance += group.chance;
}

void Item_group_amount_area::clear_points()
{
  locations.clear();
}

void Item_group_amount_area::add_point(int x, int y)
{
  Point tmp(x, y);
  locations.push_back(tmp);
}

bool Item_group_amount_area::load_data(std::istream& data, std::string name)
{
  std::string group_ident;
  std::string group_name;
  Item_group_amount tmp_chance;

  while (data >> group_ident) {
    group_ident = no_caps(group_ident);  // other stuff isn't case-sensitive

    if (group_ident.substr(0, 2) == "w:") { // It's a weight, e.g. a chance
      tmp_chance.chance = atoi( group_ident.substr(2).c_str() );

    } else if (group_ident == "c:") { // It's a count
      if (!tmp_chance.amount.load_data(data, name + " chance")) {
        debugmsg("Item_group_amount_area chance failed to load.");
        return false;
      }

    } else if (group_ident == "/") { // End of this option
      group_name = trim(group_name);
      Item_group* tmpgroup = ITEM_GROUPS.lookup_name(group_name);
      if (!tmpgroup) {
        debugmsg("Unknown Item_group '%s' (%s)", group_name.c_str(),
                 name.c_str());
        return false;
      }
      tmp_chance.group = tmpgroup;
      add_group(tmp_chance);
      tmp_chance.chance = 10;
      tmp_chance.group   = NULL;
      tmp_chance.amount = Dice(0, 1, 1);
      group_name = "";

    } else { // Otherwise, it should be a group name
      group_name = group_name + " " + group_ident;
    }

  }
// Add the last group group to our list, if the group is valid
  group_name = trim(group_name);
  Item_group* tmpgroup = ITEM_GROUPS.lookup_name(group_name);
  if (!tmpgroup) {
    debugmsg("Unknown Item_group '%s' (%s)", group_name.c_str(),
             name.c_str());
  }
  tmp_chance.group = tmpgroup;
  add_group(tmp_chance);
  return true;
}

Item_group_amount Item_group_amount_area::pick_group()
{
  if (groups.empty()) {
    return Item_group_amount();
  }

  int index = rng(1, total_chance);
  for (int i = 0; i < groups.size(); i++) {
    index -= groups[i].chance;
    if (index <= 0) {
      return groups[i];
    }
  }
  return groups.back();
}

Point Item_group_amount_area::pick_location()
{
  if (locations.empty()) {
    return Point(-1, -1);
  }
  int index = rng(0, locations.size() - 1);
  return locations[index];
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
  for (int i = 0; i < MAPFLAG_MAX; i++) {
    flags.push_back(false);
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

    } else if (ident == "flags:") {
      std::string line;
      std::getline(data, line);
      std::istringstream flag_data(line);
      std::string flag_name;
      while (flag_data >> flag_name) {
        Mapgen_flag flag = lookup_mapgen_flag(flag_name);
        if (flag == MAPFLAG_NULL) {
          debugmsg("Unknown Mapgen_flag '%s' (%s).", flag_name.c_str(),
                   name.c_str());
          return false;
        }
        flags[flag] = true;
      }

    } else if (ident == "adjacent") {
      is_adjacent = true;
      std::getline(data, junk);

    } else if (ident == "adj_on:" || ident == "adjacent_on:") {
      if (!is_adjacent) {
        debugmsg("%s line for a non-adjacency map! (%s)", ident.c_str(),
                 name.c_str());
        return false;
      }
      std::getline(data, adj_terrain_name);
      adj_terrain_name = trim(adj_terrain_name);

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
          debugmsg("Tried to map tile %c - already in use (%s)", ch,
                   name.c_str());
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
    } else if (ident == "item_group:" || ident == "all_item_group:") {
      Item_area tmp_area;
      if (ident == "all_item_group:") {
        tmp_area.use_all_items = true;
      } else {
        data >> tmp_area.overall_chance;
      }

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
      group_name = no_caps(group_name);
      group_name = trim(group_name);

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
          debugmsg("Tried to map item_group %c - already in use (%s)", ch,
                   name.c_str());
        } else {
          item_defs[ch] = tmp_area;
        }
      }
// End of item_group section

    } else if (ident == "num_item_group:") {
      Item_group_amount_area tmp_area;

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
      if (!tmp_area.load_data(item_data, name)) {
        debugmsg("Failed to load Item_group_amount_area (%s)", name.c_str());
        return false;
      }
// For every character in symbols, map that char to tmp_var
      for (int i = 0; i < symbols.length(); i++) {
        char ch = symbols[i];
        if (item_defs.count(ch) != 0) {
          debugmsg("Tried to map num_item_group %c - already in use (%s)", ch,
                   name.c_str());
        } else {
          item_group_defs[ch] = tmp_area;
        }
      }
// End if (ident == "num_item_group:") block

    } else if (ident == "num_items:") {
      Item_amount_area tmp_area;

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
      if (!tmp_area.load_data(item_data, name)) {
        debugmsg("Failed to load Item_group_amount_area (%s)", name.c_str());
        return false;
      }
// For every character in symbols, map that char to tmp_var
      for (int i = 0; i < symbols.length(); i++) {
        char ch = symbols[i];
        if (item_defs.count(ch) != 0) {
          debugmsg("Tried to map num_items %c - already in use (%s)", ch,
                   name.c_str());
        } else {
          item_amount_defs[ch] = tmp_area;
        }
      }
// End if (ident == "num_items:") block

    } else if (ident == "items:" || ident == "all_items:") {
      Item_area tmp_area;

      if (ident == "all_items:") {
        tmp_area.use_all_items = true;
      } else {
        data >> tmp_area.overall_chance;
      }

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
          debugmsg("Tried to map item %c - already in use (%s)", ch,
                   name.c_str());
        } else {
          item_defs[ch] = tmp_area;
        }
      }
// End if (ident == "items:") block

    } else if (ident == "furniture:") {
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

      std::string furniture_name;
      std::getline(data, furniture_name);
      furniture_name = trim(furniture_name);

      Furniture_type* tmptype = FURNITURE_TYPES.lookup_name(furniture_name);
      if (!tmptype) {
        debugmsg("Unknown Furniture_type '%s' (%s)", furniture_name.c_str(),
                 name.c_str());
        return false;
      }
// For every character in symbols, map that char to tmptype
      for (int i = 0; i < symbols.length(); i++) {
        char ch = symbols[i];
        if (furniture.count(ch) != 0) {
          debugmsg("Tried to map furniture %c - already in use (%s)", ch,
                   name.c_str());
        } else {
          furniture[ch] = tmptype;
        }
      }

    } else if (ident == "map:") {
      std::string mapchars;
      std::getline(data, mapchars);
      int line = 0;
      do {
        std::getline(data, mapchars);
        
        if (no_caps( trim(mapchars) ) != "endmap" &&
            mapchars.length() != MAPGEN_SIZE) {
          debugmsg("Width %d", mapchars.length());
          debugmsg("Bad map width '%s' (%s)", mapchars.c_str(), name.c_str());
        }
        for (int i = 0; i < mapchars.length(); i++) {
          terrain[i][line] = mapchars[i];
        }
        line++;
      } while (no_caps( trim(mapchars) ) != "endmap" && line < MAPGEN_SIZE);
      if (line != MAPGEN_SIZE) {
        debugmsg("Bad map height %d (%s)", line, name.c_str());
      }
    } else if (ident != "done" && ident != "endmap") {
      debugmsg("Unknown Mapgen_spec property '%s' (%s)",
               ident.c_str(), name.c_str());
    }
  } while (ident != "done" && !data.eof());

  return verify_map();
}

bool Mapgen_spec::verify_map()
{
  std::vector<std::string> errors;
  std::vector<char> chars_checked;
  for (int x = 0; x < MAPGEN_SIZE; x++) {
    for (int y = 0; y < MAPGEN_SIZE; y++) {
      char ch = terrain[x][y];
      bool needs_checked = true;
      for (int i = 0; i < chars_checked.size(); i++) {
        if (ch == chars_checked[i]) {
          needs_checked = false;
        }
      }
      if (needs_checked) {
        chars_checked.push_back(ch);
        if (substitutions.count(ch) > 0 && base_terrain.ter.empty()) {
// The tile will end up being substituted, so check all possible outcomes
          Tile_substitution *subst = &(substitutions[ch]);
          for (int i = 0; i < subst->chances.size(); i++) {
            char substch = subst->chances[i].result;
            if (terrain_defs.count(substch) == 0) {
              std::stringstream error_ss;
              error_ss << ch << " - invalid subst result '" << substch << "'";
              errors.push_back( error_ss.str() );
            }
          }
        } else if (terrain_defs.count(ch) == 0 && base_terrain.ter.empty() &&
                   !is_adjacent) {
// "else if" because we don't NEED a valid terrain_def if the char is going to
// end up substituted!
          std::stringstream error_ss;
          error_ss << ch << " - no terrain defined!";
          errors.push_back( error_ss.str() );
        }
      }
    }
  }
// Check for "empty" in floor 0 maps
  if (z_level == 0) {
    if (base_terrain.contains("empty")) {
      errors.push_back("base_terrain includes 'empty' on a 0-level map!");
    }
    for (std::map<char,Variable_terrain>::iterator it = terrain_defs.begin();
         it != terrain_defs.end();
         it++) {
      if ( it->second.contains("empty") ) {
        std::stringstream error_ss;
        error_ss << it->first << " - may be 'empty' on a 0-level map!";
        errors.push_back( error_ss.str() );
      }
    }
  }

  if (errors.empty()) {
    return true;  // Hooray!
  }

  if (errors.size() == 1) {
    debugmsg("Error in %s:\n%s", get_name().c_str(), errors[0].c_str() );
  } else {
    std::ofstream fout;
    fout.open("error_mapgen.txt", std::fstream::app);
    fout << name << std::endl;
    for (int i = 0; i < errors.size(); i++) {
      fout << errors[i] << std::endl;
    }
    fout.close();
    debugmsg("%d errors in %s.\nLogged to ./error_mapgen.txt.", errors.size(),
             get_name().c_str());
  }
  return false;
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

Furniture_type* Mapgen_spec::pick_furniture(int x, int y)
{
  if (x < 0 || x >= MAPGEN_SIZE || y < 0 || y >= MAPGEN_SIZE) {
    return NULL;
  }
  char key = prepped_terrain[x][y];
  if (furniture.count(key) == 0) {
    return NULL;
  }
  return furniture[key];
}

int Mapgen_spec::pick_furniture_uid(int x, int y)
{
  if (x < 0 || x >= MAPGEN_SIZE || y < 0 || y >= MAPGEN_SIZE) {
    return -1;
  }
  return furniture_uid[x][y];
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
  if (!is_adjacent && world_ter && world_ter[0] && num_neighbors > 0 &&
      world_ter[0]->has_flag(WTF_RELATIONAL)) {
    std::vector<bool> neighbor;
    neighbor.push_back(false);
    for (int i = 1; i < 5; i++) {
      neighbor.push_back( (world_ter[i] == world_ter[0]) );
    }
    if (num_neighbors == 1 || num_neighbors == 11) {
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
    } else if (num_neighbors == 3) { // Faster to check who DOESN'T have it
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
// If we're a road-facing map, face the road...
  } else if (!is_adjacent && world_ter && world_ter[0] &&
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
      if (item_group_defs.count(ch) != 0) {
        item_group_defs[ch].add_point(x, y);
      }
      if (item_amount_defs.count(ch) != 0) {
        item_amount_defs[ch].add_point(x, y);
      }
    }
  }
// Finally, assign furniture uids
  assign_furniture_uids();
}

void Mapgen_spec::assign_furniture_uids()
{
// First, set everything in our array to -1
  for (int x = 0; x < MAPGEN_SIZE; x++) {
    for (int y = 0; y < MAPGEN_SIZE; y++) {
      furniture_uid[x][y] = -1;
    }
  }
// Now set the UIDs
  int next_uid = 0;
  for (int x = 0; x < MAPGEN_SIZE; x++) {
    for (int y = 0; y < MAPGEN_SIZE; y++) {
      char terch = prepped_terrain[x][y];
      if (furniture_uid[x][y] == -1 && furniture.count(terch) != 0) {
        mark_furniture_uid(x, y, next_uid);
        next_uid++;
      }
    }
  }
}

void Mapgen_spec::mark_furniture_uid(int x, int y, int uid)
{
  if (x < 0 || x >= MAPGEN_SIZE || y < 0 || y >= MAPGEN_SIZE) {
    return; // Out-of-bounds, so stop.
  }
  char terch = prepped_terrain[x][y];
  if (furniture.count(terch) == 0) {
    return; // No furniture here, so stop.
  }
  if (furniture_uid[x][y] != -1) {
    return; // Already assigned a furniture uid, so stop.
  }

  furniture_uid[x][y] = uid;
// Now mark all adjacent tiles with the same furniture binding
  for (int i = 1; i <= 4; i++) {
    int xrec = x, yrec = y;
    switch (i) {
      case 1: xrec--; break;
      case 2: xrec++; break;
      case 3: yrec--; break;
      case 4: yrec++; break;
    }
/* Only recurse if the neighboring tile is inbounds and has the same prepped
 * character code as us.  As seen above, we don't have to worry about checking
 * furniture_uid.
 */
    if (xrec >= 0 && xrec < MAPGEN_SIZE && yrec >= 0 && yrec < MAPGEN_SIZE &&
        prepped_terrain[xrec][yrec] == terch) {
      mark_furniture_uid(xrec, yrec, uid);
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

bool Mapgen_spec::has_flag(Mapgen_flag flag)
{
  if (flag < 0 || flag >= flags.size()) {
    return false;
  }
  return flags[flag];
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
// Special case "straight" ones - they're 11-neighbor (1 and 1)
  if (num_neighbors == 2 && ((neighbor[DIR_NORTH] && neighbor[DIR_SOUTH]) ||
                             (neighbor[DIR_EAST]  && neighbor[DIR_WEST] )   )) {
    num_neighbors = 11;
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

Mapgen_spec* Mapgen_spec_pool::random_adjacent_to(std::string name,
                                                  std::string here)
{
  here = no_caps(here);
  here = trim(here);
  if (adjacent_name_map.count(name) == 0) {
    return NULL;
  }

  std::vector<Mapgen_spec*> valid;
  std::vector<Mapgen_spec*> *vec = &(adjacent_name_map[name]);
  int total_chance = 0;

  if (here == "") { // Works anywhere!
    for (int i = 0; i < vec->size(); i++) {
      valid.push_back( (*vec)[i] );
    }
    total_chance = adjacent_name_total_chance[name];

  } else {
/* We need to only include adjacency maps that either don't specify where they
 * can be placed, or can be placed on maps of type "here"
 */
    for (int i = 0; i < vec->size(); i++) {
      std::string adj_name = no_caps( (*vec)[i]->adj_terrain_name );
      if (adj_name.empty() || adj_name == here) {
        valid.push_back( (*vec)[i] );
        total_chance += (*vec)[i]->weight;
      }
    }
  }

  if (valid.empty()) {
    return NULL;
  }

  int index = rng(1, total_chance);
  for (int i = 0; i < valid.size(); i++) {
    index -= valid[i]->weight;
    if (index <= 0) {
      return valid[i];
    }
  }
  return valid.back();
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

Mapgen_spec* Mapgen_spec_pool::random_adjacent_to(World_terrain *ptr,
                                                  World_terrain *ptr_here)
{
  if (!ptr || adjacent_ptr_map.count(ptr) == 0) {
    return NULL;
  }

  std::vector<Mapgen_spec*> valid;
  std::vector<Mapgen_spec*> *vec = &(adjacent_ptr_map[ptr]);
  int total_chance = 0;

  if (ptr_here) {
/* We need to only include adjacency maps that either don't specify where they
 * can be placed, or can be placed on maps of type "here"
 */
    std::string here_name = no_caps( ptr_here->get_data_name() );
    for (int i = 0; i < vec->size(); i++) {
      std::string adj_name = no_caps( (*vec)[i]->adj_terrain_name );
      if (adj_name.empty() || adj_name == here_name) {
        valid.push_back( (*vec)[i] );
        total_chance += (*vec)[i]->weight;
      }
    }

  } else {  // All are valid!
    for (int i = 0; i < vec->size(); i++) {
      valid.push_back( (*vec)[i] );
    }
    total_chance = adjacent_ptr_total_chance[ptr];
  }

  if (valid.empty()) {
    return NULL;
  }

  int index = rng(1, total_chance);
  for (int i = 0; i < valid.size(); i++) {
    index -= valid[i]->weight;
    if (index <= 0) {
      return valid[i];
    }
  }
  return valid.back();
}

int Mapgen_spec_pool::size()
{
  return instances.size();
}

Mapgen_flag lookup_mapgen_flag(std::string name)
{
  name = trim( no_caps( name ) );
  for (int i = 0; i < MAPFLAG_MAX; i++) {
    Mapgen_flag ret = Mapgen_flag(i);
    if ( no_caps( mapgen_flag_name(ret) ) == name ) {
      return ret;
    }
  }
  return MAPFLAG_NULL;
}

std::string mapgen_flag_name(Mapgen_flag flag)
{
  switch (flag) {
    case MAPFLAG_NULL:          return "NULL";
    case MAPFLAG_AUTOSTAIRS:    return "autostairs";
    case MAPFLAG_MAX:           return "ERROR - MAPFLAG_MAX";
    default:                    return "UNKNOWN MAPGEN_FLAG";
  }
  return "ERROR - Escaped mapgen_flag_name switch";
}
