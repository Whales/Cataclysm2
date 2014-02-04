#include "field.h"
#include "map.h"
#include "rng.h"
#include "globals.h"
#include "monster.h"
#include "game.h"
#include "attack.h"
#include <fstream>

void Tile::set_terrain(Terrain* ter)
{
  if (!ter) {
    return;
  }
  terrain = ter;
  hp = ter->hp;
}

glyph Tile::top_glyph()
{
  if (field.is_valid()) {
    return field.top_glyph();
  }
  if (!items.empty()) {
    if (terrain && !terrain->has_flag(TF_FLOOR)) {
      return terrain->sym.hilite(c_blue);
    }
    return items.back().top_glyph();
  }
  if (!terrain) {
    return glyph();
  }
  glyph ret = terrain->sym;
  if (is_smashable() && terrain->hp > 0 && hp < terrain->hp) {
    int percent = (100 * hp) / terrain->hp;
    if (percent >= 80) {
      ret = ret.hilite(c_green);
    } else if (percent >= 40) {
      ret = ret.hilite(c_brown);
    } else {
      ret = ret.hilite(c_red);
    }
  }
  return ret;
}

int Tile::move_cost()
{
  if (!terrain) {
    return 0;
  }
  return (terrain->movecost);
}

std::string Tile::get_name()
{
  if (!terrain) {
    return "Unknown";
  }
  return (terrain->get_name());
}

bool Tile::blocks_sense(Sense_type sense)
{
  if (!terrain) {
    return false;
  }
  switch (sense) {
    case SENSE_NULL:
      return true;
    case SENSE_SIGHT:
      return (has_flag(TF_OPAQUE));
    case SENSE_SOUND:
      return false;
    case SENSE_ECHOLOCATION:
      return (move_cost() == 0);
    case SENSE_SMELL:
      return (move_cost() == 0);
    case SENSE_OMNISCIENT:
      return false;
    case SENSE_MAX:
      return false;
  }
  return false;
}

bool Tile::has_flag(Terrain_flag flag)
{
  if (field.is_valid() && field.has_flag(flag)) {
    return true;
  }
  if (!terrain) {
    return false;
  }
  return terrain->has_flag(flag);
}

bool Tile::has_field()
{
  return field.is_valid();
}

bool Tile::is_smashable()
{
  return (terrain && terrain->can_smash());
}

std::string Tile::smash(Damage_set dam)
{
  if (!is_smashable()) {  // This verifies that terrain != NULL
    return "";
  }
  Terrain_smash smash = terrain->smash;
  if (rng(1, 100) <= smash.ignore_chance) {
    return smash.failure_sound; // Make our "saving throw"
  }
  if (damage(dam)) {
    return smash.success_sound;
  }
  return smash.failure_sound;
}

bool Tile::damage(Damage_set dam)
{
  bool destroyed = false;
  for (int i = 0; i < DAMAGE_MAX; i++) {
    Damage_type type = Damage_type(i);
    int dmg = dam.get_damage(type);
/* We don't return immediately upon getting true back from damage(type, dmg)
 * because we need to apply the rest of the damage types to the result.
 */
    if (damage(type, dmg)) {
      destroyed = true;
    }
  }
  return destroyed;
}

bool Tile::damage(Damage_type type, int dam)
{
  if (dam <= 0) {
    return false;
  }
  if (!terrain) {
    return false;
  }
  int armor = terrain->smash.armor[type];
  dam -= rng(armor / 2, armor);
  if (dam <= 0) {
    return false;
  }
  hp -= dam;
  if (hp < 0) {
// If HP is negative, then we run damage *again* with the extra damage
    int extra = 0 - hp;
    Terrain* result = TERRAIN.lookup_name( terrain->destroy_result );
    if (!result) {
      debugmsg("Tried to destroy '%s' but couldn't look up result '%s'.",
               get_name().c_str(), terrain->destroy_result.c_str());
    } else {
      set_terrain(result);
      damage(type, extra);  // See above
    }
    return true;
  }
  return false;
}

void Tile::open()
{
  if (!terrain || !terrain->can_open()) {
    return;
  }
  Terrain* result = TERRAIN.lookup_name( terrain->open_result );
  if (!result) {
    debugmsg("Failed to find terrain '%s'", terrain->open_result.c_str());
    return;
  }
  terrain = result;
}

void Tile::close()
{
  if (!terrain || !terrain->can_close()) {
    return;
  }
  Terrain* result = TERRAIN.lookup_name( terrain->close_result );
  if (!result) {
    debugmsg("Failed to find terrain '%s'", terrain->close_result.c_str());
    return;
  }
  terrain = result;
}

Submap::Submap()
{
  rotation = DIR_NULL;
  level = 0;
}

Submap::~Submap()
{
}

void Submap::generate_empty()
{
  Terrain* grass = TERRAIN.lookup_name("grass");
  Terrain* dirt  = TERRAIN.lookup_name("dirt");
  if (!grass || !dirt) {
    debugmsg("Couldn't find terrain for generate_empty()");
    return;
  }

  for (int x = 0; x < SUBMAP_SIZE; x++) {
    for (int y = 0; y < SUBMAP_SIZE; y++) {
      tiles[x][y].set_terrain(one_in(2) ? grass : dirt);
    }
  }
}

void Submap::generate_open()
{
  Terrain* open = TERRAIN.lookup_name("empty");
  if (!open) {
    debugmsg("Couldn't find terrain 'empty'; Submap::generate_open()");
    return;
  }

  for (int x = 0; x < SUBMAP_SIZE; x++) {
    for (int y = 0; y < SUBMAP_SIZE; y++) {
      tiles[x][y].set_terrain(open);
    }
  }
}

void Submap::generate(Worldmap* map, int posx, int posy, int posz)
{
  if (!map) {
    debugmsg("Submap::generate(NULL, %d, %d)", posx, posy);
    generate_empty();
    return;
  }
  Worldmap_tile *tile = map->get_tile(posx, posy);
  if (!tile) {
    generate_empty();
    return;
  }
  World_terrain* ter[5];
  ter[0] = tile->terrain;
// North
  tile = map->get_tile(posx, posy - 1);
  ter[1] = (tile ? tile->terrain : NULL);
// East
  tile = map->get_tile(posx + 1, posy);
  ter[2] = (tile ? tile->terrain : NULL);
// South
  tile = map->get_tile(posx, posy + 1);
  ter[3] = (tile ? tile->terrain : NULL);
// West
  tile = map->get_tile(posx - 1, posy);
  ter[4] = (tile ? tile->terrain : NULL);

  generate(ter, posz);
}

void Submap::generate(World_terrain* terrain[5], int posz)
{
  if (!terrain[0]) {
    generate_empty();
  } else {
    Mapgen_spec* spec = NULL;
// We shouldn't ever hit this; Mapgen_pool handles above-ground.  But safety!
    if (posz > 0) {
      generate_open();
    } else if (terrain[0]->has_flag(WTF_RELATIONAL)) {
      std::vector<bool> neighbor;
      neighbor.push_back(false);
      for (int i = 1; i < 5; i++) {
        if (terrain[i] == terrain[0]) {
          neighbor.push_back(true);
        } else {
          neighbor.push_back(false);
        }
      }
      spec = MAPGEN_SPECS.random_for_terrain(terrain[0], neighbor);
    } else {
      spec = MAPGEN_SPECS.random_for_terrain(terrain[0], "", 0);
    }
    if (!spec) {
      debugmsg("Mapgen::generate() failed to find spec for %s [%d]",
               terrain[0]->get_name().c_str(), posz);
      generate_empty();
      return;
    }
    spec->prepare(terrain);
    generate( spec );
  }

// If we're above ground, DON'T do adjacency maps!
  if (posz > 0) {
    return;
  }

// Now do adjacency maps
  for (int i = 1; i < 5; i++) {
    if (terrain[i] && terrain[i] != terrain[0]) {
      Mapgen_spec* adj = MAPGEN_SPECS.random_adjacent_to(terrain[i]);
      if (adj) {
        adj->prepare(terrain);
        adj->rotate( Direction(i) );
        generate_adjacent( adj );
      }
    }
  }
}

void Submap::generate(std::string terrain_name)
{
  generate( MAPGEN_SPECS.random_for_terrain( terrain_name ) );
}

void Submap::generate(Mapgen_spec* spec)
{
  if (!spec) {
    debugmsg("Null spec in Submap::generate()!");
    generate_empty();
    return;
  }
// Set our subname to the spec's subname (defaults to empty, only matters for
// multi-story buildings
// Ditto rotation.
  subname = spec->subname;
  rotation = spec->rotation;
// First, set the terrain.
  for (int x = 0; x < SUBMAP_SIZE; x++) {
    for (int y = 0; y < SUBMAP_SIZE; y++) {
      tiles[x][y].set_terrain(spec->pick_terrain(x, y));
    }
  }
// Next, add items.
  for (std::map<char,Item_area>::iterator it = spec->item_defs.begin();
       it != spec->item_defs.end();
       it++) {
    Item_area* area = &(it->second);
    while (area && area->place_item()) {
      Point p = area->pick_location();
      Item item( area->pick_type() );
      add_item(item, p.x, p.y);
    }
  }
}

void Submap::generate_adjacent(Mapgen_spec* spec)
{
  if (spec == NULL) {
    return;
  }
// First, set the terrain.
  for (int x = 0; x < SUBMAP_SIZE; x++) {
    for (int y = 0; y < SUBMAP_SIZE; y++) {
      Terrain* tmpter = spec->pick_terrain(x, y);
// TODO: Only overwrite terrain with the "ground" tag
      if (tmpter &&
          (!tiles[x][y].terrain || tiles[x][y].terrain->has_flag(TF_FLOOR))) {
        tiles[x][y].set_terrain(tmpter);
      }
    }
  }
// Next, add items.
  for (std::map<char,Item_area>::iterator it = spec->item_defs.begin();
       it != spec->item_defs.end();
       it++) {
    Item_area* area = &(it->second);
    while (area && area->place_item()) {
      Point p = area->pick_location();
      Item item( area->pick_type() );
      tiles[p.x][p.y].items.push_back(item);
    }
  }
}

void Submap::generate_above(World_terrain* type, Submap* below)
{
  if (!type) {
    debugmsg("Submap::generate_above(NULL, ?) called!");
    generate_empty();
    return;
  }
  if (!below) {
    debugmsg("Submap::generate_above(?, NULL) called!");
    generate_empty();
    return;
  }

  level = below->level + 1;
  subname = below->subname;
  rotation = below->rotation;

  Mapgen_spec* spec = MAPGEN_SPECS.random_with_subname(subname, level);
  if (!spec) {
    generate_open();
    return;
  }
  World_terrain* ter[5];
  ter[0] = type;
  for (int i = 0; i < 5; i++) {
    ter[i] = NULL;
  }
  spec->prepare(ter);
  spec->rotate(rotation);
  generate(spec);
}

void Submap::spawn_monsters(Worldmap* world, int worldx, int worldy, int worldz,
                                             int smx, int smy)
{
  if (!world) {
    debugmsg("Submap::spawn_monsters() called with NULL world");
    return;
  }
  if (worldx < 0 || worldy < 0 ||
      worldx >= WORLDMAP_SIZE || worldy >= WORLDMAP_SIZE) {
    debugmsg("Submap::spawn_monsters() called for world position [%d:%d]",
             worldx, worldy);
    return;
  }
  std::vector<Monster_spawn>* monsters = world->get_spawns(worldx, worldy);
  if (monsters->empty()) {
    return;
  }
  std::vector<Point> valid_points;
  for (int x = 0; x < SUBMAP_SIZE; x++) {
    for (int y = 0; y < SUBMAP_SIZE; y++) {
// TODO: Allow swimming mosnters to spawn in water, etc.
      if (tiles[x][y].move_cost() > 0) {
        valid_points.push_back( Point(x, y) );
      }
    }
  }

// Iterate through all the spawn data on this tile
  for (int i = 0; i < monsters->size(); i++) {
// Proceed once for each population member
    while ((*monsters)[i].population > 0) {
      if (valid_points.empty()) { // The map is saturated with monsters!
        return;
      }
      Monster_type* type = (*monsters)[i].genus->random_member();
      int index = rng(0, valid_points.size());
      Point placement = valid_points[index];
      valid_points.erase( valid_points.begin() + index );
// Adjust placement to be relative to the map, not the submap
      placement.x += SUBMAP_SIZE * smx;
      placement.y += SUBMAP_SIZE * smy;
      Tripoint monpos(placement.x, placement.y, worldz);
      Monster* mon = new Monster;
      mon->set_type(type);
      mon->pos = monpos;
      GAME.entities.add_entity(mon);
      (*monsters)[i].population--;
    }
  }
}

bool Submap::add_item(Item item, int x, int y)
{
  if (x < 0 || y < 0 || x >= SUBMAP_SIZE || y >= SUBMAP_SIZE) {
    return false;
  }
  if (tiles[x][y].move_cost() > 0) {
    tiles[x][y].items.push_back(item);
  } else {
// Pick a random adjacent space with move_cost != 0
    std::vector<Point> valid_points;
    for (int px = x - 1; px <= x + 1; px++) {
      for (int py = y - 1; py <= y + 1; py++) {
        if (px >= 0 && py >= 0 && px < SUBMAP_SIZE && py < SUBMAP_SIZE &&
            tiles[px][py].move_cost() > 0) {
          valid_points.push_back( Point(px, py) );
        }
      }
    }
    if (valid_points.empty()) {
      return false; // No valid points!  Oh well.
    }
    int index = rng(0, valid_points.size() - 1);
    Point p = valid_points[index];
    tiles[p.x][p.y].items.push_back(item);
  }
  return true;
}

int Submap::item_count(int x, int y)
{
  if (x < 0 || x >= SUBMAP_SIZE || x < 0 || y >= SUBMAP_SIZE) {
    return 0;
  }
  return tiles[x][y].items.size();
}

std::vector<Item>* Submap::items_at(int x, int y)
{
  if (x < 0 || x >= SUBMAP_SIZE || x < 0 || y >= SUBMAP_SIZE) {
    return NULL;
  }
  return &(tiles[x][y].items);
}

Point Submap::random_empty_tile()
{
  std::vector<Point> options;
  for (int x = 0; x < SUBMAP_SIZE; x++) {
    for (int y = 0; y < SUBMAP_SIZE; y++) {
      debugmsg("Trying tile [%d:%d] (%s)", x, y, tiles[x][y].get_name().c_str());
      if (tiles[x][y].move_cost() > 0) {
        options.push_back( Point(x, y) );
      }
    }
  }

  if (options.empty()) {
    return Point(-1, -1);
  }
  return options[ rng(0, options.size() - 1) ];
}

Submap_pool::Submap_pool()
{
}

Submap_pool::~Submap_pool()
{
  for (std::list<Submap*>::iterator it = instances.begin();
       it != instances.end();
       it++) {
    delete (*it);
  }
}

Submap* Submap_pool::at_location(int x, int y, int z)
{
  return at_location( Tripoint(x, y, z) );
}

Submap* Submap_pool::at_location(Point p)
{
  Tripoint trip(p.x, p.y, 0);
  return at_location(trip);
}

Submap* Submap_pool::at_location(Tripoint p)
{
  if (point_map.count(p) > 0) {
    return point_map[p];
  }
  Submap* sub = new Submap;
  if (p.z > 0) {
    Submap* below = at_location(p.x, p.y, p.z - 1);
    Worldmap_tile *tile = GAME.worldmap->get_tile(p.x, p.y);
    if (!tile) {
      sub->generate_empty();
      return sub;
    }
    sub->generate_above(tile->terrain, below);
    return sub;
  }
  sub->generate(GAME.worldmap, p.x, p.y, p.z);
  point_map[p] = sub;
  return sub;
}

Map::Map()
{
  posx = 0;
  posy = 0;
  posz = 0;
}

Map::~Map()
{
}

void Map::generate_empty()
{
  for (int x = 0; x < MAP_SIZE; x++) {
    for (int y = 0; y < MAP_SIZE; y++) {
      submaps[x][y][posz]->generate_empty();
    }
  }
}

void Map::test_generate(std::string terrain_name)
{
  for (int x = 0; x < MAP_SIZE; x++) {
    for (int y = 0; y < MAP_SIZE; y++) {
      submaps[x][y][posz]->generate(terrain_name);
    }
  }
}

void Map::generate(Worldmap *world, int wposx, int wposy, int wposz)
{
// All arguments default to -999
  if (wposx != -999) {
    posx = wposx;
  }
  if (wposy != -999) {
    posy = wposy;
  }
  if (wposz != -999) {
    posz = wposz;
  }
// TODO: Support posz < 0
  for (int z = 0; z <= posz; z++) {
    int z_index = z + VERTICAL_MAP_SIZE - posz;
    for (int x = 0; x < MAP_SIZE; x++) {
      for (int y = 0; y < MAP_SIZE; y++) {
        submaps[x][y][z_index] = SUBMAP_POOL.at_location(posx + x, posy + y, z);
        if (z == 0) {
          submaps[x][y][z_index]->spawn_monsters(world, posx+x, posy+y, 0,
                                                 x, y);
        }
      }
    }
  }
}

void Map::shift(Worldmap *world, int shiftx, int shifty, int shiftz)
{
  if (shiftx == 0 && shifty == 0 && shiftz == 0) {
    return;
  }
  posx += shiftx;
  posy += shifty;
  posz += shiftz;
  generate(world);

// Also, handle generating monsters for any new submaps!
/*
  int x_start = (shiftx > 0 ? MAP_SIZE - shiftx : 0         );
  int y_start = (shifty > 0 ? MAP_SIZE - shifty : 0         );
  int x_end   = (shiftx > 0 ? MAP_SIZE - 1      : 0 - shiftx);
  int y_end   = (shifty > 0 ? MAP_SIZE - 1      : 0 - shifty);
  for (int x = x_start; x <= x_end; x++) {
    for (int y = y_start; y <= y_end; y++) {
      spawn_monsters(world, x, y);
    }
  }
*/
}

void Map::spawn_monsters(Worldmap *world, int x, int y)
{
  if (!world) {
    debugmsg("Map::spawn_monsters() called with NULL world");
    return;
  }
  if (x < 0 || y < 0 || x >= MAP_SIZE || y >= MAP_SIZE) {
    debugmsg("Map::spawn_monsters() called on submap [%d:%d]", x, y);
    return;
  }
  int monx = posx + x, mony = posx + y;
  std::vector<Monster_spawn>* monsters = world->get_spawns(monx, mony);

  if (monsters->size() > 0) {
    debugmsg("Spawning monsters: [%d:%d] / [%d:%d] (%d)", x, y, posx+x, posy+y,
             monsters->size());
  }
  for (int i = 0; i < monsters->size(); i++) {
    for (int n = 0; n < (*monsters)[i].population; n++) {
      Monster_type* type = (*monsters)[i].genus->random_member();
/* TODO: Support placing aquatic monsters in water tiles, etc.
 *       Perhaps by replacing random_empty_tile() with tile_for(Monster_type*)?
 */
      debugmsg("%d:%d:%d => %d", x, y, posz, submaps[x][y][posz]);
      Point tmppos = submaps[x][y][posz]->random_empty_tile();
      if (tmppos.x >= 0 && tmppos.y <= 0) {
// Adjust to be relative to the map, not the submap
        tmppos.x += SUBMAP_SIZE * x;
        tmppos.y += SUBMAP_SIZE * y;
        Tripoint monpos(tmppos.x, tmppos.y, posz);
        Monster* mon = new Monster;
        mon->set_type(type);
        mon->pos = monpos;
        GAME.entities.add_entity(mon);
        debugmsg("Spawned %s", mon->get_name().c_str());
      }
    }
    (*monsters)[i].population = 0;
  }
}

Generic_map Map::get_movement_map(Entity_AI AI,
                                  Tripoint origin, Tripoint target)
{
// Set the bounds of the map
  int min_x = (origin.x < target.x ? origin.x : target.x);
  int min_y = (origin.y < target.y ? origin.y : target.y);
  int min_z = (origin.z < target.z ? origin.z : target.z);
  int max_x = (origin.x > target.x ? origin.x : target.x);
  int max_y = (origin.y > target.y ? origin.y : target.y);
  int max_z = (origin.z > target.z ? origin.z : target.z);

// Expand the bounds of the map by our area awareness bonus.
  min_x -= AI.area_awareness;
  min_y -= AI.area_awareness;
  min_z -= AI.area_awareness;
  max_x += AI.area_awareness;
  max_y += AI.area_awareness;
  max_z += AI.area_awareness;

  int x_size = 1 + max_x - min_x;
  int y_size = 1 + max_y - min_y;
  int z_size = 1 + max_z - min_z;

  Generic_map ret(x_size, y_size, z_size);
  ret.x_offset = min_x;
  ret.y_offset = min_y;
  ret.z_offset = min_z;

  for (int x = min_x; x <= max_x; x++) {
    for (int y = min_y; y <= max_y; y++) {
      for (int z = min_z; z <= max_z; z++) {
        int map_x = x - min_x;
        int map_y = y - min_y;
        int map_z = z - min_z;
        int cost = move_cost(x, y);
// TODO: If there's a field here, increase cost accordingly
        if (cost == 0 && is_smashable(x, y)) {
          cost = 500; // TODO: Estimate costs more intelligently
        }
        ret.set_cost(map_x, map_y, map_z, cost);
      }
    }
  }

  return ret;
}

Generic_map Map::get_dijkstra_map(Tripoint target, int weight,
                                  bool include_smashable)
{
  Generic_map ret(SUBMAP_SIZE * MAP_SIZE, SUBMAP_SIZE * MAP_SIZE, posz + 1);
  ret.set_cost(target, weight);
  std::vector<Tripoint> active;
  active.push_back(target);
  while (!active.empty()) {
    Tripoint cur = active[0];
    active.erase(active.begin());
// Check all adjacent terrain
    for (int x = cur.x - 1; x <= cur.x + 1; x++) {
      for (int y = cur.y - 1; y <= cur.y + 1; y++) {
        if (x == cur.x && y == cur.y) { // Skip our own cell
          y++;
        }
        if (((include_smashable && is_smashable(x, y, cur.z)) ||
             move_cost(x, y, cur.z) > 0) &&
            ret.get_cost(x, y, cur.z) < ret.get_cost(cur) - 1) {
          ret.set_cost(x, y, cur.z, ret.get_cost(cur) - 1);
          active.push_back( Tripoint(x, y, cur.z) );
        }
      }
    }
    if (has_flag(TF_STAIRS_DOWN, cur)) {
      Tripoint down(cur.x, cur.y, cur.z - 1);
      if (ret.get_cost(down) < ret.get_cost(cur) - 1) {
        ret.set_cost(down, ret.get_cost(cur) - 1);
        active.push_back( down );
      }
    }
    if (has_flag(TF_STAIRS_UP, cur)) {
      Tripoint down(cur.x, cur.y, cur.z + 1);
      if (ret.get_cost(down) < ret.get_cost(cur) - 1) {
        ret.set_cost(down, ret.get_cost(cur) - 1);
        active.push_back( down );
      }
    }
  } // while (!active.empty())
  return ret;
}

int Map::move_cost(Tripoint pos)
{
  return move_cost(pos.x, pos.y, pos.z);
}

int Map::move_cost(int x, int y, int z)
{
  Tile *t = get_tile(x, y, z);
  if (!t) {
    return 100;
  }
  return t->move_cost();
}

bool Map::is_smashable(Tripoint pos)
{
  return is_smashable(pos.x, pos.y, pos.z);
}

bool Map::is_smashable(int x, int y, int z)
{
  Tile *t = get_tile(x, y, z);
  if (!t) {
    return false;
  }
  return t->is_smashable();
}

bool Map::has_flag(Terrain_flag flag, Tripoint pos)
{
  return has_flag(flag, pos.x, pos.y, pos.z);
}

bool Map::has_flag(Terrain_flag flag, int x, int y, int z)
{
  Tile *t = get_tile(x, y, z);
  return t->has_flag(flag);
}

bool Map::blocks_sense(Sense_type sense, Tripoint pos)
{
  return blocks_sense(sense, pos.x, pos.y, pos.z);
}

bool Map::blocks_sense(Sense_type sense, int x, int y, int z)
{
  Tile *t = get_tile(x, y, z);
  return t->blocks_sense(sense);
}

bool Map::add_item(Item item, Tripoint pos)
{
  return add_item(item, pos.x, pos.y, pos.z);
}

bool Map::add_item(Item item, int x, int y, int z)
{
  if (z == 999) { // z defaults to 999
    z = posz;
  }
  z = z - posz + VERTICAL_MAP_SIZE;
  if (x < 0 || x >= SUBMAP_SIZE * MAP_SIZE ||
      y < 0 || y >= SUBMAP_SIZE * MAP_SIZE ||
      z < 0 || z >= VERTICAL_MAP_SIZE * 2 + 1) {
    return false;
  }
  int sx = x / SUBMAP_SIZE, sy = y / SUBMAP_SIZE;
  x %= SUBMAP_SIZE;
  y %= SUBMAP_SIZE;
  return submaps[sx][sy][z]->add_item(item, x, y);
}

bool Map::add_field(Field_type* type, Tripoint pos, std::string creator)
{
  return add_field(type, pos.x, pos.y, pos.z);
}

bool Map::add_field(Field_type* type, int x, int y, int z, std::string creator)
{
  Field field(type, 1, creator);
  return add_field(field, x, y, z);
}

bool Map::add_field(Field field, Tripoint pos)
{
  return add_field(field, pos.x, pos.y, pos.z);
}

bool Map::add_field(Field field, int x, int y, int z)
{
  Tile* tile = get_tile(x, y, z);
  if (tile->has_field()) {
// We can combine fields of the same type
    tile->field += field;
    return true;
  }
  if (tile->move_cost() == 0 && !field.has_flag(FIELD_FLAG_SOLID)) {
    return false;
  }
  tile->field = field;
  return true;
}

int Map::item_count(Tripoint pos)
{
  return item_count(pos.x, pos.y, pos.z);
}

int Map::item_count(int x, int y, int z)
{
  std::vector<Item>* it = items_at(x, y, z);
  if (!it) {
    return 0;
  }
  return it->size();
}

std::vector<Item>* Map::items_at(Tripoint pos)
{
  return items_at(pos.x, pos.y, pos.z);
}

std::vector<Item>* Map::items_at(int x, int y, int z)
{
  if (z == 999) { // z defaults to 999
    z = posz;
  }
  z = z - posz + VERTICAL_MAP_SIZE;
  if (x < 0 || x >= SUBMAP_SIZE * MAP_SIZE ||
      y < 0 || y >= SUBMAP_SIZE * MAP_SIZE ||
      z < 0 || z >= VERTICAL_MAP_SIZE * 2 + 1) {
    return NULL;
  }
  int sx = x / SUBMAP_SIZE, sy = y / SUBMAP_SIZE;
  x %= SUBMAP_SIZE;
  y %= SUBMAP_SIZE;
  return submaps[sx][sy][z]->items_at(x, y);
}

bool Map::contains_field(Tripoint pos)
{
  return contains_field(pos.x, pos.y, pos.z);
}

bool Map::contains_field(int x, int y, int z)
{
  return (get_tile(x, y, z)->has_field());
}

Field* Map::field_at(Tripoint pos)
{
  return field_at(pos.x, pos.y, pos.z);
}

Field* Map::field_at(int x, int y, int z)
{
  Tile* tile = get_tile(x, y, z);
  return &(tile->field);
}

int Map::field_uid_at(Tripoint pos)
{
  return field_uid_at(pos.x, pos.y, pos.z);
}

int Map::field_uid_at(int x, int y, int z)
{
  Field* tmp = field_at(x, y, z);
  if (tmp->level <= 0) {
    return -1;
  }
  return tmp->get_type_uid();
}

Tile* Map::get_tile(Tripoint pos)
{
  return get_tile(pos.x, pos.y, pos.z);
}

Tile* Map::get_tile(int x, int y, int z)
{
// TODO: Set all fields, traps, etc. on tile_oob to "nothing"
  if (z == 999) { // z defaults to 999
    z = posz;
  }
  z = z - posz + VERTICAL_MAP_SIZE;
  if (x < 0 || x >= SUBMAP_SIZE * MAP_SIZE ||
      y < 0 || y >= SUBMAP_SIZE * MAP_SIZE ||
      z < 0 || z >= VERTICAL_MAP_SIZE * 2 + 1 ) {
    tile_oob.set_terrain(TERRAIN.lookup_uid(0));
    tile_oob.field.level = 0;
    return &tile_oob;
  }

  int sx = x / SUBMAP_SIZE, sy = y / SUBMAP_SIZE;
  return &(submaps[sx][sy][z]->tiles[x % SUBMAP_SIZE][y % SUBMAP_SIZE]);
}

std::string Map::get_name(Tripoint pos)
{
  return get_name(pos.x, pos.y, pos.z);
}

std::string Map::get_name(int x, int y, int z)
{
  Terrain* ter = get_tile(x, y, z)->terrain;
  if (!ter) {
    return "Bug - Null terrain";
  }
  return ter->get_name();
}

void Map::smash(int x, int y, Damage_set dam, bool make_sound)
{
  return smash(x, y, 999, dam, make_sound);
}

void Map::smash(int x, int y, int z, Damage_set dam, bool make_sound)
{
  Tile* hit = get_tile(x, y, z);
  if (hit) {
    std::string sound = hit->smash(dam);
    if (make_sound) {
      GAME.make_sound(sound, x, y);
    }
  }
}

void Map::smash(Tripoint pos, Damage_set dam, bool make_sound)
{
  return smash(pos.x, pos.y, pos.z, dam);
}

void Map::damage(int x, int y, Damage_set dam)
{
  damage(x, y, 999, dam);
}

void Map::damage(int x, int y, int z, Damage_set dam)
{
  Tile* hit = get_tile(x, y, z);
  if (hit) {
    hit->damage(dam);
  }
}

void Map::damage(Tripoint pos, Damage_set dam)
{
  damage(pos.x, pos.y, pos.z, dam);
}

bool Map::open(Tripoint pos)
{
  return open(pos.x, pos.y, pos.y);
}

bool Map::open(int x, int y, int z)
{
  Tile* target = get_tile(x, y, z);
  if (target->terrain->can_open()) {
    target->open();
    return true;
  }
  return false;
}

bool Map::close(Tripoint pos)
{
  return close(pos.x, pos.y, pos.z);
}

bool Map::close(int x, int y, int z)
{
  Tile* target = get_tile(x, y, z);
  if (target->terrain->can_close()) {
    target->close();
    return true;
  }
  return false;
}

/* TODO:  We should track currently-active fields in a list of points.  At
 *        present, we check *all* tiles for an active field.  This is probably
 *        inefficient.
 */
void Map::process_fields()
{
/* TODO:  Won't work below ground level.
 * TODO:  Since we start at the upper-left and work our way down & right, fields
 *        to the north-west will have a better chance of spreading than fields
 *        to the south-east.  Best way to fix this is to create a output map of
 *        fields, the copy that output map back to this after processing is
 *        done.
 */
  for (int x = 0; x < SUBMAP_SIZE * MAP_SIZE; x++) {
    for (int y = 0; y < SUBMAP_SIZE * MAP_SIZE; y++) {
      for (int z = 0; z <= posz; z++) {
        Field* field = field_at(x, y, z);
        if (!field) {
          debugmsg("Somehow encountered NULL field at [%d:%d:%d]", x, y, z);
          return;
        }
        if (field->is_valid()) {
          Entity* ent = GAME.entities.entity_at(x, y, z);
          if (ent) {
            field->hit_entity(ent);
          }
          field->process(this, Tripoint(x, y, z));
        }
      }
    }
  }
}
          

/* Still using Cataclysm style LOS.  It sucks and is slow and I hate it.
 * Basically, iterate over all Bresenham lines between [x0,y0] and [x1,y1].
 * If any of the lines doesn't have something that blocks the relevent sense,
 * return true.  If we iterate through all of them and they all block, return
 * false.
 */
bool Map::senses(int x0, int y0, int x1, int y1, int range, Sense_type sense)
{
  return senses(x0, x0, posz, x1, y1, posz, range, sense);
}

bool Map::senses(int x0, int y0, int z0, int x1, int y1, int z1, int range,
                 Sense_type sense)
{
  if (x0 < 0 || y0 < 0 ||
      x1 >= SUBMAP_SIZE * MAP_SIZE || y1 >= SUBMAP_SIZE * MAP_SIZE) {
    return false;
  }
  if (rl_dist(x0, y0, z0, x1, y1, z1) > range) {
    return false;
  }
  if (sense == SENSE_SIGHT) {
    std::vector<Point> line = line_of_sight(x0, y0, z0, x1, y1, z1);
    return (!line.empty() && line.size() <= range);
  } else if (sense == SENSE_SMELL) {
// TODO: More realistic smell
    return (rl_dist(x0, y0, z0, x1, y1, z1) <= range);
  }
  return false;
}

bool Map::senses(Point origin, Point target, int range, Sense_type sense)
{
  return senses(origin.x, origin.y, posz, target.x, target.y, posz, range,
                sense);
}

bool Map::senses(Tripoint origin, Tripoint target, int range, Sense_type sense)
{
  return senses(origin.x, origin.y, origin.z, target.x, target.y, target.z,
                range, sense);
}

std::vector<Point> Map::line_of_sight(int x0, int y0, int x1, int y1)
{
  return line_of_sight(x0, y0, posz, x1, y1, posz);
}

std::vector<Point> Map::line_of_sight(int x0, int y0, int z0,
                                      int x1, int y1, int z1)
{
  std::vector<Point>  lines;    // Process many lines at once.
  std::vector<std::vector<Point> > return_values;
  std::vector<int>    t_values; // T-values for Bresenham lines

  int dx = x1 - x0, dy = y1 - y0, dz = z1 - z0;
  int ax = abs(dx) << 1, ay = abs(dy) << 1;
  int sx = (dx < 0 ? -1 : 1), sy = (dy < 0 ? -1 : 1);
  int dist = rl_dist(x0, y0, x1, y1);
  int z_step;
  if (dist == 0) {
    z_step = 0;
  } else {
    z_step = (100 * dz) / dist;
  }
  if (dx == 0) {
    sx = 0;
  }
  if (dy == 0) {
    sy = 0;
  }

  int min_t = (ax > ay ? ay - ax : ax - ay),
      max_t = 0;
  if (dx == 0 || dy == 0) {
    min_t = 0;
  }
// Init our "lines"
  std::vector<Point> seed;
  for (int t = min_t; t <= max_t; t++) {
    lines.push_back( Point(x0, y0) );
    return_values.push_back(seed);
    t_values.push_back(t);
  }
  int z_value = 0; // Each tile is 100 microunits tall
  int z_level = z0;
// Keep going as long as we've got at least one valid line
  while (!lines.empty()) {
// Since we track z_value universally, don't do it inside the for loop below
    z_value += z_step;
    if (z_value < 0) {
      z_level--;
      z_value += 100;
    } else if (z_value >= 100) {
      z_level++;
      z_value -= 100;
    }
    for (int i = 0; i < lines.size(); i++) {
      if (ax > ay) {
        lines[i].x += sx;
        if (t_values[i] >= 0) {
          lines[i].y += sy;
          t_values[i] -= ax;
        }
        t_values[i] += ay;
      } else {
        lines[i].y += sy;
        if (t_values[i] >= 0) {
          lines[i].x += sx;
          t_values[i] -= ay;
        }
        t_values[i] += ax;
      }
      return_values[i].push_back(lines[i]);
      if (lines[i].x == x1 && lines[i].y == y1) {
        return return_values[i];
      }
      if (blocks_sense(SENSE_SIGHT, lines[i].x, lines[i].y, z_level)) {
        lines.erase(lines.begin() + i);
        t_values.erase(t_values.begin() + i);
        return_values.erase(return_values.begin() + i);
        i--;
      }
    }
  }
  return std::vector<Point>();
}

std::vector<Point> Map::line_of_sight(Point origin, Point target)
{
  return line_of_sight(origin.x, origin.y, target.x, target.y);
}

std::vector<Point> Map::line_of_sight(Tripoint origin, Tripoint target)
{
  return line_of_sight(origin.x, origin.y, origin.z,
                       target.x, target.y, target.z);
}

void Map::draw(Window* w, Entity_pool *entities, Tripoint ref, Sense_type sense)
{
  draw(w, entities, ref.x, ref.y, ref.z, sense);
}

void Map::draw(Window* w, Entity_pool *entities, int refx, int refy, int refz,
               Sense_type sense)
{
  if (!w) {
    return;
  }
  int winx = w->sizex(), winy = w->sizey();
  int dist = winx > winy ? winx / 2 : winy / 2;
  for (int x = 0; x < winx; x++) {
    for (int y = 0; y < winy; y++) {
      int terx = refx + x - (winx / 2), tery = refy + y - (winy / 2);
      if (senses(refx, refy, refz, terx, tery, posz, dist, sense)) {
        draw_tile(w, entities, terx, tery, refx, refy, false);
      } else {
// TODO: Don't use a literal glyph!  TILES GEEZE
        w->putglyph(x, y, glyph(' ', c_black, c_black));
      }
    }
  }
}

void Map::draw_tile(Window* w, Entity_pool *entities, int tilex, int tiley,
                    int refx, int refy, bool invert)
{
  draw_tile(w, entities, tilex, tiley, posz, refx, refy, invert);
}

void Map::draw_tile(Window* w, Entity_pool *entities,
                    int tilex, int tiley, int tilez,
                    int refx, int refy, bool invert)
{
  if (!w) {
    return;
  }
  int winx = w->sizex(), winy = w->sizey();
  int centerx = winx / 2, centery = winy / 2;
  int dx = tilex - refx, dy = tiley - refy;
  int tile_winx = centerx + dx, tile_winy = centery + dy;
  if (tile_winx < 0 || tile_winx >= winx || tile_winy < 0 || tile_winy >= winy){
    return; // It won't fit in the window!
  }
// Now pick a glyph...
  glyph output;
  bool picked_glyph = false;
  int curz = tilez;
/* Start from the z-level that we're looking at.  As long as there's no entity,
 * and the terrain is open space, drop down a level.
 */
  while (!picked_glyph && curz >= 0) {
    if (entities) {
      Entity* ent = entities->entity_at(tilex, tiley, curz);
      if (ent) {
        output = ent->get_glyph();
        picked_glyph = true;
      }
    }
    if (!picked_glyph) {
      Tile* tile = get_tile(tilex, tiley, curz);
      if (!tile->has_flag(TF_OPEN_SPACE)) {
        output = tile->top_glyph();
        picked_glyph = true;
      }
    }
    if (picked_glyph) {
      if (curz < tilez) {
        output = output.hilite();
      }
    } else {
      curz--;
    }
  }
  if (!picked_glyph) {
    debugmsg("Really could not find a glyph!");
    return;
  }
  if (invert) {
    output = output.invert();
  }
  w->putglyph(tile_winx, tile_winy, output);
}
  

Point Map::get_center_point()
{
  return Point(posx + MAP_SIZE / 2, posy + MAP_SIZE / 2);
}
