#ifndef _WORLDMAP_H_
#define _WORLDMAP_H_

#include <string>
#include "world_terrain.h"
#include "window.h"
#include "globals.h"
#include "worldmap.h"

#define WORLDMAP_SIZE 150

struct Worldmap_tile
{
  World_terrain *terrain;
  glyph top_glyph();
};

class Worldmap
{
public:
  Worldmap();
  ~Worldmap();

  void generate();

  void draw(int posx, int posy);
  Worldmap_tile* get_tile(int x, int y);

private:
  Worldmap_tile tiles[WORLDMAP_SIZE][WORLDMAP_SIZE];
  Worldmap_tile tile_oob;
};

#endif
