#include "field.h"
#include "globals.h"
#include "datapool.h"
#include "files.h"
#include "game.h"
#include "map.h"

Game                      GAME;
Data_pool<Terrain>        TERRAIN;
Data_pool<World_terrain>  WORLD_TERRAIN;
Data_pool<Item_type>      ITEM_TYPES;
Data_pool<Item_group>     ITEM_GROUPS;
Data_pool<Monster_genus>  MONSTER_GENERA;
Data_pool<Monster_type>   MONSTER_TYPES;
Data_pool<Biome>          BIOMES;
Data_pool<Field_type>     FIELD_TYPES;
Submap_pool               SUBMAP_POOL;
Mapgen_spec_pool          MAPGEN_SPECS;
Keybinding_pool           KEYBINDINGS;

void load_global_data()
{
  TERRAIN.load_from       ("data/terrain.dat"       );
  WORLD_TERRAIN.load_from ("data/world_terrain.dat" );
  FIELD_TYPES.load_from   ("data/fields.dat"        );
  ITEM_TYPES.load_from    ("data/items.dat"         );
  ITEM_GROUPS.load_from   ("data/item_groups.dat"   );
  MONSTER_GENERA.load_from("data/monster_genera.dat");
  MONSTER_TYPES.load_from ("data/monsters.dat"      );
  BIOMES.load_from        ("data/biomes.dat"        );
  KEYBINDINGS.load_from   ("data/keybindings.txt"   );

  load_mapgen_specs();
}

void load_mapgen_specs()
{
  std::vector<std::string> mapgen_files = files_in("data/mapgen", "map");
  for (int i = 0; i < mapgen_files.size(); i++) {
    std::string filename = "data/mapgen/" + mapgen_files[i];
    MAPGEN_SPECS.load_from(filename);
  }

// Now confirm we have a mapgen_spec for every WORLD_TERRAIN
  for (std::list<World_terrain*>::iterator it = WORLD_TERRAIN.instances.begin();
       it != WORLD_TERRAIN.instances.end();
       it++) {
    if (MAPGEN_SPECS.lookup_terrain_ptr( (*it) ).empty()) {
      debugmsg("No mapgen specs for %s!", (*it)->name.c_str());
    }
  }
}
