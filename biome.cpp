#include "biome.h"
#include "globals.h"
#include "rng.h"

World_terrain* terrain_from_biome(Biome_type type)
{
  std::string name;
  switch (type) {
    case BIOME_NULL:
      name = "ocean";
      break;
    case BIOME_GRASSLAND:
      name = "field";
      break;
    case BIOME_FOREST:
      if (one_in(10)) {
        name = "field";
      } else {
        name = "forest";
      }
      break;
    case BIOME_SWAMP:
      name = "swamp";
      break;
    case BIOME_MUSHROOM:
      name = "mushroom forest";
      break;
    case BIOME_ROCKY:
      name = "rocky";
      break;
    case BIOME_CITY:
      name = "town square";
      break;
    case BIOME_LAKE:
      name = "fresh water";
      break;
    default:
      name = "field";
      break;
  }
  return WORLD_TERRAIN.lookup_name(name);
}
