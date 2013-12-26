#include "globals.h"
#include "datapool.h"
#include "files.h"
#include "game.h"

Game                      GAME;
Data_pool<Terrain>        TERRAIN;
Data_pool<World_terrain>  WORLD_TERRAIN;
Data_pool<Item_type>      ITEM_TYPES;
Data_pool<Monster_type>   MONSTER_TYPES;
Mapgen_spec_pool          MAPGEN_SPECS;
Keybinding_pool           KEYBINDINGS;

void load_global_data()
{
  TERRAIN.load_from("data/terrain.dat");
  WORLD_TERRAIN.load_from("data/world_terrain.dat");
  ITEM_TYPES.load_from("data/items.dat");
  MONSTER_TYPES.load_from("data/monsters.dat");
  KEYBINDINGS.load_from("data/keybindings.txt");

  load_mapgen_specs();
}

void load_mapgen_specs()
{
  std::vector<std::string> mapgen_files = files_in("data/mapgen", "map");
  for (int i = 0; i < mapgen_files.size(); i++) {
    std::string filename = "data/mapgen/" + mapgen_files[i];
    MAPGEN_SPECS.load_from(filename);
  }
}
