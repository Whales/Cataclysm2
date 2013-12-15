#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include "game.h"
#include "datapool.h"
#include "terrain.h"
#include "world_terrain.h"

extern Game                     GAME;
extern Data_pool<Terrain>       TERRAIN;
extern Data_pool<World_terrain> WORLD_TERRAIN;

void load_global_data();

#endif
