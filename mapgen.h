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
  Terrain_chance(int C = 10, Terrain* T = NULL) : chance (C), terrain(T) {}
  int chance;
  Terrain* terrain;
};

struct Item_type_chance
{
  Item_type_chance(int C = 10, Item_type* I = NULL) : chance(C), item(I) {}
  int chance;
  Item_type* item;
};

struct Variable_terrain
{
public:
  Variable_terrain();
  ~Variable_terrain(){}

  void add_terrain(int chance, Terrain* terrain);
  void add_terrain(Terrain_chance terrain);
  bool load_data(std::istream &data, std::string name = "unknown",
                 bool allow_nothing = false);

  void prepare(); // If it's a lock, then define Terrain* choice.
  Terrain* pick(bool refresh_choice = false);

  bool lock;

private:
  std::vector<Terrain_chance> ter;
  int total_chance;
  Terrain* choice;

};

// Item_group is just a named list of Item_type_chances.
// It allows for shorthand in mapgen specs.
struct Item_group
{
  Item_group();
  ~Item_group(){}

  std::string name;
  int uid;
  std::vector<Item_type_chance> item_types;
  int total_chance;

  void assign_uid(int id);
  std::string get_data_name();
// TODO: Do we need a get_name()?
  bool load_data(std::istream &data);

  void add_item(int chance, Item_type* item_type);
  void add_item(Item_type_chance item_type);
};

struct Item_area
{
public:
  Item_area();
  ~Item_area(){}

  void add_item(int chance, Item_type* item_type);
  void add_item(Item_type_chance item_type);
  void set_group(Item_group *group);
  void clear_points();
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

struct Subst_chance
{
  Subst_chance(int C = 10, char R = '.') : chance (C), result (R) {}
  int chance;
  char result;
};

struct Tile_substitution
{
  Tile_substitution();
  ~Tile_substitution(){}

  void add_result(int chance, char result);
  void add_result(Subst_chance chance);
  void load_data(std::istream &data, std::string name = "unknown");

  void make_selection();
  char current_selection();

private:
  std::vector<Subst_chance> chances;
  int total_chance;
  char selected;
};

struct Mapgen_spec
{
  Mapgen_spec();
  ~Mapgen_spec(){}

  bool load_data(std::istream &data);

  void prepare(World_terrain* world_ter[5] = NULL);
  void random_rotate();
  void rotate(Direction dir);
  Terrain* pick_terrain(int x, int y);

  std::string get_name();
  void debug_output();

  int uid;
  std::string name;
  std::string terrain_name; // World_terrain we belong to
// The following two are used for building 2nd floors.
  std::string subname;  // Specific flavor of the terrain type
  Direction rotation;

  bool is_adjacent; // True is this is instructions for building adjacent to
                    // terrain_name
  int  num_neighbors;
  int  weight;
  int  z_level;
  std::map<char,Variable_terrain> terrain_defs;
  std::map<char,Item_area> item_defs;
  std::map<char,Tile_substitution> substitutions;
  std::list<std::string> shuffles;
  Variable_terrain base_terrain; // Default terrain

  char terrain[MAPGEN_SIZE][MAPGEN_SIZE]; // Keys to terrain_defs, item_defs etc
  char prepped_terrain[MAPGEN_SIZE][MAPGEN_SIZE]; // After prepare() (subst etc)

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
  Mapgen_spec* random_for_terrain(World_terrain* ptr,
                                  std::vector<bool> neighbor);
  Mapgen_spec* random_for_terrain(World_terrain* ptr, std::string subname,
                                  int z_level);
  Mapgen_spec* random_with_subname(std::string subname, int z_level = 0);

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

  std::map<std::string,std::vector<Mapgen_spec*> > subname_map;
  std::map<std::string,int> subname_total_chance;
};

#endif
