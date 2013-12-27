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
#include "enum.h"
#include "geometry.h"

class Monster_pool;

struct Tile
{
  Terrain *terrain;
  std::vector<Item> items;

   Tile() { };
  ~Tile() { };

  glyph top_glyph();

  int move_cost();

  bool blocks_sense(Sense_type sense = SENSE_SIGHT);
};

struct Submap
{
  Tile tiles[SUBMAP_SIZE][SUBMAP_SIZE];

  void generate_empty();

  void generate(Worldmap* map, int posx, int posy);
  void generate(World_terrain* terrain[5]);
  void generate(std::string terrain_name);
  void generate(Mapgen_spec* spec);
  void generate_adjacent(Mapgen_spec* spec);

};

struct Submap_pool
{
public:
  Submap_pool();
  ~Submap_pool();
  Submap* at_location(int x, int y);
  Submap* at_location(Point p);

  std::list<Submap*> instances;
private:
  std::map<Point,Submap*,Pointcomp> point_map;
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
  void generate(Worldmap *world); // Uses posx/posy
  void shift(Worldmap *world, int shiftx, int shifty);

// Game engine access
  int move_cost(int x, int y);
  Tile* get_tile(int x, int y);
  bool senses(int x0, int y0, int x1, int y1, Sense_type sense = SENSE_SIGHT);

// Output
  void draw(Window *w, Monster_pool *monsters, int refx, int refy,
            Sense_type sense = SENSE_SIGHT);

  Point get_corner_point();
  int posx, posy;

private:
  Submap* submaps[MAP_SIZE][MAP_SIZE];
  Tile tile_oob;
};
#endif
