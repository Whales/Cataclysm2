#ifndef _WORLD_TERRAIN_H_
#define _WORLD_TERRAIN_H_

#include "glyph.h"
#include "mapgen.h"
#include <string>
#include <istream>
#include <vector>

enum World_terrain_flag
{
  WTF_NULL = 0,
  WTF_WATER,      // For river building
  WTF_NO_RIVER,   // For river building
  WTF_SALTY,      // Water is salty; also marks end of a river
  WTF_BRIDGE,     // Road on top of this tile is a bridge
  WTF_NO_ROAD,    // Roads don't draw here - but the path continues anyway
  WTF_LINE_DRAWING, // Use line drawings as glyphs
  WTF_MAX
};

World_terrain_flag lookup_world_terrain_flag(std::string name);
std::string world_terrain_flag_name(World_terrain_flag flag);

struct World_terrain
{
  int uid;
  std::string name;
  std::string beach_name;
  int beach_range;
  int road_cost;
  glyph sym;

  World_terrain();
  ~World_terrain(){}

  void assign_uid(int id) { uid = id; }
  bool load_data(std::istream &data);
  std::string get_name() { return name; }

  bool has_flag(World_terrain_flag flag);
private:
  std::vector<bool> flags;
};

World_terrain* make_into_beach(World_terrain* original);

#endif
