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
#include "attack.h"
#include "pathfind.h"

class Entity_pool;

struct Tile
{
  Terrain *terrain;
  std::vector<Item> items;
  int hp;

   Tile() { hp = 0; }
  ~Tile() { }

  void set_terrain(Terrain* ter);

  glyph top_glyph();
  int move_cost();
  bool blocks_sense(Sense_type sense = SENSE_SIGHT);

  std::string smash(Damage_set damage); // Returns the sound
  void open();
  void close();
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

  bool add_item(Item item, int x, int y);
  int item_count(int x, int y);
  std::vector<Item>* items_at(int x, int y);

};

struct Submap_pool
{
public:
  Submap_pool();
  ~Submap_pool();
  Submap* at_location(int x, int y, int z = 0);
  Submap* at_location(Point p);
  Submap* at_location(Tripoint p);

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
  Generic_map get_movement_map(Intel_level intel);
  int move_cost(int x, int y);
  bool is_smashable(int x, int y);
  int item_count(int x, int y);
  std::vector<Item>* items_at(int x, int y);
  Tile* get_tile(int x, int y);
  std::string get_name(int x, int y);
  bool add_item(Item item, int x, int y);
  std::string smash(int x, int y, Damage_set damage); // Returns the sound
  bool open (int x, int y);
  bool close(int x, int y);

  bool senses(int x0, int y0, int x1, int y1, Sense_type sense = SENSE_SIGHT);
  bool senses(Point origin, Point target, Sense_type sense = SENSE_SIGHT);
  std::vector<Point> line_of_sight(int x0, int y0, int x1, int y1);
  std::vector<Point> line_of_sight(Point origin, Point target);

// Output
  void draw(Window *w, Entity_pool *entities, int refx, int refy,
            Sense_type sense = SENSE_SIGHT);

  void draw_tile(Window* w, Entity_pool *entities, int tilex, int tiley,
                    int refx, int refy, bool invert = false);

  Point get_center_point();
  int posx, posy;

private:
  Submap* submaps[MAP_SIZE][MAP_SIZE];
  Tile tile_oob;
};
#endif
