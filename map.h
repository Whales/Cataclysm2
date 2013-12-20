#ifndef _MAP_H_
#define _MAP_H_

#define SUBMAP_SIZE 25
#define MAP_SIZE 13

#include "window.h"
#include "terrain.h"
#include "world_terrain.h"
#include "mapgen.h"
#include "worldmap.h"
#include "item.h"

struct Tile
{
  Terrain *terrain;
  std::vector<Item> items;

  Tile() { };
  ~Tile() { };

  glyph top_glyph();

  int move_cost();
};

struct Submap
{
  Tile tiles[SUBMAP_SIZE][SUBMAP_SIZE];

  void generate_empty();
  void generate(World_terrain* terrain);
  void generate(std::string terrain_name);
  void generate(Mapgen_spec* spec);

};

class Map
{
public:
  Map();
  ~Map();

// Generation
  void generate_empty();
  void test_generate(std::string terrain_name);
  void generate(Worldmap *world, int posx, int posy,
                int sizex = MAP_SIZE, int sizey = MAP_SIZE);

// Game engine access
  int move_cost(int x, int y);
  Tile* get_tile(int x, int y);

// Output
  void draw(Window *w, int refx, int refy);

private:
  Submap* submaps[MAP_SIZE][MAP_SIZE];
  Tile tile_oob;
};
#endif
