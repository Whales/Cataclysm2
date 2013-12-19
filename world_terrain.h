#ifndef _WORLD_TERRAIN_H_
#define _WORLD_TERRAIN_H_

#include <string>
#include <istream>
#include "glyph.h"
#include "mapgen.h"

struct World_terrain
{
  int uid;
  std::string name;
  glyph sym;

  World_terrain();
  ~World_terrain(){};

  void assign_uid(int id) { uid = id; }
  bool load_data(std::istream &data);
  std::string get_name() { return name; }
};

#endif
