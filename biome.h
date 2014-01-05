#ifndef _BIOME_H_
#define _BIOME_H_

#include "world_terrain.h"
#include <istream>

struct World_terrain_chance
{
  World_terrain_chance(int C = 10, World_terrain* WT = NULL) :
    chance (C), terrain (WT) {};
  int chance;
  World_terrain* terrain;
};

struct Variable_world_terrain
{
public:
  Variable_world_terrain();
  ~Variable_world_terrain(){};

  void add_terrain(int chance, World_terrain* terrain);
  void add_terrain(World_terrain_chance terrain);
  void load_data(std::istream &data, std::string name = "unknown");

  World_terrain* pick();

private:
  std::vector<World_terrain_chance> ter;
  int total_chance;
};

enum Biome_flag
{
  BIOME_FLAG_NULL = 0,
  BIOME_FLAG_LAKE,    // "lake" - turn to ocean if ocean-adjacent
  BIOME_FLAG_MAX
};

Biome_flag lookup_biome_flag(std::string name);
std::string biome_flag_name(Biome_flag flag);

struct Biome
{
  Biome();
  ~Biome();

  std::string name;
  int uid;

  Variable_world_terrain terrain;

  void assign_uid(int id);
  std::string get_name();
  bool load_data(std::istream &data);

  World_terrain* pick_terrain();
  bool has_flag(Biome_flag flag);
private:
  std::vector<bool> flags;
};

enum Biome_type
{
  BIOME_NULL,
  BIOME_GRASSLAND,
  BIOME_FOREST,
  BIOME_SWAMP,
  BIOME_MUSHROOM,
  BIOME_ROCKY,
  BIOME_CITY,
  BIOME_LAKE, // May be a lake or a gulf (if ocean-adjacent)
  BIOME_MAX
};

enum Lake_status
{
  LAKE_NOTLAKE,
  LAKE_UNKNOWN,
  LAKE_LANDLOCKED,
  LAKE_OCEAN
};

#endif
