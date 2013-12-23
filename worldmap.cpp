#include "worldmap.h"

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

void Worldmap::draw(int posx, int posy)
{
  int xdim, ydim;
  get_screen_dims(xdim, ydim);
  Window w_worldmap(0, 0, xdim, ydim);
  int winx = w_worldmap.sizex(), winy = w_worldmap.sizey();
  bool done = false;
  while (!done) {
    for (int x = 0; x < winx; x++) {
      for (int y = 0; y < winy; y++) {
        int terx = posx + x - (winx / 2), tery = posy + y - (winy / 2);
        Worldmap_tile* tile = get_tile(terx, tery);
        w_worldmap.putglyph(x, y, tile->top_glyph());
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

Worldmap_tile* Worldmap::get_tile(int x, int y)
{
  if (x < 0 || x >= WORLDMAP_SIZE || y < 0 || y >= WORLDMAP_SIZE) {
    tile_oob.terrain = WORLD_TERRAIN.lookup_uid(0);
    return &tile_oob;
  }

  return &(tiles[x][y]);
}
