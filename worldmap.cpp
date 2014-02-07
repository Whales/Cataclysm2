#include "worldmap.h"
#include "rng.h"
#include "map.h"

glyph Worldmap_tile::top_glyph()
{
  if (!terrain) {
    return glyph();
  }
  return terrain->sym;
}

std::string Worldmap_tile::get_name()
{
  if (!terrain) {
    return "Unknown";
  }
  return terrain->get_name();
}

void Worldmap_tile::set_terrain(std::string name)
{
  World_terrain *ter = WORLD_TERRAIN.lookup_name(name);
  if (!ter) {
    debugmsg("Couldn't find world terrain named '%s'", name.c_str());
  } else {
    terrain = ter;
  }
}

Worldmap::Worldmap()
{
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    for (int y = 0; y < WORLDMAP_SIZE; y++) {
      tiles[x][y].terrain = NULL;
    }
  }

  for (std::list<World_terrain*>::iterator it = WORLD_TERRAIN.instances.begin();
       it != WORLD_TERRAIN.instances.end();
       it++) {
    if ( (*it)->has_flag(WTF_SHOP) ) {
      shops.push_back( (*it) );
    }
  }

  if (shops.empty()) {
    debugmsg("No shops available!");
  }
}

Worldmap::~Worldmap()
{
}

void Worldmap::set_terrain(int x, int y, std::string terrain_name)
{
  if (x < 0 || x >= WORLDMAP_SIZE || y < 0 || y >= WORLDMAP_SIZE) {
    return;
  }
  World_terrain* ter = WORLD_TERRAIN.lookup_name(terrain_name);
  if (!ter) {
    debugmsg("Worldmap::set_terrain() couldn't find '%s'",
             terrain_name.c_str());
    return;
  }
  tiles[x][y].terrain = ter;
}

void Worldmap::init_shop_picker()
{
  for (int i = 0; i < shops.size(); i++) {
    shop_count[ shops[i] ] = 0;
  }
}

World_terrain* Worldmap::random_shop()
{
  int most_shops = 0;
  for (int i = 0; i < shops.size(); i++) {
    if (shop_count[ shops[i] ] > most_shops) {
      most_shops = shop_count[ shops[i] ];
    }
  }

  int total_chance = 0;
  std::vector<int> chance;
  for (int i = 0; i < shops.size(); i++) {
    chance.push_back( most_shops * 2 - shop_count[ shops[i] ] );
    total_chance += chance.back();
  }

  int index = rng(1, total_chance);
  for (int i = 0; i < shops.size(); i++) {
    index -= chance[i];
    if (index <= 0) {
      shop_count[ shops[i] ]++;
      return shops[i];
    }
  }
  shop_count[ shops.back() ]++;
  return shops.back();
}

void Worldmap::draw(int posx, int posy)
{
  int origx = posx, origy = posy;
  int xdim, ydim;
  get_screen_dims(xdim, ydim);
  Window w_worldmap(0, 0, xdim, ydim);
  int winx = w_worldmap.sizex(), winy = w_worldmap.sizey();
  bool done = false;
  while (!done) {
    for (int x = 0; x < winx; x++) {
      for (int y = 0; y < winy; y++) {
        int terx = posx + x - (winx / 2), tery = posy + y - (winy / 2);
        glyph sym = get_glyph(terx, tery);
        if ((terx == posx && tery == posy) ||
            (terx == origx && tery == origy) ) {
          sym = sym.invert();
        } else if (terx >= 0 && tery >= 0 &&
                   terx < WORLDMAP_SIZE && tery < WORLDMAP_SIZE &&
                   !tiles[terx][tery].monsters.empty()) {
          sym = sym.hilite(c_red);
        }
        w_worldmap.putglyph(x, y, sym);
      }
    }
    w_worldmap.putstr(0, 0, c_red, c_black, "[%d:%d]", posx, posy);
    w_worldmap.refresh();
    long ch = input();
    switch (ch) {
      case 'j':
      case '2':
      case KEY_DOWN:    posy++; break;
      case 'k':
      case '8':
      case KEY_UP:      posy--; break;
      case 'h':
      case '4':
      case KEY_LEFT:    posx--; break;
      case 'l':
      case '6':
      case KEY_RIGHT:   posx++; break;
      case 'y':
      case '7': posx--; posy--; break;
      case 'u':
      case '9': posx++; posy--; break;
      case 'b':
      case '1': posx--; posy++; break;
      case 'n':
      case '3': posx++; posy++; break;
      case 'q':
      case 'Q': done = true;    break;
    }
  }
}

void Worldmap::draw_minimap(cuss::element *drawing, int cornerx, int cornery)
{
  for (int x = 0; x < drawing->sizex; x++) {
    for (int y = 0; y < drawing->sizey; y++) {
      int terx = cornerx + x, tery = cornery + y;
      glyph sym = get_glyph(terx, tery);
      if (x == drawing->sizex / 2 && y == drawing->sizey / 2) {
        sym = sym.invert();
      }
      drawing->set_data(sym, x, y);
    }
  }
}


Worldmap_tile* Worldmap::get_tile(int x, int y, bool warn)
{
  if (x < 0 || x >= WORLDMAP_SIZE || y < 0 || y >= WORLDMAP_SIZE) {
    tile_oob.terrain = WORLD_TERRAIN.lookup_uid(0);
    tile_oob.monsters.clear();
    if (warn) {
      debugmsg("Worldmap::get_tile(%d, %d) OOB", x, y);
    }
    return &tile_oob;
  }

  return &(tiles[x][y]);
}

glyph Worldmap::get_glyph(int x, int y)
{
/*
  if (x < 0 || x >= WORLDMAP_SIZE || y < 0 || y >= WORLDMAP_SIZE) {
    return glyph();
  }
*/
  Worldmap_tile* tile = get_tile(x, y, false);
  glyph ret = tile->top_glyph();
  if (tile->terrain->has_flag(WTF_LINE_DRAWING)) {
    bool north = (get_tile(x, y - 1)->terrain->has_flag(WTF_LINE_DRAWING));
    bool  east = (get_tile(x + 1, y)->terrain->has_flag(WTF_LINE_DRAWING));
    bool south = (get_tile(x, y + 1)->terrain->has_flag(WTF_LINE_DRAWING));
    bool  west = (get_tile(x - 1, y)->terrain->has_flag(WTF_LINE_DRAWING));
    ret.make_line_drawing(north, east, south, west);
  }

  return ret;
}

std::string Worldmap::get_name(int x, int y)
{
  Worldmap_tile* tile = get_tile(x, y, false);
  return tile->get_name();
}

std::vector<Monster_spawn>* Worldmap::get_spawns(int x, int y)
{
  Worldmap_tile* tile = get_tile(x, y);
  return &(tile->monsters);
}

Generic_map Worldmap::get_generic_map()
{
  Generic_map ret(WORLDMAP_SIZE, WORLDMAP_SIZE);
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    for (int y = 0; y < WORLDMAP_SIZE; y++) {
      ret.set_cost(x, y, tiles[x][y].terrain->road_cost);
    }
  }
  return ret;
}

Point Worldmap::random_tile_with_terrain(std::string name)
{
  return random_tile_with_terrain( WORLD_TERRAIN.lookup_name(name) );
}

Point Worldmap::random_tile_with_terrain(World_terrain* terrain)
{
  if (!terrain) {
    return Point(0, 0);
  }
  std::vector<Point> ret;
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    for (int y = 0; y < WORLDMAP_SIZE; y++) {
      if (get_tile(x, y)->terrain == terrain) {
        ret.push_back( Point(x, y) );
      }
    }
  }
  if (ret.empty()) {
    return Point(0, 0);
  }

  return ret[rng(0, ret.size() - 1)];
}
