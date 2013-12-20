#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include <vector>
#include "glyph.h"
#include "enum.h"

Terrain_flag lookup_terrain_flag(std::string name);
std::string terrain_flag_name(Terrain_flag flag);

struct Terrain
{
  int uid;
  std::string name;
  glyph sym;
  unsigned int movecost;
  std::vector<bool> flags;

  Terrain();
  ~Terrain(){};

  void assign_uid(int id) { uid = id; };
  bool load_data(std::istream &data);
  std::string get_name() { return name; };
};

#endif
