#include "globals.h"
#include "datapool.h"

Game                      GAME;
Data_pool<Terrain>        TERRAIN;
Data_pool<World_terrain>  WORLD_TERRAIN;

void load_global_data()
{
  TERRAIN.load_from("terrain.dat");
  WORLD_TERRAIN.load_from("world_terrain.dat");
}
