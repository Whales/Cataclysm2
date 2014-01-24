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
  unsigned int height; // Range: 0 - 100, how well it blocks vertically

  Terrain_smash smash;
  std::string open_result;
  std::string close_result;

  bool can_smash() { return !smash.result.empty(); }
  bool can_open()  { return !open_result.empty();  }
  bool can_close() { return !close_result.empty(); }

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
