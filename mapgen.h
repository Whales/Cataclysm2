#ifndef _MAPGEN_SPEC_H_
#define _MAPGEN_SPEC_H_

#include <istream>
#include <vector>
#include <map>
#include <list>
#include <string>
#include "terrain.h"
#include "item_type.h"
#include "geometry.h"

// MAPGEN_SIZE must be a divisor of SUBMAP_SIZE (specified in map.h)!
// Also, changing MAPGEN_SIZE will break all of the already-written mapgen specs
#define MAPGEN_SIZE 25

struct World_terrain;

/* Mapgen_spec_pool looks very much like a datapool but has a few special
 * features, so sadly it must be its own class.
 */
struct Terrain_chance
{
  Terrain_chance(int C = 10, Terrain* T = NULL) : chance (C), terrain(T) {};
  int chance;
  Terrain* terrain;
};

struct Item_type_chance
{
  Item_type_chance(int C = 10, Item_type* I = NULL) : chance(C), item(I) {};
  int chance;
  Item_type* item;
};

struct Variable_terrain
{
public:
  Variable_terrain();
  ~Variable_terrain(){};

  void add_terrain(int chance, Terrain* terrain);
  void add_terrain(Terrain_chance terrain);
  void load_data(std::istream &data, std::string name = "unknown");

  Terrain* pick();

private:
  std::vector<Terrain_chance> ter;
  int total_chance;

};

struct Item_area
{
public:
  Item_area();
  ~Item_area(){};

  void add_item(int chance, Item_type* item_type);
  void add_item(Item_type_chance item_type);
  void add_point(int x, int y);
  void load_data(std::istream &data, std::string name = "unknown");

// Functions used for item placement.
  bool place_item();
  Item_type* pick_type();
  Point pick_location();

  int overall_chance;

private:
  std::vector<Item_type_chance> item_types;
  std::vector<Point> locations;
  int total_chance;

};

struct Mapgen_spec
{
  Mapgen_spec();
  ~Mapgen_spec(){};

  bool load_data(std::istream &data);
  Terrain* pick_terrain(int x, int y);

  Mapgen_spec random_rotate();
  Mapgen_spec rotate(Direction dir);

  void debug_output();

  int uid;
  std::string name;
  std::string terrain_name; // World_terrain we belong to

  bool is_adjacent; // True is this is instructions for building adjacent to
                    // terrain_name
  int weight;
  std::map<char,Variable_terrain> terrain_defs;
  std::map<char,Item_area> item_defs;
  Variable_terrain base_terrain; // Default terrain

  char terrain[MAPGEN_SIZE][MAPGEN_SIZE]; // Keys to terrain_defs, item_defs etc

};

class Mapgen_spec_pool
{
public:
  Mapgen_spec_pool();
  ~Mapgen_spec_pool();

  bool load_from(std::string filename);
  bool load_element(std::istream &data);
  
  Mapgen_spec* lookup_uid(int uid);
  Mapgen_spec* lookup_name(std::string name);

  std::vector<Mapgen_spec*> lookup_terrain_name(std::string name);
  Mapgen_spec* random_for_terrain(std::string name);

  std::vector<Mapgen_spec*> lookup_terrain_ptr(World_terrain* ptr);
  Mapgen_spec* random_for_terrain(World_terrain* ptr);

  std::vector<Mapgen_spec*> lookup_adjacent_name(std::string name);
  Mapgen_spec* random_adjacent_to(std::string name);

  std::vector<Mapgen_spec*> lookup_adjacent_ptr(World_terrain* ptr);
  Mapgen_spec* random_adjacent_to(World_terrain* ptr);

  int size();

  std::list<Mapgen_spec*> instances;
private:
  int next_uid;
  std::map<int,Mapgen_spec*> uid_map;
  std::map<std::string,Mapgen_spec*> name_map;

  std::map<std::string,std::vector<Mapgen_spec*> > terrain_name_map;
  std::map<std::string,int> terrain_name_total_chance;
  std::map<World_terrain*,std::vector<Mapgen_spec*> > terrain_ptr_map;
  std::map<World_terrain*,int> terrain_ptr_total_chance;

  std::map<std::string,std::vector<Mapgen_spec*> > adjacent_name_map;
  std::map<std::string,int> adjacent_name_total_chance;
  std::map<World_terrain*,std::vector<Mapgen_spec*> > adjacent_ptr_map;
  std::map<World_terrain*,int> adjacent_ptr_total_chance;
};

#endif
