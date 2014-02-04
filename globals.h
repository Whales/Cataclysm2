#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include "datapool.h"
#include "terrain.h"
#include "world_terrain.h"
#include "mapgen.h"
#include "item_type.h"
#include "monster_type.h"
#include "keybind.h"
#include "biome.h"

class Game;
class Submap_pool;
class Field_type;

extern Game                     GAME;
extern Data_pool<Terrain>       TERRAIN;
extern Data_pool<World_terrain> WORLD_TERRAIN;
extern Data_pool<Item_type>     ITEM_TYPES;
extern Data_pool<Item_group>    ITEM_GROUPS;
extern Data_pool<Monster_genus> MONSTER_GENERA;
extern Data_pool<Monster_type>  MONSTER_TYPES;
extern Data_pool<Biome>         BIOMES;
extern Data_pool<Field_type>    FIELD_TYPES;
extern Submap_pool              SUBMAP_POOL;
extern Mapgen_spec_pool         MAPGEN_SPECS;
extern Keybinding_pool          KEYBINDINGS;

void load_global_data();
void load_mapgen_specs();
void init_default_keybindings();

#endif
