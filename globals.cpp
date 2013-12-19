#include "globals.h"
#include "datapool.h"
#include "files.h"
#include "game.h"

Game                      GAME;
Data_pool<Terrain>        TERRAIN;
Data_pool<World_terrain>  WORLD_TERRAIN;
Data_pool<Itemtype>       ITEMTYPES;
Mapgen_spec_pool          MAPGEN_SPECS;

void load_global_data()
{
  TERRAIN.load_from("data/terrain.dat");
  WORLD_TERRAIN.load_from("data/world_terrain.dat");
  ITEMTYPES.load_from("data/items.dat");

  load_mapgen_specs();
  debugmsg("%d mapgen specs", MAPGEN_SPECS.size());
  debugmsg("%d items", ITEMTYPES.size());
}

void load_mapgen_specs()
{
  std::vector<std::string> mapgen_files = files_in("data/mapgen", "map");
  for (int i = 0; i < mapgen_files.size(); i++) {
    std::string filename = "data/mapgen/" + mapgen_files[i];
    MAPGEN_SPECS.load_from(filename);
  }
}
