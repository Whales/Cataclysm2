#include "map.h"
#include "rng.h"
#include "globals.h"
#include "monster.h"
#include "game.h"
#include "attack.h"

void Tile::set_terrain(Terrain* ter)
{
  if (!ter) {
    return;
  }
  terrain = ter;
  hp = ter->smash.hp;
}

glyph Tile::top_glyph()
{
/* TODO: If terrain is "important" (i.e. not ground) and there's items on top,
 * display terrain glyph, highlighted.
 */
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
  if (terrain->smash.hp > 0 && hp < terrain->smash.hp) {
    int percent = (100 * hp) / terrain->smash.hp;
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
  if (!terrain) {
    return false;
  }
  return terrain->has_flag(flag);
}

std::string Tile::smash(Damage_set damage)
{
  if (!terrain || terrain->smash.result.empty()) {
    return "";
  }
  Terrain_smash smash = terrain->smash;
  if (rng(1, 100) <= smash.ignore_chance) {
    return smash.failure_sound; // Make our "saving throw"
  }
  for (int i = 0; i < DAMAGE_MAX; i++) {
    int dam   = damage.get_damage(i),
        armor = rng(smash.armor[i] / 2, smash.armor[i]);
    dam -= armor;
    if (dam > 0) {
      hp -= dam;
    }
  }
  std::string ret = smash.failure_sound;
  if (hp <= 0) {
    ret = smash.success_sound;
    Terrain* result = TERRAIN.lookup_name(smash.result);
    if (!result) {
      debugmsg("Smash resulted in unknown terrain '%s'", smash.result.c_str());
    } else {
      set_terrain(result);
    }
  }

  return ret;
}

void Tile::open()
{
  if (!terrain->can_open()) {
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
  if (!terrain->can_close()) {
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
    Mapgen_spec* spec;
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
  if (!below) {
    debugmsg("Submap::generate_above(NULL) called!");
    generate_empty();
  }

  level = below->level + 1;
  subname = below->subname;
  rotation = below->rotation;

  Mapgen_spec* spec = MAPGEN_SPECS.random_for_terrain(type, subname, level);
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
  for (int z = 0; z <= VERTICAL_MAP_SIZE * 2 + 1; z++) {
    int zpos = posz - VERTICAL_MAP_SIZE + z;
    for (int x = 0; x < MAP_SIZE; x++) {
      for (int y = 0; y < MAP_SIZE; y++) {
        submaps[x][y][z] = SUBMAP_POOL.at_location(posx + x, posy + y, zpos);
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
}

Generic_map Map::get_movement_map(Intel_level intel)
{
  Generic_map ret(SUBMAP_SIZE * MAP_SIZE, SUBMAP_SIZE * MAP_SIZE);

  for (int x = 0; x < SUBMAP_SIZE * MAP_SIZE; x++) {
    for (int y = 0; y < SUBMAP_SIZE * MAP_SIZE; y++) {
      int cost = move_cost(x, y);
// TODO: If there's a field here, increase cost accordingly
      if (cost == 0 && is_smashable(x, y)) {
        cost = 500; // TODO: Estimate costs more intelligently
      }
      ret.set_cost(x, y, cost);
    }
  }

  return ret;
}

int Map::move_cost(int x, int y, int z)
{
  return get_tile(x, y, z)->move_cost();
}

bool Map::is_smashable(int x, int y, int z)
{
  Tile *t = get_tile(x, y, z);
  if (!t->terrain) {
    return false;
  }
  return !(t->terrain->smash.result.empty());
}

bool Map::has_flag(Terrain_flag flag, int x, int y, int z)
{
  Tile *t = get_tile(x, y, z);
  return t->has_flag(flag);
}

bool Map::add_item(Item item, int x, int y, int z)
{
  if (z == 999) {
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

int Map::item_count(int x, int y, int z)
{
  std::vector<Item>* it = items_at(x, y, z);
  if (!it) {
    return 0;
  }
  return it->size();
}

std::vector<Item>* Map::items_at(int x, int y, int z)
{
  if (z == 999) {
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

Tile* Map::get_tile(int x, int y, int z)
{
// z defaults to 999
  if (z == 999) {
    z = posz;
  }
  z = z - posz + VERTICAL_MAP_SIZE;
  if (x < 0 || x >= SUBMAP_SIZE * MAP_SIZE ||
      y < 0 || y >= SUBMAP_SIZE * MAP_SIZE ||
      z < 0 || z >= VERTICAL_MAP_SIZE * 2 + 1 ) {
    tile_oob.set_terrain(TERRAIN.lookup_uid(0));
    return &tile_oob;
  }

  int sx = x / SUBMAP_SIZE, sy = y / SUBMAP_SIZE;
  return &(submaps[sx][sy][z]->tiles[x % SUBMAP_SIZE][y % SUBMAP_SIZE]);
}

std::string Map::get_name(int x, int y, int z)
{
  Terrain* ter = get_tile(x, y, z)->terrain;
  if (!ter) {
    return "Bug - Null terrain";
  }
  return ter->name;
}

std::string Map::smash(int x, int y, Damage_set damage)
{
  return smash(x, y, 999, damage);
}

std::string Map::smash(int x, int y, int z, Damage_set damage)
{
  Tile* hit = get_tile(x, y, z);
  if (hit) {
    return hit->smash(damage);
  }
  return "";
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

bool Map::close(int x, int y, int z)
{
  Tile* target = get_tile(x, y, z);
  if (target->terrain->can_close()) {
    target->close();
    return true;
  }
  return false;
}

/* Still using Cataclysm style LOS.  It sucks and is slow and I hate it.
 * Basically, iterate over all Bresenham lines between [x0,y0] and [x1,y1].
 * If any of the lines doesn't have something that blocks the relevent sense,
 * return true.  If we iterate through all of them and they all block, return
 * false.
 */
bool Map::senses(int x0, int y0, int x1, int y1, Sense_type sense)
{
  return senses(x0, x0, posz, x1, y1, posz, sense);
}

bool Map::senses(int x0, int y0, int z0, int x1, int y1, int z1,
                 Sense_type sense)
{
  if (sense == SENSE_SIGHT) {
    return (!line_of_sight(x0, y0, z0, x1, y1, z1).empty());
  } else {
    return false;
  }
}

bool Map::senses(Point origin, Point target, Sense_type sense)
{
  return senses(origin.x, origin.y, posz, target.x, target.y, posz, sense);
}

bool Map::senses(Tripoint origin, Tripoint target, Sense_type sense)
{
  return senses(origin.x, origin.y, origin.z, target.x, target.y, target.z,
                sense);
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
  std::vector<int>    t_values; // T-values for bresenham lines

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
    for (int i = 0; i < lines.size(); i++) {
      z_value += z_step;
      if (z_value < 0) {
        z_level--;
        z_value += 100;
      } else if (z_value >= 100) {
        z_level++;
        z_value -= 100;
      }
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
      if (get_tile(lines[i].x, lines[i].y, z_level)->blocks_sense(SENSE_SIGHT)){
        lines.erase(lines.begin() + i);
        t_values.erase(t_values.begin() + i);
        return_values.erase(return_values.begin() + i);
        i--;
      }
    }
  }
  std::vector<Point> ret;
  return ret;
}

std::vector<Point> Map::line_of_sight(Point origin, Point target)
{
  return line_of_sight(origin.x, origin.y, target.x, target.y);
}

void Map::draw(Window* w, Entity_pool *entities, int refx, int refy, int refz,
               Sense_type sense)
{
  if (!w) {
    return;
  }
  int winx = w->sizex(), winy = w->sizey();
  for (int x = 0; x < winx; x++) {
    for (int y = 0; y < winy; y++) {
      int terx = refx + x - (winx / 2), tery = refy + y - (winy / 2);
      if (senses(refx, refy, refz, terx, tery, posz, sense)) {
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
