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
#include "profession.h"
#include "mission.h"

class Game;
class Submap_pool;
class Field_type;

extern int TESTING_MODE;

extern Game                         GAME;
extern Data_pool<Terrain>           TERRAIN;
extern Data_pool<World_terrain>     WORLD_TERRAIN;
extern Data_pool<Item_type>         ITEM_TYPES;
extern Data_pool<Item_group>        ITEM_GROUPS;
extern Data_pool<Monster_genus>     MONSTER_GENERA;
extern Data_pool<Monster_type>      MONSTER_TYPES;
extern Data_pool<Biome>             BIOMES;
extern Data_pool<Field_type>        FIELDS;
extern Data_pool<Furniture_type>    FURNITURE_TYPES;
extern Data_pool<Profession>        PROFESSIONS;
extern Submap_pool                  SUBMAP_POOL;
extern Mapgen_spec_pool             MAPGEN_SPECS;
extern Keybinding_pool              KEYBINDINGS;
extern Data_pool<Mission_template>  MISSIONS;

void load_global_data();
void load_mapgen_specs();
void init_missions(); // Autogenerate missions pool from existing items/monsters
void init_default_keybindings();

/* For now, prep_directories() just handles SAVE_DIR & its subfolders; CUSS_DIR
 * and DATA_DIR actually need their contents to be there before the game starts,
 * so it doesn't make sense to set up their folder structure, since unless the
 * files are where they need to be, the game will fail to start.
 * SAVE_DIR, on the other hand, is not required when the game launches, so we'll
 * need to make sure that it, and its contents, exist.
 */
bool prep_directories();

#endif
