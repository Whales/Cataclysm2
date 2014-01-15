#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include <vector>
#include "glyph.h"
#include "enum.h"

Terrain_flag lookup_terrain_flag(std::string name);
std::string terrain_flag_name(Terrain_flag flag);

struct Terrain;

struct Terrain_smash
{
  Terrain_smash();
  ~Terrain_smash(){}

  std::string result;
  std::string failure_sound;
  std::string success_sound;
  int hp;
  int armor[DAMAGE_MAX];
  int ignore_chance;

  bool load_data(std::istream &data, std::string name = "unknown");
};

struct Terrain
{
  int uid;
  std::string name;
  glyph sym;
  unsigned int movecost;

  Terrain_smash smash;

  Terrain();
  ~Terrain(){}

  void assign_uid(int id) { uid = id; }
  bool load_data(std::istream &data);
  std::string get_name() { return name; }
  bool has_flag(Terrain_flag flag);
private:
  std::vector<bool> flags;
};

#endif
