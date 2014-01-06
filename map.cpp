#include "map.h"
#include "rng.h"
#include "globals.h"
#include "monster.h"
#include "game.h"

glyph Tile::top_glyph()
{
/* TODO: If terrain is "important" (i.e. not ground) and there's items on top,
 * display terrain glyph, highlighted.
 */
  if (!items.empty()) {
    return items.back().top_glyph();
  }
  if (!terrain) {
    return glyph();
  }
  return terrain->sym;
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
      return (terrain->flags[TF_OPAQUE]);
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
      tiles[x][y].terrain = (one_in(2) ? grass : dirt);
    }
  }
}

void Submap::generate(Worldmap* map, int posx, int posy)
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

  generate(ter);
}

void Submap::generate(World_terrain* terrain[5])
{
  if (!terrain[0]) {
    generate_empty();
  } else {
    generate( MAPGEN_SPECS.random_for_terrain(terrain[0]) );
  }

  for (int i = 1; i < 5; i++) {
    if (terrain[i] && terrain[i] != terrain[0]) {
      Mapgen_spec* adj = MAPGEN_SPECS.random_adjacent_to(terrain[i]);
      if (adj) {
        Mapgen_spec rotated = adj->rotate( Direction(i) );
        generate_adjacent( &rotated );
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
  if (spec == NULL) {
    generate_empty();
  }
// First, set the terrain.
  for (int x = 0; x < SUBMAP_SIZE; x++) {
    for (int y = 0; y < SUBMAP_SIZE; y++) {
      tiles[x][y].terrain = spec->pick_terrain(x, y);
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
      if (tmpter != NULL) {
        tiles[x][y].terrain = tmpter;
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

Submap* Submap_pool::at_location(int x, int y)
{
  return at_location( Point(x, y) );
}

Submap* Submap_pool::at_location(Point p)
{
  if (point_map.count(p) > 0) {
    return point_map[p];
  }
  Submap* sub = new Submap;
  sub->generate(GAME.worldmap, p.x, p.y);
  point_map[p] = sub;
  return sub;
}

Map::Map()
{
  for (int x = 0; x < MAP_SIZE; x++) {
    for (int y = 0; y < MAP_SIZE; y++) {
      submaps[x][y] = new Submap;
    }
  }
}

Map::~Map()
{
  for (int x = 0; x < MAP_SIZE; x++) {
    for (int y = 0; y < MAP_SIZE; y++) {
      delete submaps[x][y];
    }
  }
}

void Map::generate_empty()
{
  for (int x = 0; x < MAP_SIZE; x++) {
    for (int y = 0; y < MAP_SIZE; y++) {
      submaps[x][y]->generate_empty();
    }
  }
}

void Map::test_generate(std::string terrain_name)
{
  for (int x = 0; x < MAP_SIZE; x++) {
    for (int y = 0; y < MAP_SIZE; y++) {
      submaps[x][y]->generate(terrain_name);
    }
  }
}

void Map::generate(Worldmap *world, int wposx, int wposy, int sizex, int sizey)
{
  posx = wposx;
  posy = wposy;
  for (int x = 0; x < sizex && x < MAP_SIZE; x++) {
    for (int y = 0; y < sizey && y < MAP_SIZE; y++) {
      submaps[x][y] = SUBMAP_POOL.at_location(posx + x, posy + y);
    }
  }
}

void Map::generate(Worldmap *world)
{
  generate(world, posx, posy);
}

void Map::shift(Worldmap *world, int shiftx, int shifty)
{
  if (shiftx == 0 && shifty == 0) {
    return;
  }
  posx += shiftx;
  posy += shifty;
  generate(world, posx, posy);
}

int Map::move_cost(int x, int y)
{
  return get_tile(x, y)->move_cost();
}

bool Map::add_item(Item item, int x, int y)
{
  if (x < 0 || x >= SUBMAP_SIZE * MAP_SIZE ||
      y < 0 || y >= SUBMAP_SIZE * MAP_SIZE   ) {
    return false;
  }
  int sx = x / SUBMAP_SIZE, sy = y / SUBMAP_SIZE;
  x %= SUBMAP_SIZE;
  y %= SUBMAP_SIZE;
  return submaps[sx][sy]->add_item(item, x, y);
}

int Map::item_count(int x, int y)
{
  if (x < 0 || x >= SUBMAP_SIZE * MAP_SIZE ||
      y < 0 || y >= SUBMAP_SIZE * MAP_SIZE   ) {
    return 0;
  }
  int sx = x / SUBMAP_SIZE, sy = y / SUBMAP_SIZE;
  x %= SUBMAP_SIZE;
  y %= SUBMAP_SIZE;
  return submaps[sx][sy]->item_count(x, y);
}

std::vector<Item>* Map::items_at(int x, int y)
{
  if (x < 0 || x >= SUBMAP_SIZE * MAP_SIZE ||
      y < 0 || y >= SUBMAP_SIZE * MAP_SIZE   ) {
    return NULL;
  }
  int sx = x / SUBMAP_SIZE, sy = y / SUBMAP_SIZE;
  x %= SUBMAP_SIZE;
  y %= SUBMAP_SIZE;
  return submaps[sx][sy]->items_at(x, y);
}

Tile* Map::get_tile(int x, int y)
{
  if (x < 0 || x >= SUBMAP_SIZE * MAP_SIZE ||
      y < 0 || y >= SUBMAP_SIZE * MAP_SIZE   ) {
    tile_oob.terrain = TERRAIN.lookup_uid(0);
    return &tile_oob;
  }

  int sx = x / SUBMAP_SIZE, sy = y / SUBMAP_SIZE;
  return &(submaps[sx][sy]->tiles[x % SUBMAP_SIZE][y % SUBMAP_SIZE]);
}

/* Still using Cataclysm style LOS.  It sucks and is slow and I hate it.
 * Basically, iterate over all Bresenham lines between [x0,y0] and [x1,y1].
 * If any of the lines doesn't have something that blocks the relevent sense,
 * return true.  If we iterate through all of them and they all block, return
 * false.
 */
bool Map::senses(int x0, int y0, int x1, int y1, Sense_type sense)
{
  std::vector<Point>  lines;    // Process many lines at once.
  std::vector<int>    t_values; // T-values for bresenham lines

  int dx = x1 - x0, dy = y1 - y0;
  int ax = abs(dx) << 1, ay = abs(dy) << 1;
  int sx = (dx < 0 ? -1 : 1), sy = (dy < 0 ? -1 : 1);
  if (dx == 0) {
    sx = 0;
  }
  if (dy == 0) {
    sy = 0;
  }

  int min_t = (ax > ay ? ay - ax : ax - ay),
  //int min_t = 0,
      max_t = 0;
  if (dx == 0 || dy == 0) {
    min_t = 0;
  }
// Init our "lines"
  for (int t = min_t; t <= max_t; t++) {
    lines.push_back( Point(x0, y0) );
    t_values.push_back(t);
  }
// Keep going as long as we've got at least one valid line
  while (!lines.empty()) {
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
      if (lines[i].x == x1 && lines[i].y == y1) {
        return true;
      }
      if (get_tile(lines[i].x, lines[i].y)->blocks_sense(sense)) {
        lines.erase(lines.begin() + i);
        t_values.erase(t_values.begin() + i);
        i--;
      }
    }
  }
  return false;
}
  

void Map::draw(Window* w, Monster_pool *monsters, int refx, int refy,
               Sense_type sense)
{
  if (!w) {
    return;
  }
  int winx = w->sizex(), winy = w->sizey();
  for (int x = 0; x < winx; x++) {
    for (int y = 0; y < winy; y++) {
      int terx = refx + x - (winx / 2), tery = refy + y - (winy / 2);
      if (senses(refx, refy, terx, tery)) {
        glyph output;
        bool picked_glyph = false;
// First, check if we should draw a monster
        if (monsters) {
          Monster* monster = monsters->monster_at(terx, tery);
          if (monster) {
            output = monster->top_glyph();
            picked_glyph = true;
          }
        }
// Finally, if nothing else, get the glyph from the tile
        if (!picked_glyph) {
          Tile* tile = get_tile(terx, tery);
          if (tile) {
            output = tile->top_glyph();
          } else {
            debugmsg("Really could not find a glyph!");
          }
        }
        w->putglyph(x, y, output);
      } else {
// TODO: Don't use a literal glyph!  TILES GEEZE
        w->putglyph(x, y, glyph(' ', c_black, c_black));
      }
    }
  }
}
