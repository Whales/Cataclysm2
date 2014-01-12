#include "worldmap.h"
#include "rng.h"

glyph Worldmap_tile::top_glyph()
{
  if (!terrain) {
    return glyph();
  }
  return terrain->sym;
}

Worldmap::Worldmap()
{
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    for (int y = 0; y < WORLDMAP_SIZE; y++) {
      tiles[x][y].terrain = NULL;
    }
  }
}

Worldmap::~Worldmap()
{
}

/*
void Worldmap::generate()
{
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    for (int y = 0; y < WORLDMAP_SIZE; y++) {
      if ( (x + y) % 2 ) {
        tiles[x][y].terrain = WORLD_TERRAIN.lookup_name("field");
      } else {
        tiles[x][y].terrain = WORLD_TERRAIN.lookup_name("forest");
      }
    }
  }
}
*/

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
        }
        w_worldmap.putglyph(x, y, sym);
      }
    }
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


Worldmap_tile* Worldmap::get_tile(int x, int y)
{
  if (x < 0 || x >= WORLDMAP_SIZE || y < 0 || y >= WORLDMAP_SIZE) {
    tile_oob.terrain = WORLD_TERRAIN.lookup_uid(0);
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
  Worldmap_tile* tile = get_tile(x, y);
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
