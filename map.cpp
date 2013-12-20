#include "map.h"
#include "rng.h"
#include "globals.h"

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

void Submap::generate(World_terrain* terrain)
{
  if (!terrain) {
    generate_empty();
    return;
  }
  generate(terrain->get_name());
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
    while (area->place_item()) {
      Point p = area->pick_location();
      Item item( area->pick_type() );
      tiles[p.x][p.y].items.push_back(item);
    }
  }
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

void Map::generate(Worldmap *world, int posx, int posy, int sizex, int sizey)
{
  for (int x = 0; x < sizex; x++) {
    for (int y = 0; y < sizey; y++) {
      Worldmap_tile* tile = world->get_tile(posx + x, posy + y);
      if (!tile) {
        submaps[x][y]->generate_empty();
      } else {
        submaps[x][y]->generate(tile->terrain);
      }
    }
  }
}

int Map::move_cost(int x, int y)
{
  return get_tile(x, y)->move_cost();
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

void Map::draw(Window* w, int refx, int refy)
{
  if (!w) {
    return;
  }
  int winx = w->sizex(), winy = w->sizey();
  for (int x = 0; x < winx; x++) {
    for (int y = 0; y < winy; y++) {
      int terx = refx + x - (winx / 2), tery = refy + y - (winy / 2);
      Tile* tile = get_tile(terx, tery);
      w->putglyph(x, y, tile->top_glyph());
    }
  }
}
