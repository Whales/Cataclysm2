#ifndef _BIOME_H_
#define _BIOME_H_

#include "world_terrain.h"

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

World_terrain* terrain_from_biome(Biome_type type);

#endif
