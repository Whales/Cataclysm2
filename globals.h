#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include "datapool.h"
#include "terrain.h"
#include "world_terrain.h"
#include "mapgen.h"
#include "item_type.h"
#include "monster_type.h"
#include "keybind.h"


class Game;

extern Game                     GAME;
extern Data_pool<Terrain>       TERRAIN;
extern Data_pool<World_terrain> WORLD_TERRAIN;
extern Data_pool<Item_type>     ITEM_TYPES;
extern Data_pool<Monster_type>  MONSTER_TYPES;
extern Mapgen_spec_pool         MAPGEN_SPECS;
extern Keybinding_pool          KEYBINDINGS;

void load_global_data();
void load_mapgen_specs();

#endif
