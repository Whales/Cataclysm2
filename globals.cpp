#include "globals.h"
#include "datapool.h"

Game                      GAME;
Data_pool<Terrain>        TERRAIN;
Data_pool<World_terrain>  WORLD_TERRAIN;

void load_global_data()
{
  TERRAIN.load_from("data/terrain.dat");
  WORLD_TERRAIN.load_from("data/world_terrain.dat");

  load_mapgen_specs();
}

void load_mapgen_specs()
{
// Start by clearing mapgen specs
  for (std::list<World_terrain*>::iterator it = WORLD_TERRAIN.instances.begin();
       it != WORLD_TERRAIN.instances.end();
       it++) {
    (*it)->mapgen_specs.clear();
  }

  std::vector<std::string> mapgen_files = files_in("data/mapgen");
