#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include <stdint.h>
#include "glyph.h"

struct Terrain
{
  int uid;
  std::string name;
  glyph sym;
  unsigned int movecost;
  uint64_t flags;

  Terrain();
  ~Terrain(){};

  void assign_uid(int id) { uid = id; };
  bool load_data(std::istream &data);
  std::string get_name() { return name; };
};

#endif
