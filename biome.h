#ifndef _BIOME_H_
#define _BIOME_H_

#include "world_terrain.h"
#include "monster_type.h"
#include "dice.h"
#include <istream>

// TODO: Move this to monster_type.h?

struct Monster_genus_chance
{
  Monster_genus_chance(int C = 100, Monster_genus* MG = NULL) :
    chance (C), genus (MG) { }

  int chance;
  Monster_genus* genus;
};

struct Variable_monster_genus
{
public:
  Variable_monster_genus();
  ~Variable_monster_genus();

  void add_genus(int chance, Monster_genus* genus);
  void add_genus(Monster_genus_chance genus);
  bool load_data(std::istream &data, std::string name = "unknown");

  int size();
  Monster_genus* pick();  // Pick a single random genus
  std::vector<Monster_genus*> pick(int num);  // Return a list of $num genera
  int pick_number(); // Pick how many genera to spawn

private:
  std::vector<Monster_genus_chance> genera;
  int total_chance;
};

enum Biome_flag
{
  BIOME_FLAG_NULL = 0,
  BIOME_FLAG_LAKE,    // "lake" - turn to ocean if ocean-adjacent
  BIOME_FLAG_CITY,    // "city" - turn into city buildings
  BIOME_FLAG_NO_OCEAN,// "no_ocean" - don't turn into ocean even if altitude<=0
  BIOME_FLAG_MAX
};

Biome_flag lookup_biome_flag(std::string name);
std::string biome_flag_name(Biome_flag flag);

struct Biome
{
  Biome();
  ~Biome();

  std::string name;
  std::string display_name;
  int uid;

  Variable_world_terrain terrain;
  Variable_world_terrain bonuses;
  Variable_world_terrain road_bonuses;

  Variable_monster_genus monsters;
  Dice monster_population;

  void assign_uid(int id);
  std::string get_data_name();
  std::string get_name();
  bool load_data(std::istream &data);

  World_terrain* pick_terrain();
  World_terrain* pick_bonus();
  World_terrain* pick_road_bonus();

  bool has_flag(Biome_flag flag);

private:
  std::vector<bool> flags;
};

enum Lake_status
{
  LAKE_UNCHECKED,
  LAKE_CHECKED,
  LAKE_NOTLAKE
};

enum City_status
{
  CITY_NOTCITY,
  CITY_HUB,
  CITY_RAW, // Not generated yet
  CITY_BUILDING,
  CITY_BUILDING_CLOSED,
  CITY_ROAD,
  CITY_ROAD_CLOSED
};

#endif
