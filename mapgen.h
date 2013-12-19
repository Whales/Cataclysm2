#ifndef _MAPGEN_SPEC_H_
#define _MAPGEN_SPEC_H_

#include <istream>
#include <vector>
#include <map>
#include <list>
#include <string>
#include "terrain.h"
#include "itemtype.h"

// MAPGEN_SIZE must be a divisor of SUBMAP_SIZE (specified in map.h)!
// Also, changing MAPGEN_SIZE will break all of the already-written mapgen specs
#define MAPGEN_SIZE 25

/* Mapgen_spec_pool looks very much like a datapool but has a few special
 * features, so sadly it must be its own class.
 */
struct Terrain_chance
{
  Terrain_chance(int C = 10, Terrain* T = NULL) : chance (C), terrain(T) {};
  int chance;
  Terrain* terrain;
};

struct Itemtype_chance
{
  Item_chance(int C = 10, Itemtype* I = NULL) : chance(c), itemtype(I) {};
  int chance;
  Itemtype* item;
};

struct Variable_terrain
{
public:
  Variable_terrain();
  ~Variable_terrain(){};

  void load_data(std::istream &data, std::string name = "unknown");
  void add_terrain(int chance, Terrain* terrain);
  void add_terrain(Terrain_chance terrain);

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

  void load_data(std::istream &data, std::string name = "unknown");
  void add_item(int chance, Itemtype* itemtype);
  void add_item(Itemtype_chance itemtype);

  Itemtype* pick();

private:
  std::vector<Itemtype_chance> itemtypes;
  int total_chance;

};

struct Mapgen_spec
{
  Mapgen_spec();
  ~Mapgen_spec(){};

  bool load_data(std::istream &data);
  Terrain* pick_terrain(int x, int y);

  int uid;
  std::string name;
  std::string terrain_name; // World_terrain we belong to

  std::map<char,Variable_terrain> terrain_defs;
  Variable_terrain base_terrain; // Default terrain

  char terrain[MAPGEN_SIZE][MAPGEN_SIZE]; // Keys to terrain_defs

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

  int size();

  std::list<Mapgen_spec*> instances;
private:
  int next_uid;
  std::map<int,Mapgen_spec*> uid_map;
  std::map<std::string,Mapgen_spec*> name_map;
  std::map<std::string,std::vector<Mapgen_spec*> > terrain_name_map;
};

#endif
