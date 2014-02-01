#include "biome.h"
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

World_terrain* Variable_world_terrain::pick()
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

Variable_monster_genus::Variable_monster_genus()
{
  total_chance = 0;
}

Variable_monster_genus::~Variable_monster_genus()
{
}

void Variable_monster_genus::add_genus(int chance, Monster_genus* genus)
{
  if (!genus) {
    return;
  }
  add_genus( Monster_genus_chance(chance, genus) );
}

void Variable_monster_genus::add_genus(Monster_genus_chance genus)
{
  total_chance += genus.chance;
  genera.push_back(genus);
}

bool Variable_monster_genus::load_data(std::istream &data, std::string name)
{
  std::string ident;
  std::string genus_name;
  Monster_genus_chance tmp_chance;
  while (data >> ident) {
    ident = no_caps(ident);
    if (ident.substr(0, 2) == "w:") { // It's a weight, i.e. a chance
      tmp_chance.chance = atoi( ident.substr(2).c_str() );
    } else if (ident == "/") { // End of this option
      genus_name = trim(genus_name);
      Monster_genus* tmpgenus = MONSTER_GENERA.lookup_name(genus_name);
      if (!tmpgenus) {
        debugmsg("Unknown genus '%s' (%s)", genus_name.c_str(), name.c_str());
        return false;
      }
      tmp_chance.genus = tmpgenus;
      add_genus(tmp_chance);
      tmp_chance.chance  = 10;
      tmp_chance.genus = NULL;
      genus_name = "";
    } else { // Otherwise it should be a terrain name
      genus_name = genus_name + " " + ident;
    }
  }
// Add the last genus to our list
  genus_name = trim(genus_name);
  Monster_genus* tmpgenus = MONSTER_GENERA.lookup_name(genus_name);
  tmp_chance.genus = tmpgenus;
  if (!tmpgenus) {
    debugmsg("Unknown world terrain '%s' (%s)", genus_name.c_str(),
             name.c_str());
    return false;
  }
  add_genus(tmp_chance);
  return true;
}

int Variable_monster_genus::size()
{
  return genera.size();
}

Monster_genus* Variable_monster_genus::pick()
{
  if (genera.empty()) {
    return NULL;
  }

  int index = rng(1, total_chance);
  for (int i = 0; i < genera.size(); i++) {
    index -= genera[i].chance;
    if (index <= 0) {
      return genera[i].genus;
    }
  }
  return genera.back().genus;
}

std::vector<Monster_genus*> Variable_monster_genus::pick(int num)
{
  std::vector<Monster_genus*> ret;

  if (genera.empty()) {
    return ret;
  }

  if (num >= genera.size()) {
    for (int i = 0; i < genera.size(); i++) {
      ret.push_back( genera[i].genus );
    }
    return ret;
  }

  std::vector<bool> picked;
  for (int i = 0; i < genera.size(); i++) {
    picked.push_back(false);
  }

  int new_total = total_chance;
  while (ret.size() < num) {
    int index = rng(1, new_total);
    for (int i = 0; i < genera.size(); i++) {
// Skip any we've already picked
      while (picked[i] && i < genera.size()) {
        i++;
      }
      index -= genera[i].chance;
      if (index <= 0) {
        picked[i] = true;
        new_total -= genera[i].chance;
        ret.push_back( genera[i].genus );
      }
    }
  }
  return ret;
}

int Variable_monster_genus::pick_number()
{
  int ret = 0;
  while (rng(1, size()) > ret) {
    ret++;
  }
  return ret;
}

Biome::Biome()
{
  name = "Unknown";
  uid = -1;
  for (int i = 0; i < BIOME_FLAG_MAX; i++) {
    flags.push_back(false);
  }
}

Biome::~Biome()
{
}

void Biome::assign_uid(int id)
{
  uid = id;
}

std::string Biome::get_data_name()
{
  return name;
}

std::string Biome::get_name()
{
  if (display_name.empty()) {
    return name;
  }
  return display_name;
}

bool Biome::load_data(std::istream &data)
{
  std::string ident, junk;
  while (ident != "done" && !data.eof()) {

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

    } else if (ident == "terrain:") {
      std::string terrain_line;
      std::getline(data, terrain_line);
      std::istringstream terrain_data(terrain_line);
      terrain.load_data(terrain_data, name);

    } else if (ident == "monsters:") {
      std::string monster_line;
      std::getline(data, monster_line);
      std::istringstream monster_data(monster_line);
      monsters.load_data(monster_data, name);

    } else if (ident == "population:" || ident == "monster_population:") {
      std::string dice_line;
      std::getline(data, dice_line);
      std::istringstream dice_data(dice_line);
      monster_population.load_data(dice_data, name);

    } else if (ident == "flags:") {
      std::string line;
      std::getline(data, line);
      std::istringstream flag_data(line);
      std::string flag_name;
      while (flag_data >> flag_name) {
        flags[ lookup_biome_flag(flag_name) ] = true;
      }

    } else if (ident != "done") {
      debugmsg("Unknown Biome property '%s' (%s)",
               ident.c_str(), name.c_str());
    }
  }
  return true;
}

World_terrain* Biome::pick_terrain()
{
  return terrain.pick();
}

bool Biome::has_flag(Biome_flag flag)
{
  return flags[flag];
}

Biome_flag lookup_biome_flag(std::string name)
{
  name = no_caps(name);
  for (int i = 0; i < BIOME_FLAG_MAX; i++) {
    Biome_flag ret = Biome_flag(i);
    if ( no_caps(biome_flag_name(ret)) == name ) {
      return ret;
    }
  }
  return BIOME_FLAG_NULL;
}

std::string biome_flag_name(Biome_flag flag)
{
  switch (flag) {
    case BIOME_FLAG_NULL:     return "NULL";
    case BIOME_FLAG_LAKE:     return "lake";
    case BIOME_FLAG_CITY:     return "city";
    case BIOME_FLAG_NO_OCEAN: return "no_ocean";
    case BIOME_FLAG_MAX:      return "ERROR - BIOME_FLAG_MAX";
    default:                  return "ERROR - Unnamed Biome_flag";
  }
  return "ERROR - Escaped switch";
}
