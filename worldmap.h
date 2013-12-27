#ifndef _WORLDMAP_H_
#define _WORLDMAP_H_

#include "world_terrain.h"
#include "window.h"
#include "globals.h"
#include "worldmap.h"
#include "cuss.h"
#include <string>

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
  void draw_minimap(cuss::element *drawing, int cornerx, int cornery);
  Worldmap_tile* get_tile(int x, int y);

private:
  Worldmap_tile tiles[WORLDMAP_SIZE][WORLDMAP_SIZE];
  Worldmap_tile tile_oob;
};

#endif
