#ifndef _WORLDMAP_H_
#define _WORLDMAP_H_

#include "world_terrain.h"
#include "window.h"
#include "globals.h"
#include "worldmap.h"
#include "cuss.h"
#include "pathfind.h"
#include "monster_spawn.h"
#include <string>

#define WORLDMAP_SIZE 150

struct Worldmap_tile
{
  World_terrain *terrain;
  std::vector<Monster_spawn> monsters;

  glyph top_glyph();
  std::string get_name();

  void set_terrain(std::string name);
};

class Worldmap
{
public:
  Worldmap();
  ~Worldmap();

  void generate();
  void place_monsters();
  void set_terrain(int x, int y, std::string terrain_name);

  void init_shop_picker();
  World_terrain* random_shop();

  void draw(int posx, int posy);
  void draw_minimap(cuss::element *drawing, int cornerx, int cornery);
  Worldmap_tile* get_tile(int x, int y, bool warn = true);
  glyph get_glyph(int x, int y);
  std::string get_name(int x, int y);
  std::vector<Monster_spawn>* get_spawns(int x, int y);

  Generic_map get_generic_map();  // For road drawing and long-travel

  Point random_tile_with_terrain(std::string name);
  Point random_tile_with_terrain(World_terrain* terrain);

private:
  Worldmap_tile tiles[WORLDMAP_SIZE][WORLDMAP_SIZE];
  Biome*       biomes[WORLDMAP_SIZE][WORLDMAP_SIZE];
  Worldmap_tile tile_oob;
  std::vector<World_terrain*> shops;
  std::map<World_terrain*,int> shop_count;
};

#endif
