#include "map.h"
#include "rng.h"
#include "globals.h"

glyph Tile::top_glyph()
{
  if (!terrain) {
    return glyph('x', c_white, c_red);
  }
  return terrain->sym;
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

Map::Map()
{
  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {
      submaps[x][y] = new Submap;
    }
  }
}

Map::~Map()
{
  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {
      delete submaps[x][y];
    }
  }
}

void Map::generate_empty()
{
  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {
      submaps[x][y]->generate_empty();
    }
  }
}

Tile* Map::get_tile(int x, int y)
{
  if (x < 0 || x >= SUBMAP_SIZE * 3 || y <= 0 || y >= SUBMAP_SIZE * 3) {
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
