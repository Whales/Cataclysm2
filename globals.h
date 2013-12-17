#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include "game.h"
#include "datapool.h"
#include "terrain.h"
#include "world_terrain.h"
#include "mapgen.h"

extern Game                     GAME;
extern Data_pool<Terrain>       TERRAIN;
extern Data_pool<World_terrain> WORLD_TERRAIN;
extern Mapgen_spec_pool         MAPGEN_SPECS;

void load_global_data();
void load_mapgen_specs();

#endif
