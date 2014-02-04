#ifndef _MAP_H_
#define _MAP_H_

#define SUBMAP_SIZE 25
#define MAP_SIZE 13
#define VERTICAL_MAP_SIZE 3

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
#include "field.h"

class Entity_pool;
class Field;
class Field_type;

struct Tile
{
  Terrain *terrain;
  std::vector<Item> items;
  Field field;
  int hp;

   Tile() { hp = 0; }
  ~Tile() { }

  void set_terrain(Terrain* ter);

  glyph top_glyph();
  int move_cost();
  std::string get_name();
  bool blocks_sense(Sense_type sense = SENSE_SIGHT);
  bool has_flag(Terrain_flag flag);
  bool has_field();

  bool is_smashable();
  std::string smash(Damage_set dam);  // Returns the sound
  bool damage(Damage_set dam);            // Returns true on destruction
  bool damage(Damage_type type, int dam); // Returns true on destruction
  void open();
  void close();
};

struct Submap
{
  Tile tiles[SUBMAP_SIZE][SUBMAP_SIZE];

/* Subname is the specific flavor of the mapgen_spec used here.  This is used
 * when building second stories; so only use house_wood to build the 2nd floor
 * of a house_wood map.
 * rotation and level are used to further match to the floor below.
 */
  std::string subname;
  Direction rotation;
  int level;

  Submap();
  ~Submap();

  void generate_empty();
  void generate_open();

  void generate(Worldmap* map, int posx, int posy, int posz = 0);
  void generate(World_terrain* terrain[5], int posz = 0);
  void generate(std::string terrain_name);
  void generate(Mapgen_spec* spec);
  void generate_adjacent(Mapgen_spec* spec);
  void generate_above(World_terrain* type, Submap* below);
  void spawn_monsters(Worldmap* world, int worldx, int worldy, int worldz,
                      int smx, int smy);

  bool add_item(Item item, int x, int y);
  int  item_count(int x, int y);
  std::vector<Item>* items_at(int x, int y);
  Point random_empty_tile();

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
  std::map<Tripoint,Submap*,Tripointcomp> point_map;
};

class Map
{
public:
  Map();
  ~Map();

// Generation
  void generate_empty();
  void test_generate(std::string terrain_name);
  void generate(Worldmap *world, int wposx = -999, int wposy = -999,
                                 int wposz = -999);
  void shift(Worldmap *world, int shiftx, int shifty, int shiftz = 0);
  void spawn_monsters(Worldmap *world, int x, int y);

// Mapping & pathing
  Generic_map get_dijkstra_map(Tripoint target, int weight,
                               bool include_smashable = true);
  Generic_map get_movement_map(Entity_AI AI, Tripoint origin, Tripoint target);

  bool senses(int x0, int y0, int x1, int y1, int range, Sense_type sense);
  bool senses(int x0, int y0, int z0, int x1, int y1, int z1, int range,
              Sense_type sense);
  bool senses(Point origin, Point target, int range, Sense_type sense);
  bool senses(Tripoint origin, Tripoint target, int range, Sense_type sense);

  std::vector<Point> line_of_sight(int x0, int y0, int x1, int y1);
  std::vector<Point> line_of_sight(int x0, int y0, int z0,
                                   int x1, int y1, int z1);
  std::vector<Point> line_of_sight(Point origin, Point target);
  std::vector<Point> line_of_sight(Tripoint origin, Tripoint target);


// Tile information
  int  move_cost(Tripoint pos);
  int  move_cost(int x, int y, int z = 999);

  bool is_smashable(Tripoint pos);
  bool is_smashable(int x, int y, int z = 999);

  bool has_flag(Terrain_flag flag, Tripoint pos);
  bool has_flag(Terrain_flag flag, int x, int y, int z = 999);

  bool blocks_sense(Sense_type sense, Tripoint pos);
  bool blocks_sense(Sense_type sense, int x, int y, int z = 999);

  bool add_item(Item item, Tripoint pos);
  bool add_item(Item item, int x, int y, int z = 999);

  bool add_field(Field_type* type, Tripoint pos, std::string creator = "");
  bool add_field(Field_type* type, int x, int y, int z = 999,
                 std::string creator = "");
  bool add_field(Field field,      Tripoint pos);
  bool add_field(Field field,      int x, int y, int z = 999);

  int  item_count(Tripoint pos);
  int  item_count(int x, int y, int z = 999);

  std::vector<Item>* items_at(Tripoint pos);
  std::vector<Item>* items_at(int x, int y, int z = 999);

  bool contains_field(Tripoint pos);
  bool contains_field(int x, int y, int z = 999);
  Field* field_at(Tripoint pos);
  Field* field_at(int x, int y, int z = 999);
  int field_uid_at(Tripoint pos);
  int field_uid_at(int x, int y, int z = 999);

  Tile* get_tile(Tripoint pos);
  Tile* get_tile(int x, int y, int z = 999);

  std::string get_name(Tripoint pos);
  std::string get_name(int x, int y, int z = 999);

// Map-altering
  void smash(int x, int y,          Damage_set dam, bool make_sound = true);
  void smash(int x, int y, int z,   Damage_set dam, bool make_sound = true);
  void smash(Tripoint pos,          Damage_set dam, bool make_sound = true);
  void damage(int x, int y,         Damage_set dam);
  void damage(int x, int y, int z,  Damage_set dam);
  void damage(Tripoint pos,         Damage_set dam);

  bool open (Tripoint pos);
  bool open (int x, int y, int z = 999);
  bool close(Tripoint pos);
  bool close(int x, int y, int z = 999);

// Regularly-run functions
  void process_fields();

// Output
  void draw(Window *w, Entity_pool *entities, Tripoint ref,
            Sense_type sense = SENSE_SIGHT);
  void draw(Window *w, Entity_pool *entities, int refx, int refy, int refz,
            Sense_type sense = SENSE_SIGHT);

  void draw_tile(Window* w, Entity_pool *entities, int tilex, int tiley,
                 int refx, int refy, bool invert);
  void draw_tile(Window* w, Entity_pool *entities,
                 int tilex, int tiley, int tilez,
                 int refx, int refy, bool invert);

  Point get_center_point();
  int posx, posy, posz;

private:
  Submap* submaps[MAP_SIZE][MAP_SIZE][VERTICAL_MAP_SIZE * 2 + 1];
  Tile tile_oob;
};
#endif
