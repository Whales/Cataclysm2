#ifndef _MAP_H_
#define _MAP_H_

#define SUBMAP_SIZE 500

#include "window.h"
#include "terrain.h"

struct Tile
{
  Terrain *terrain;

  Tile() { };
  ~Tile() { };


  glyph top_glyph();
};

struct Submap
{
  Tile tiles[SUBMAP_SIZE][SUBMAP_SIZE];

  void generate_empty();
};

class Map
{
public:
  Map();
  ~Map();

  void generate_empty();

  Tile* get_tile(int x, int y);
  void draw(Window *w, int refx, int refy);

private:
  Submap* submaps[3][3];
  Tile tile_oob;
};
#endif
